/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "i2cProtocol.h"
#include "app.h"
#include "extern.h"
#include "dbgCmds.h"

/************************************************************************/
/* Globals                                                              */
/************************************************************************/
struct pid_instance pidLoop1;
struct pid_instance pidLoop2;
struct pid_instance pidFan1;
struct pid_instance pidFan2;
struct pid_instance pidFan4;
struct chamber_vars chamber1;
struct chamber_vars chamber2;
struct ps_vars ps1;
struct ps_vars ps2;
//struct php_vars php1;
struct fan_vars fan1;
struct fan_vars fan2;
struct fan_vars fan3;
struct fan_vars fan4;
struct fan_vars fan5;
struct fan_vars fan6;
struct fan_vars fan7;
struct fan_vars fan8;
struct heater_vars condHeater;
struct heater_vars doorHeater;
struct heater_vars htxAHeater;
struct heater_vars htxBHeater;

uint8_t firmwareVersion;
uint8_t firmwareRevision;
uint16_t defrostFreqMaxHrs;
uint8_t i2cFunctionReg = 0;
uint16_t errorRegister = 0x0000;
float maxOutputVoltage = MAX_OUTPUT_VOLTAGE_DEFAULT;
uint8_t mfgTestModeFlag1 = 0x01;
uint8_t mfgTestModeFlag2 = 0x01;
uint8_t UIfwVersion = 0x00;
uint8_t doorOpenStatus = 0x00;
float ambientTemp = DEFAULT_AMBIENT_TEMP;
bool uiSetpointControl = true;
uint16_t pwrCapTimeoutInMins = DEFAULT_PWR_CAP_TIMEOUT_MINS;
uint16_t doorOpenLifetimeCnt_hi = 0;
uint16_t doorOpenLifetimeCnt_lo = 0;

/************************************************************************/
/* Functions                                                            */
/************************************************************************/
struct _varInfo paramLookup(uint8_t idx)
{
	struct _varInfo varInfo;
	
	varInfo.nonvolatileFlag = false;
	
	doorOpenLifetimeCnt_hi = (doorOpenLifetimeCnt & 0xffff0000)>>16;
	doorOpenLifetimeCnt_lo = (doorOpenLifetimeCnt & 0x0000ffff);
	
