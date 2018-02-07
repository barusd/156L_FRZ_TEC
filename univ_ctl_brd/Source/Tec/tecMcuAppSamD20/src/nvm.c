/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "dbgCmds.h"
#include "i2cProtocol.h"
#include "extern.h"

/************************************************************************/
/* Module Instances                                                     */
/************************************************************************/
struct nvm_fusebits fuses_settings;

uint8_t MAC_address[MAC_ADDR_SIZE];
uint8_t strModel[MODEL_SIZE];
uint8_t strSerial[SERIAL_SIZE];
uint8_t strUiFwVer[UIFWVER_SIZE] = {' ',' ',' ',' ',' ',' ','\0'};
bool writeToEepromFlag = false;
uint8_t eepromPageData[EEPROM_PAGE_SIZE];
struct _varInfo eepromDataInfo;
uint8_t eepromTemp[4];
uint8_t *pTempEepromBuf = &eepromTemp[0];
uint8_t defrostFreqHrs;
uint8_t defrostDurationMins;
uint32_t eepromStatus = EEPROM_NOT_WRITTEN;
uint32_t doorOpenLifetimeCnt;

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
void set_fuses()
{
	bool writeFlag = false;
	
	nvm_get_fuses(&fuses_settings);
	
	if (fuses_settings.eeprom_size != NVM_EEPROM_EMULATOR_SIZE_1024)
	{
		fuses_settings.eeprom_size = NVM_EEPROM_EMULATOR_SIZE_1024;
		writeFlag = true;
	}
	
	// add more fuse settings here !!!
	
	if (writeFlag == true)
	{
		nvm_set_fuses(&fuses_settings);
	}
	
	writeFlag = false;
}

void configure_eeprom(void)
{
	// Setup EEPROM emulator service
	enum status_code error_code = eeprom_emulator_init();
	if (error_code == STATUS_ERR_NO_MEMORY) {
		while (true) {
			// No EEPROM section has been set in the device's fuses
		}
	}
	else if (error_code != STATUS_OK) {
		// Erase the emulated EEPROM memory (assume it is unformatted or
		// irrecoverably corrupt)
		eeprom_emulator_erase_memory();
		eeprom_emulator_init();
	}
}

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
uint8_t* floatToByteArray(float f)
{
	uint8_t* ret = malloc(4 * sizeof(uint8_t));
	unsigned int asInt = *((int*)&f);

	int i;
	for (i = 0; i < 4; i++) {
		ret[i] = (asInt >> 8 * i) & 0xFF;
	}

	return ret;
}

uint8_t* intToByteArray(uint16_t f)
{
	uint8_t* ret = malloc(2 * sizeof(uint8_t));
	//unsigned int asInt = *((int*)&f);

	int i;
	for (i = 0; i < 2; i++) {
		ret[i] = (f >> 8 * i) & 0xFF;
	}

	return ret;
}

uint8_t* longToByteArray(uint32_t l)
{
	uint8_t* ret = malloc(4 * sizeof(uint8_t));
	unsigned int asInt = *((int*)&l);

	int i;
	for (i = 0; i < 4; i++) {
		ret[i] = (asInt >> 8 * i) & 0xFF;
	}

	return ret;
}