	switch (idx)
	{		
		case 1:		varInfo.pAddr = &chamber1.setpoint;				varInfo.size = sizeof(chamber1.setpoint);				break;
		case 2:		varInfo.pAddr = &chamber2.chamberOffset;		varInfo.size = sizeof(chamber2.chamberOffset);			varInfo.nonvolatileFlag = true;		break;
		case 3:		varInfo.pAddr = &chamber1.glycolOffset;			varInfo.size = sizeof(chamber1.glycolOffset);			varInfo.nonvolatileFlag = true;		break;	
		case 4:		varInfo.pAddr = &chamber1.mode_state;			varInfo.size = sizeof(chamber1.mode_state);				break;
		case 5:		varInfo.pAddr = &controlTemperature;			varInfo.size = sizeof(controlTemperature);				break;			
		case 6:		varInfo.pAddr = &chamber1.rejectTemp;			varInfo.size = sizeof(chamber1.rejectTemp);				break;			
		case 7:		varInfo.pAddr = &chamber1.VmaxCOP;				varInfo.size = sizeof(chamber1.VmaxCOP);				break;			
		case 8:		varInfo.pAddr = &chamber1.Vvarq;				varInfo.size = sizeof(chamber1.Vvarq);					break;			
		case 9:		varInfo.pAddr = &chamber1.rejectLimit;			varInfo.size = sizeof(chamber1.rejectLimit);			break;			
		case 10:	varInfo.pAddr = &chamber1.rejectLimitCritical;	varInfo.size = sizeof(chamber1.rejectLimitCritical);	break;			
		case 11:	varInfo.pAddr = &chamber1.rejectLowLimit;		varInfo.size = sizeof(chamber1.rejectLowLimit);			break;			
		case 12:	varInfo.pAddr = &chamber1.deadband;				varInfo.size = sizeof(chamber1.deadband);				break;			
		case 13:	varInfo.pAddr = &chamber1.deadbandLow;			varInfo.size = sizeof(chamber1.deadbandLow);			break;			
		case 14:	varInfo.pAddr = &chamber1.deactivateAutoControl;varInfo.size = sizeof(chamber1.deactivateAutoControl);	break;			
		case 15:	varInfo.pAddr = &chamber1.glycolTemp;			varInfo.size = sizeof(chamber1.glycolTemp);				break;						
		case 16:	varInfo.pAddr = &pidLoop1.kp_highq;				varInfo.size = sizeof(pidLoop1.kp_highq);				break;
		case 17:	varInfo.pAddr = &pidLoop1.ki_highq;				varInfo.size = sizeof(pidLoop1.ki_highq);				break;
		case 18:	varInfo.pAddr = &pidLoop1.kd_highq;				varInfo.size = sizeof(pidLoop1.kd_highq);				break;		
		case 19:	varInfo.pAddr = &pidLoop1.kp_varq;				varInfo.size = sizeof(pidLoop1.kp_varq);				break;
		case 20:	varInfo.pAddr = &pidLoop1.ki_varq;				varInfo.size = sizeof(pidLoop1.ki_varq);				break;
		case 21:	varInfo.pAddr = &pidLoop1.kd_varq;				varInfo.size = sizeof(pidLoop1.kd_varq);				break;		
		case 22:	varInfo.pAddr = &pidLoop1.kp_copq;				varInfo.size = sizeof(pidLoop1.kp_copq);				break;		
		case 23:	varInfo.pAddr = &pidLoop1.ki_copq;				varInfo.size = sizeof(pidLoop1.ki_copq);				break;		
		case 24:	varInfo.pAddr = &pidLoop1.kd_copq;				varInfo.size = sizeof(pidLoop1.kd_copq);				break;		
		case 25:	varInfo.pAddr = &ps1.desiredVoltage;			varInfo.size = sizeof(ps1.desiredVoltage);				break;		
		case 26:	varInfo.pAddr = &ps1.phpVoltage;				varInfo.size = sizeof(ps1.phpVoltage);					break;		
		case 27:	varInfo.pAddr = &ps1.outputCurrent;				varInfo.size = sizeof(ps1.outputCurrent);				break;		
		case 28:	varInfo.pAddr = &ps1.bulkVoltage;				varInfo.size = sizeof(ps1.bulkVoltage);					break;		
		//case 29:	varInfo.pAddr = &fan1.Vtop;						varInfo.size = sizeof(fan1.Vtop);						break;		
		//case 30:	varInfo.pAddr = &fan1.Vbottom;					varInfo.size = sizeof(fan1.Vbottom);					break;		
		case 31:	varInfo.pAddr = &fan1.minDuty;					varInfo.size = sizeof(fan1.minDuty);					break;		
		case 32:	varInfo.pAddr = &fan1.maxDuty;					varInfo.size = sizeof(fan1.maxDuty);					break;		
		case 33:	varInfo.pAddr = &fan1.rpm;						varInfo.size = sizeof(fan1.rpm);						break;		
		case 34:	varInfo.pAddr = &fan1.state;					varInfo.size = sizeof(fan1.state);						break;	
		//case 35:	varInfo.pAddr = &fan2.Vtop;						varInfo.size = sizeof(fan2.Vtop);						break;
		//case 36:	varInfo.pAddr = &fan2.Vbottom;					varInfo.size = sizeof(fan2.Vbottom);					break;
		case 37:	varInfo.pAddr = &fan2.minDuty;					varInfo.size = sizeof(fan2.minDuty);					break;
		case 38:	varInfo.pAddr = &fan2.maxDuty;					varInfo.size = sizeof(fan2.maxDuty);					break;
		case 39:	varInfo.pAddr = &fan2.rpm;						varInfo.size = sizeof(fan2.rpm);						break;
		case 40:	varInfo.pAddr = &fan2.state;					varInfo.size = sizeof(fan2.state);						break;				
		case 41:	varInfo.pAddr = &firmwareVersion;				varInfo.size = sizeof(firmwareVersion);					break;		
		case 42:	varInfo.pAddr = &defrostFreqHrs;				varInfo.size = sizeof(defrostFreqHrs);					varInfo.nonvolatileFlag = true;		break;		
		case 43:	varInfo.pAddr = &defrostDurationMins;			varInfo.size = sizeof(defrostDurationMins);				varInfo.nonvolatileFlag = true;		break;		
		case 44:	varInfo.pAddr = &defrostCycleCount;				varInfo.size = sizeof(defrostCycleCount);				break;		
		case 45:	varInfo.pAddr = &defrostActive;					varInfo.size = sizeof(defrostActive);					break;		
		case 46:	varInfo.pAddr = &enableAutoDefrost;				varInfo.size = sizeof(enableAutoDefrost);				break;		
		case 47:	varInfo.pAddr = &defrostFreqMaxHrs;				varInfo.size = sizeof(defrostFreqMaxHrs);				varInfo.nonvolatileFlag = true;		break;		
		case 48:	varInfo.pAddr = &i2cFunctionReg;				varInfo.size = sizeof(i2cFunctionReg);					break;	
		case 49:	varInfo.pAddr = &ps1.standbyVoltage;			varInfo.size = sizeof(ps1.standbyVoltage);				break;	
			
		// MAC Address	
		// reserved for mac address bytes
		// 50 to 55
		
		// Serial Number	
		// reserved for serial number bytes																										
		// 56 to 70
			
		// Model Number
		// reserved for model number bytes
		// 71 to 83

		case 84:	varInfo.pAddr = &errorRegister;					varInfo.size = sizeof(errorRegister);					break;
		case 85:	varInfo.pAddr = &ps1.logicVoltage;				varInfo.size = sizeof(ps1.logicVoltage);				break;	
		case 86:	varInfo.pAddr = &maxOutputVoltage;				varInfo.size = sizeof(maxOutputVoltage);				break;
		case 87:	varInfo.pAddr = &ps1.batteryVoltage;			varInfo.size = sizeof(ps1.batteryVoltage);				break;	
		case 88:	varInfo.pAddr = &mfgTestModeFlag1;				varInfo.size = sizeof(mfgTestModeFlag1);				varInfo.nonvolatileFlag = true;		break;	
		case 89:	varInfo.pAddr = &mfgTestModeFlag2;				varInfo.size = sizeof(mfgTestModeFlag2);				varInfo.nonvolatileFlag = true;		break;	
		// case 90:			
		case 91:	varInfo.pAddr = &chamber1.chamberTemp;			varInfo.size = sizeof(chamber1.chamberTemp);			break;
		case 92:	varInfo.pAddr = &chamber2.chamberTemp;			varInfo.size = sizeof(chamber2.chamberTemp);			break;
		case 93:	varInfo.pAddr = &chamber2.rejectTemp;			varInfo.size = sizeof(chamber2.rejectTemp);				break;
		case 94:	varInfo.pAddr = &ps1.outputPower;				varInfo.size = sizeof(ps1.outputPower);					break;
		case 95:	varInfo.pAddr = &ps2.phpVoltage;				varInfo.size = sizeof(ps2.phpVoltage);					break;
		case 96:	varInfo.pAddr = &ps2.outputCurrent;				varInfo.size = sizeof(ps2.outputCurrent);				break;
		case 97:	varInfo.pAddr = &ps2.outputPower;				varInfo.size = sizeof(ps2.outputPower);					break;
		case 98:	varInfo.pAddr = &fan1.targetDutyPercent;		varInfo.size = sizeof(fan1.targetDutyPercent);			break;
		case 99:	varInfo.pAddr = &fan2.state;					varInfo.size = sizeof(fan2.state);						break;
		case 100:	varInfo.pAddr = &fan2.targetDutyPercent;		varInfo.size = sizeof(fan2.targetDutyPercent);			break;
		case 101:	varInfo.pAddr = &fan2.rpm;						varInfo.size = sizeof(fan2.rpm);						break;
		case 102:	varInfo.pAddr = &fan3.state;					varInfo.size = sizeof(fan3.state);						break;
		case 103:	varInfo.pAddr = &fan3.targetDutyPercent;		varInfo.size = sizeof(fan3.targetDutyPercent);			break;
		case 104:	varInfo.pAddr = &fan3.rpm;						varInfo.size = sizeof(fan3.rpm);						break;
		case 105:	varInfo.pAddr = &fan4.state;					varInfo.size = sizeof(fan4.state);						break;
		case 106:	varInfo.pAddr = &fan4.targetDutyPercent;		varInfo.size = sizeof(fan4.targetDutyPercent);			break;
		case 107:	varInfo.pAddr = &fan4.rpm;						varInfo.size = sizeof(fan4.rpm);						break;
		case 108:	varInfo.pAddr = &ps1.outputStatus;				varInfo.size = sizeof(ps1.outputStatus);				break;
		case 109:	varInfo.pAddr = &ps1.outputDuty;				varInfo.size = sizeof(ps1.outputDuty);					break;
		case 110:	varInfo.pAddr = &ps2.outputStatus;				varInfo.size = sizeof(ps2.outputStatus);				break;
		case 111:	varInfo.pAddr = &ps2.outputDuty;				varInfo.size = sizeof(ps2.outputDuty);					break;
		//case 112:	varInfo.pAddr = &humidityPercentage;			varInfo.size = sizeof(humidityPercentage);				break;
		case 113:	varInfo.pAddr = &condensateTemp;				varInfo.size = sizeof(condensateTemp);					break;
		case 114:	varInfo.pAddr = &UIfwVersion;					varInfo.size = sizeof(UIfwVersion);						break;
		case 115:	varInfo.pAddr = &doorOpenStatus;				varInfo.size = sizeof(doorOpenStatus);					break;
		case 116:	varInfo.pAddr = &ambientTemp;					varInfo.size = sizeof(ambientTemp);						break;
		case 117:	varInfo.pAddr = &maxAllowedChamberTemp;			varInfo.size = sizeof(maxAllowedChamberTemp);			varInfo.nonvolatileFlag = true;		break;
		case 118:	varInfo.pAddr = &minAllowedChamberTemp;			varInfo.size = sizeof(minAllowedChamberTemp);			varInfo.nonvolatileFlag = true;		break;
		case 119:   varInfo.pAddr = &self_test_status_hi;			varInfo.size = sizeof(self_test_status_hi);				break;
		case 120:   varInfo.pAddr = &self_test_status_lo;			varInfo.size = sizeof(self_test_status_lo);				break;	
		case 121:	varInfo.pAddr = &selfTestIsRunning;				varInfo.size = sizeof(selfTestIsRunning);				break;	
		
		// UI Firmware Version String
		// reserved for ui firmware string bytes
		// 122 to 128	
		
		case 129:	varInfo.pAddr = &last_defrost_timestamp.time.month;	varInfo.size = sizeof(last_defrost_timestamp.time.month);	break;
		case 130:	varInfo.pAddr = &last_defrost_timestamp.time.day;	varInfo.size = sizeof(last_defrost_timestamp.time.day);		break;		
		case 131:	varInfo.pAddr = &last_defrost_timestamp.time.hour;	varInfo.size = sizeof(last_defrost_timestamp.time.hour);	break;		
		case 132:	varInfo.pAddr = &last_defrost_timestamp.time.minute;varInfo.size = sizeof(last_defrost_timestamp.time.minute);	break;
		case 133:	varInfo.pAddr = &last_defrost_timestamp.time.pm;	varInfo.size = sizeof(last_defrost_timestamp.time.pm);		break;					

		case 134:	varInfo.pAddr = &fan1.stall;					varInfo.size = sizeof(fan1.stall);						break;
		case 135:	varInfo.pAddr = &fan2.stall;					varInfo.size = sizeof(fan2.stall);						break;
		case 136:	varInfo.pAddr = &fan3.stall;					varInfo.size = sizeof(fan3.stall);						break;
		case 137:	varInfo.pAddr = &fan4.stall;					varInfo.size = sizeof(fan4.stall);						break;
		case 138:	varInfo.pAddr = &runSelfTestOnPOR;				varInfo.size = sizeof(runSelfTestOnPOR);				varInfo.nonvolatileFlag = true;		break;
		
		case 139:	varInfo.pAddr = &last_defrost_timestamp.time.second;	varInfo.size = sizeof(last_defrost_timestamp.time.second);	break;		

		case 140:	varInfo.pAddr = &fan1.fanType;					varInfo.size = sizeof(fan1.fanType);					break;
		case 141:	varInfo.pAddr = &fan2.fanType;					varInfo.size = sizeof(fan2.fanType);					break;
		case 142:	varInfo.pAddr = &fan3.fanType;					varInfo.size = sizeof(fan3.fanType);					break;
		case 143:	varInfo.pAddr = &fan4.fanType;					varInfo.size = sizeof(fan4.fanType);					break;
		case 144:	varInfo.pAddr = &maxSnsErrorCount;				varInfo.size = sizeof(maxSnsErrorCount);				break;
		case 145:	varInfo.pAddr = &chamberThermistorBeta;			varInfo.size = sizeof(chamberThermistorBeta);			varInfo.nonvolatileFlag = true;		break;
		case 146:	varInfo.pAddr = &rejectThermistorBeta;			varInfo.size = sizeof(rejectThermistorBeta);			varInfo.nonvolatileFlag = true;		break;
		case 147:	varInfo.pAddr = &cha1SnsErrCnt;					varInfo.size = sizeof(cha1SnsErrCnt);					break;
		case 148:	varInfo.pAddr = &cha2SnsErrCnt;					varInfo.size = sizeof(cha2SnsErrCnt);					break;
		case 149:	varInfo.pAddr = &rejSnsErrCnt;					varInfo.size = sizeof(rejSnsErrCnt);					break;
		case 150:	varInfo.pAddr = &rejCritErrCnt;					varInfo.size = sizeof(rejCritErrCnt);					break;
		case 151:	varInfo.pAddr = &maxFanErrorCount;				varInfo.size = sizeof(maxFanErrorCount);				break;
		case 152:	varInfo.pAddr = &fanSpdTolerance;				varInfo.size = sizeof(fanSpdTolerance);					break;
		case 153:	varInfo.pAddr = &fan1ErrorCount;				varInfo.size = sizeof(fan1ErrorCount);					break;
		case 154:	varInfo.pAddr = &fan2ErrorCount;				varInfo.size = sizeof(fan2ErrorCount);					break;
		case 155:	varInfo.pAddr = &fan4ErrorCount;				varInfo.size = sizeof(fan4ErrorCount);					break;		
		case 156:	varInfo.pAddr = &fan1LowSpdErrCnt;				varInfo.size = sizeof(fan1LowSpdErrCnt);				break;
		case 157:	varInfo.pAddr = &fan2LowSpdErrCnt;				varInfo.size = sizeof(fan2LowSpdErrCnt);				break;
		case 158:	varInfo.pAddr = &fan4LowSpdErrCnt;				varInfo.size = sizeof(fan4LowSpdErrCnt);				break;
		case 159:	varInfo.pAddr = &maxPhpErrorCount;				varInfo.size = sizeof(maxPhpErrorCount);				break;
		case 160:	varInfo.pAddr = &phpNominalResistance;			varInfo.size = sizeof(phpNominalResistance);			break;
		case 161:	varInfo.pAddr = &phpResTolPercentage;			varInfo.size = sizeof(phpResTolPercentage);				break;
		case 162:	varInfo.pAddr = &php1TotalErrorCount;			varInfo.size = sizeof(php1TotalErrorCount);				break;
		case 163:	varInfo.pAddr = &php2TotalErrorCount;			varInfo.size = sizeof(php2TotalErrorCount);				break;
		case 164:	varInfo.pAddr = &php1CurrentLwrLimSetting;		varInfo.size = sizeof(php1CurrentLwrLimSetting);		break;
		case 165:	varInfo.pAddr = &php2CurrentLwrLimSetting;		varInfo.size = sizeof(php2CurrentLwrLimSetting);		break;
		case 166:   varInfo.pAddr = &enablePhpCurrentToleranceCheck;varInfo.size = sizeof(enablePhpCurrentToleranceCheck);	break;		
 		case 167:   varInfo.pAddr = &phpChk_lastI1lim;				varInfo.size = sizeof(phpChk_lastI1lim);				break;
 		case 168:   varInfo.pAddr = &phpChk_lastI1;					varInfo.size = sizeof(phpChk_lastI1);					break;
 		case 169:   varInfo.pAddr = &phpChk_lastV1;					varInfo.size = sizeof(phpChk_lastV1);					break;
 		case 170:   varInfo.pAddr = &phpChk_lastI2lim;				varInfo.size = sizeof(phpChk_lastI2lim);				break;
 		case 171:   varInfo.pAddr = &phpChk_lastI2;					varInfo.size = sizeof(phpChk_lastI2);					break;
 		case 172:   varInfo.pAddr = &phpChk_lastV2;					varInfo.size = sizeof(phpChk_lastV2);					break;
		case 173:	varInfo.pAddr = &phpChkTimeIntervalInSecs;		varInfo.size = sizeof(phpChkTimeIntervalInSecs);		break;
		case 174:	varInfo.pAddr = &firmwareRevision;				varInfo.size = sizeof(firmwareRevision);				break;
		case 175:	varInfo.pAddr = &defrostCancelTemp;				varInfo.size = sizeof(defrostCancelTemp);				break;
		case 176:	varInfo.pAddr = &pwrCapTimeoutInMins;			varInfo.size = sizeof(pwrCapTimeoutInMins);				break;
		case 177:	varInfo.pAddr = &php1CurrentDiffLim;			varInfo.size = sizeof(php1CurrentDiffLim);				break;
		case 178:	varInfo.pAddr = &php2CurrentDiffLim;			varInfo.size = sizeof(php2CurrentDiffLim);				break;	
		case 179:   varInfo.pAddr = &doorOpenLifetimeCnt_hi;		varInfo.size = sizeof(doorOpenLifetimeCnt_hi);			break;			
		case 180:   varInfo.pAddr = &doorOpenLifetimeCnt_lo;		varInfo.size = sizeof(doorOpenLifetimeCnt_lo);			break;
		case 181:   varInfo.pAddr = &highUsageModeFlag;				varInfo.size = sizeof(highUsageModeFlag);				break;
		case 182:   varInfo.pAddr = &doorOpenCount1HrLimit;			varInfo.size = sizeof(doorOpenCount1HrLimit);			break;
				
		default:	varInfo.pAddr = &chamber1.setpoint;				varInfo.size = sizeof(chamber1.setpoint);				break;
	}
	
	return varInfo;
	
}

void i2cCommandHandler()
{
	dbg_info("\n[i2cFunction %x]", i2cFunctionReg);
			
	switch (i2cFunctionReg)
	{
		// read sensors (for battery backup)
		case 0xa5:
			port_pin_set_output_level_EX(PIN_RDY_N, HIGH, enableManualOverride);
			readSensors();
			port_pin_set_output_level_EX(PIN_RDY_N,  LOW, enableManualOverride);
			delay_ms(100);
			break;

#ifdef ENABLE_SLEEP_WAKE
		// initiate low power mode
		case 0xb5:
			lowPowerModeEnter();
			break;
#endif
		// initiate defrost
		case 0xc5:
			setDefrostState(DEFROST_START);
			break;

		// cancel defrost
		case 0xc6:
			setDefrostState(DEFROST_CANCEL);
			break;
		
		// reset defrost timer	
		case 0xc7:
			initDefrost();
			dbg("\n[defrost init]\n");
			break;
			
		// run selfTest
		case 0xca:
			selfTest();
			break;
			
		// run php check
		case 0xcc:
			//phpCheckCurrentOutOfBounds();
			break;	
				
		// reset
		case 0xd5:
			dbg("\n[soft reset now!]\n");
			system_reset();
			break;
				
		default:
			break;
	}
}
		
/*
uint16_t protocolType(uint8_t ptb)
{
	uint16_t retValue = 0;
	
	retValue = lookupType[ptb];
	
	return retValue;
}
*/

bool checkCRC (unsigned char* pInputBuf, uint32_t nNumBytes)
{
	uint8_t crcCheck = 0;
	uint32_t i;
	bool bCRCPass = false;
	
	if(nNumBytes == PROTOCOL_BUFR_SIZE)
	{
		//this needs to be quick
		for (i = 0; i < nNumBytes-1; i++)
		{
			crcCheck ^= pInputBuf[i];
		}
	
		if (crcCheck == pInputBuf[nNumBytes-1])
		{
			bCRCPass = true;
		}
	}
	
	return bCRCPass;
}

uint8_t calcCRC (unsigned char* pInputBuf, uint32_t nNumBytes)
{
	uint8_t crcCheck = 0;
	uint32_t i;
	//this needs to be quick
	for (i = 0; i < nNumBytes; i++)
	{
		crcCheck ^= pInputBuf[i];
	}
	
	return crcCheck;
}