void eepromReadParams(void)
{
	eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
	*(pTempEepromBuf + 0) = eepromPageData[19];
	*(pTempEepromBuf + 1) = eepromPageData[20];
	*(pTempEepromBuf + 2) = eepromPageData[21];
	*(pTempEepromBuf + 3) = 0xff;
	eepromStatus = (*(uint32_t *)pTempEepromBuf);
	eepromStatus = ~eepromStatus;
	
	if ( (eepromStatus & EEPROM_MFGTESTFLAG1_WRITTEN) == EEPROM_MFGTESTFLAG1_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		mfgTestModeFlag1 = eepromPageData[22];
	}
	else
	{
		mfgTestModeFlag1 = 0x01;
	}

	if ( (eepromStatus & EEPROM_MFGTESTFLAG2_WRITTEN) == EEPROM_MFGTESTFLAG2_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		mfgTestModeFlag2 = eepromPageData[23];
	}
	else
	{
		mfgTestModeFlag2 = 0x01;
	}
	
	if ( (eepromStatus & EEPROM_CHAMBEROFFSET_WRITTEN) == EEPROM_CHAMBEROFFSET_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[0];
		*(pTempEepromBuf + 1) = eepromPageData[1];
		*(pTempEepromBuf + 2) = eepromPageData[2];
		*(pTempEepromBuf + 3) = eepromPageData[3];
		chamber1.chamberOffset = *(float *)pTempEepromBuf;
	}
	else
	{
		chamber1.chamberOffset = 0.0;
	}
	
	if ( (eepromStatus & EEPROM_GLYCOLOFFSET_WRITTEN) == EEPROM_GLYCOLOFFSET_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[4];
		*(pTempEepromBuf + 1) = eepromPageData[5];
		*(pTempEepromBuf + 2) = eepromPageData[6];
		*(pTempEepromBuf + 3) = eepromPageData[7];
		chamber1.glycolOffset = *(float *)pTempEepromBuf;
	}
	else
	{
		chamber1.glycolOffset = 0.0;
	}

	if ( (eepromStatus & EEPROM_CHAMBER2OFFSET_WRITTEN) == EEPROM_CHAMBER2OFFSET_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[24];
		*(pTempEepromBuf + 1) = eepromPageData[25];
		*(pTempEepromBuf + 2) = eepromPageData[26];
		*(pTempEepromBuf + 3) = eepromPageData[27];
		chamber2.chamberOffset = *(float *)pTempEepromBuf;
	}
	else
	{
		chamber2.chamberOffset = 0.0;
	}
	
	if ( (eepromStatus & EEPROM_DEFROSTFREQMAXHRS_WRITTEN) == EEPROM_DEFROSTFREQMAXHRS_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[8];
		*(pTempEepromBuf + 1) = eepromPageData[9];
		defrostFreqMaxHrs = *(uint16_t *)pTempEepromBuf;
	}
	else
	{
		defrostFreqMaxHrs = DEFROST_MAX_INTERVAL_TIME_IN_HRS;
	}
	
	if ( (eepromStatus & EEPROM_DEFROSTFREQHRS_WRITTEN) == EEPROM_DEFROSTFREQHRS_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		defrostFreqHrs = eepromPageData[10];
	}
	else
	{
		defrostFreqHrs = DEFAULT_DEFROST_FREQ_HRS;
	}
	
	if ( (eepromStatus & EEPROM_DEFROSTDURATIONMINS_WRITTEN) == EEPROM_DEFROSTDURATIONMINS_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		defrostDurationMins = eepromPageData[11];
	}
	else
	{
		defrostDurationMins = DEFAULT_DEFROST_DURATION_MINS;
	}
	
	if ( (eepromStatus & EEPROM_MACADDR_WRITTEN) == EEPROM_MACADDR_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		MAC_address[0] = eepromPageData[12];
		MAC_address[1] = eepromPageData[13];
		MAC_address[2] = eepromPageData[14];
		MAC_address[3] = eepromPageData[15];
		MAC_address[4] = eepromPageData[16];
		MAC_address[5] = eepromPageData[17];
	}
	else
	{
		;
	}

	if ( (eepromStatus & EEPROM_MODEL_WRITTEN) == EEPROM_MODEL_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE1, eepromPageData);
		memcpy(strModel, eepromPageData, MODEL_SIZE);
	}
	else
	{
		strcpy(strModel, DEFAULT_MODEL);
	}

	if ( (eepromStatus & EEPROM_UIFWVER_WRITTEN) == EEPROM_UIFWVER_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE1, eepromPageData);
		memcpy(strUiFwVer, eepromPageData, UIFWVER_SIZE);
		strUiFwVer[UIFWVER_SIZE-1] = '\0';
	}
	else
	{
		strcpy(strUiFwVer, DEFAULT_UIFWVER);
		strUiFwVer[UIFWVER_SIZE-1] = '\0';
	}	

	if ( (eepromStatus & EEPROM_SERIAL_WRITTEN) == EEPROM_SERIAL_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE2, eepromPageData);
		memcpy(strSerial, eepromPageData, SERIAL_SIZE);
	}
	else
	{
		strcpy(strSerial, DEFAULT_SERIAL);
	}

	if ( (eepromStatus & EEPROM_CHAMBERBETA_WRITTEN) == EEPROM_CHAMBERBETA_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[28];
		*(pTempEepromBuf + 1) = eepromPageData[29];
		chamberThermistorBeta = *(uint16_t *)pTempEepromBuf;
	}
	else
	{
		chamberThermistorBeta = BCOEFFICIENT_TCHAMBER;
	}

	if ( (eepromStatus & EEPROM_REJECTBETA_WRITTEN) == EEPROM_REJECTBETA_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[30];
		*(pTempEepromBuf + 1) = eepromPageData[31];
		rejectThermistorBeta = *(uint16_t *)pTempEepromBuf;
	}
	else
	{
		rejectThermistorBeta = BCOEFFICIENT_TREJ;
	}	

	if ( (eepromStatus & EEPROM_RUNSELFTESTFLAG_WRITTEN) == EEPROM_RUNSELFTESTFLAG_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		runSelfTestOnPOR = eepromPageData[32];
	}
	else
	{
		runSelfTestOnPOR = DEFAULT_SELFTESTONPOR_SETTING;
	}

	if ( (eepromStatus & EEPROM_PS1OUTPUTDUTYOFFSET_WRITTEN) == EEPROM_PS1OUTPUTDUTYOFFSET_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[33];
		*(pTempEepromBuf + 1) = eepromPageData[34];
		*(pTempEepromBuf + 2) = eepromPageData[35];
		*(pTempEepromBuf + 3) = eepromPageData[36];
		ps1.phpOutputDutyOffset = *(float *)pTempEepromBuf;
	}
	else
	{
		ps1.phpOutputDutyOffset = DEFAULT_PS1OUTPUTDUTYOFFSET_SETTING;
	}
	
	if ( (eepromStatus & EEPROM_PS2OUTPUTDUTYOFFSET_WRITTEN) == EEPROM_PS2OUTPUTDUTYOFFSET_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[37];
		*(pTempEepromBuf + 1) = eepromPageData[38];
		*(pTempEepromBuf + 2) = eepromPageData[39];
		*(pTempEepromBuf + 3) = eepromPageData[40];
		ps2.phpOutputDutyOffset = *(float *)pTempEepromBuf;
	}
	else
	{
		ps2.phpOutputDutyOffset = DEFAULT_PS2OUTPUTDUTYOFFSET_SETTING;
	}	

	if ( (eepromStatus & EEPROM_MAXALLOWEDTEMP_WRITTEN) == EEPROM_MAXALLOWEDTEMP_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[41];
		*(pTempEepromBuf + 1) = eepromPageData[42];
		*(pTempEepromBuf + 2) = eepromPageData[43];
		*(pTempEepromBuf + 3) = eepromPageData[44];
		maxAllowedChamberTemp = *(float *)pTempEepromBuf;
	}
	else
	{
		maxAllowedChamberTemp = DEFAULT_MAX_ALLOWED_TCHA;
	}	
	
	if ( (eepromStatus & EEPROM_MINALLOWEDTEMP_WRITTEN) == EEPROM_MINALLOWEDTEMP_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[45];
		*(pTempEepromBuf + 1) = eepromPageData[46];
		*(pTempEepromBuf + 2) = eepromPageData[47];
		*(pTempEepromBuf + 3) = eepromPageData[48];
		minAllowedChamberTemp = *(float *)pTempEepromBuf;
	}
	else
	{
		minAllowedChamberTemp = DEFAULT_MIN_ALLOWED_TCHA;
	}	

	if ( (eepromStatus & EEPROM_DOOROPENLIFETIMECNT_WRITTEN) == EEPROM_DOOROPENLIFETIMECNT_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		*(pTempEepromBuf + 0) = eepromPageData[49];
		*(pTempEepromBuf + 1) = eepromPageData[50];
		*(pTempEepromBuf + 2) = eepromPageData[51];
		*(pTempEepromBuf + 3) = eepromPageData[52];
		doorOpenLifetimeCnt = (*(uint32_t *)pTempEepromBuf);
	}
	else
	{
		doorOpenLifetimeCnt = 0x00;
	}	
	
	if ( (eepromStatus & EEPROM_DOOROPENCOUNT1HRLIMIT_WRITTEN) == EEPROM_DOOROPENCOUNT1HRLIMIT_WRITTEN)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		doorOpenCount1HrLimit = eepromPageData[53];
	}
	else
	{
		doorOpenCount1HrLimit = DEFAULT_DOOR_OPEN_CNT_1HR_LIMIT;
	}	

}

void eepromWriteParams(void)
{
	// PAGE0
	// [0..3]   (float) chamberOffset1
	// [4..7]   (float) glycolOffset
	// [8..9]   (uint16) defrostFreqMaxHrs
	// [10]     (uint8) defrostFreqHrs
	// [11]     (uint8) defrostDurationMins
	// [12..17] (uint8) MAC_address[0..5]
	// [18]
	// [19..21] (uint32) eepromStatus
	// [22]     (uint8) mfgTestModeFlag1
	// [23]     (uint8) mfgTestModeFlag2
	// [24..27] (float) chamberOffset2
	// [28..29] (uint16) chamberThermistorBeta
	// [30..31] (uint16) rejectThermistorBeta
	// [32]		(uint8) runSelfTestOnPOR
	// [33..36]	(float) ps1.phpOutputDutyOffset
	// [37..40]	(float) ps2.phpOutputDutyOffset
	// [41..44] (float) maxAllowedChamberTemp
	// [45..48] (float) minAllowedChamberTemp
	// [49..52] (uint32) doorOpenLifetimeCnt
	// [53]     (uint8) doorOpenCount1HrLimit
				
	// PAGE1
	// [0..12]  (string) model
	// [13..19] (string) ui fw ver string
	
	// PAGE2
	// [0..15]  (string) serial
	
	if (eepromDataInfo.pAddr == &chamber1.chamberOffset)
	{
		pTempEepromBuf = floatToByteArray((float)chamber1.chamberOffset);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[0] = *(pTempEepromBuf + 0);
		eepromPageData[1] = *(pTempEepromBuf + 1);
		eepromPageData[2] = *(pTempEepromBuf + 2);
		eepromPageData[3] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_CHAMBEROFFSET_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &chamber1.glycolOffset)
	{
		pTempEepromBuf = floatToByteArray((float)chamber1.glycolOffset);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[4] = *(pTempEepromBuf + 0);
		eepromPageData[5] = *(pTempEepromBuf + 1);
		eepromPageData[6] = *(pTempEepromBuf + 2);
		eepromPageData[7] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_GLYCOLOFFSET_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &defrostFreqMaxHrs)
	{
		pTempEepromBuf = intToByteArray((uint16_t)defrostFreqMaxHrs);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[8]  = *(pTempEepromBuf + 0);
		eepromPageData[9]  = *(pTempEepromBuf + 1);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_DEFROSTFREQMAXHRS_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &defrostFreqHrs)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[10]  = defrostFreqHrs;
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_DEFROSTFREQHRS_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &defrostDurationMins)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[11]  = defrostDurationMins;
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_DEFROSTDURATIONMINS_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &mfgTestModeFlag1)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[22]  = mfgTestModeFlag1;
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_MFGTESTFLAG1_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &mfgTestModeFlag2)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[23]  = mfgTestModeFlag2;
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_MFGTESTFLAG2_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &chamber2.chamberOffset)
	{
		pTempEepromBuf = floatToByteArray((float)chamber2.chamberOffset);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[24] = *(pTempEepromBuf + 0);
		eepromPageData[25] = *(pTempEepromBuf + 1);
		eepromPageData[26] = *(pTempEepromBuf + 2);
		eepromPageData[27] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_CHAMBER2OFFSET_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &chamberThermistorBeta)
	{
		pTempEepromBuf = intToByteArray((uint16_t)chamberThermistorBeta);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[28]  = *(pTempEepromBuf + 0);
		eepromPageData[29]  = *(pTempEepromBuf + 1);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_CHAMBERBETA_WRITTEN;
		//dbg("\neeprom programmed");
	}
	else if (eepromDataInfo.pAddr == &rejectThermistorBeta)
	{
		pTempEepromBuf = intToByteArray((uint16_t)rejectThermistorBeta);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[30]  = *(pTempEepromBuf + 0);
		eepromPageData[31]  = *(pTempEepromBuf + 1);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_REJECTBETA_WRITTEN;
		//dbg("\neeprom programmed");
	}	
	else if (eepromDataInfo.pAddr == &runSelfTestOnPOR)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[32]  = runSelfTestOnPOR;
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_RUNSELFTESTFLAG_WRITTEN;
	}
	else if (eepromDataInfo.pAddr == &ps1.phpOutputDutyOffset)
	{
		pTempEepromBuf = floatToByteArray((float)ps1.phpOutputDutyOffset);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[33] = *(pTempEepromBuf + 0);
		eepromPageData[34] = *(pTempEepromBuf + 1);
		eepromPageData[35] = *(pTempEepromBuf + 2);
		eepromPageData[36] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_PS1OUTPUTDUTYOFFSET_WRITTEN;
		//dbg("\neeprom programmed");
	}	
	else if (eepromDataInfo.pAddr == &ps2.phpOutputDutyOffset)
	{
		pTempEepromBuf = floatToByteArray((float)ps2.phpOutputDutyOffset);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[37] = *(pTempEepromBuf + 0);
		eepromPageData[38] = *(pTempEepromBuf + 1);
		eepromPageData[39] = *(pTempEepromBuf + 2);
		eepromPageData[40] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_PS2OUTPUTDUTYOFFSET_WRITTEN;
		//dbg("\neeprom programmed");
	}	
	else if (eepromDataInfo.pAddr == &maxAllowedChamberTemp)
	{
		pTempEepromBuf = floatToByteArray((float)maxAllowedChamberTemp);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[41] = *(pTempEepromBuf + 0);
		eepromPageData[42] = *(pTempEepromBuf + 1);
		eepromPageData[43] = *(pTempEepromBuf + 2);
		eepromPageData[44] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_MAXALLOWEDTEMP_WRITTEN;
		//dbg("\neeprom programmed");
	}	
	else if (eepromDataInfo.pAddr == &minAllowedChamberTemp)
	{
		pTempEepromBuf = floatToByteArray((float)minAllowedChamberTemp);
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[45] = *(pTempEepromBuf + 0);
		eepromPageData[46] = *(pTempEepromBuf + 1);
		eepromPageData[47] = *(pTempEepromBuf + 2);
		eepromPageData[48] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_MINALLOWEDTEMP_WRITTEN;
		//dbg("\neeprom programmed");
	}	
	else if (eepromDataInfo.pAddr == &doorOpenLifetimeCnt)
	{
		pTempEepromBuf = longToByteArray((uint32_t)(doorOpenLifetimeCnt));
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[49] = *(pTempEepromBuf + 0);
		eepromPageData[50] = *(pTempEepromBuf + 1);
		eepromPageData[51] = *(pTempEepromBuf + 2);
		eepromPageData[52] = *(pTempEepromBuf + 3);
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_DOOROPENLIFETIMECNT_WRITTEN;
		//dbg("\neeprom programmed");
	}	
	else if (eepromDataInfo.pAddr == &doorOpenCount1HrLimit)
	{
		eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
		eepromPageData[53]  = doorOpenCount1HrLimit;
		eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
		eeprom_emulator_commit_page_buffer();
		eepromStatus |= EEPROM_DOOROPENCOUNT1HRLIMIT_WRITTEN;
		//dbg("\neeprom programmed");
	}

	writeToEepromFlag = false;
	
	eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
	eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
	eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
	eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
	eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
	eeprom_emulator_commit_page_buffer();
}