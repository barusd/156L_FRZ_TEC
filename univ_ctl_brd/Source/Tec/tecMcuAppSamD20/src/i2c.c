/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "i2cProtocol.h"
#include "extern.h"

/************************************************************************/
/* Module Instances                                                     */
/************************************************************************/
struct i2c_master_module i2c_master_instance;
struct i2c_slave_module i2c_slave_instance;
struct i2c_packet wr_packet;
struct i2c_packet rd_packet;

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
// i2c slave
uint8_t i2cRxBufr[RX_BUFR_SIZE] = {0};
//uint8_t i2cTxBufr[RX_BUFR_SIZE] = {0};
uint8_t i2cBytesRecevied = 0;
uint8_t i2cZone = 0;
uint8_t i2cIndex = 0;
uint8_t i2cCRC = 0;
uint8_t i2cWriteEn = 0;
uint8_t i2cNegative = 0;
uint8_t regArray[REGARRAY_SIZE] = {0};
enum i2c_slave_direction dir;
uint32_t errorCRC = 0;

// i2c master
uint8_t i2c_txdat[F_DATA_LENGTH] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t i2c_rxdat[F_DATA_LENGTH] = {0x00};

/************************************************************************/
/* Callback Functions                                                   */
/************************************************************************/

// i2c packet definition for ui communication
//
//   [S][DEV_ADDR][R/W][ACK][LOW_BYTE][HIGH_BYTE][PARAM_ID][FLAGS][CRC][P]
//

// read request from master
// master reads, slave writes
// slave writes data on bus back to master
void i2c_read_request_callback(struct i2c_slave_module *const module)
{
	/* Init i2c packet. */
	wr_packet.data_length = RX_BUFR_SIZE;
	wr_packet.data = regArray;
	
	//float tmpValue;
	//Wire.write(&regArray[i2cIndex*2], 2);
	regArray[FLAGS_INDEX] = ZONE1_FLAG;
	regArray[HIGH_BYTE_INDEX] = 0;
	
	// lookup mac addr
	if (i2cIndex>=IDX_MAC_ADDR && i2cIndex<=IDX_MAC_ADDR+MAC_ADDR_SIZE-1)
	{
		regArray[LOW_BYTE_INDEX] = MAC_address[i2cIndex-IDX_MAC_ADDR];
	}
	// lookup serial number
	else if (i2cIndex>=IDX_SERIAL && i2cIndex<=IDX_SERIAL+SERIAL_SIZE-1)
	{
		regArray[LOW_BYTE_INDEX] = strSerial[i2cIndex-IDX_SERIAL];
	}
	// lookup model number
	else if (i2cIndex>=IDX_MODEL && i2cIndex<=IDX_MODEL+MODEL_SIZE-1)
	{
		regArray[LOW_BYTE_INDEX] = strModel[i2cIndex-IDX_MODEL];
	}
	// lookup ui fw ver string
	else if (i2cIndex>=IDX_UIFWVER && i2cIndex<=IDX_UIFWVER+UIFWVER_SIZE-1)
	{
		regArray[LOW_BYTE_INDEX] = strUiFwVer[i2cIndex-IDX_UIFWVER];
	}	
	// lookup other variable
	else
	{
		struct _varInfo varInfo = paramLookup(i2cIndex);
	
		if (varInfo.size == sizeof(uint8_t))
		{
			regArray[LOW_BYTE_INDEX] = *(uint8_t*)(varInfo.pAddr);

		} 
		else if (varInfo.size == sizeof(uint16_t))
		{
			uint16_t tmpValue = *(uint16_t*)(varInfo.pAddr);
			regArray[LOW_BYTE_INDEX] = (uint8_t)tmpValue;
			regArray[HIGH_BYTE_INDEX] = (uint8_t)(tmpValue>>8);
		}
		else if (varInfo.size == sizeof(float))
		{
			float tmpValue = *(float*)(varInfo.pAddr);
			if (tmpValue < 0)
			{
				regArray[FLAGS_INDEX] |= SIGN_NEGATIVE_FLAG;
				tmpValue = -1.0 * tmpValue; //abs(tmpValue);
			}
			regArray[LOW_BYTE_INDEX] = (uint8_t)(tmpValue);
			regArray[DECIMAL_BYTE_INDEX] = round((float)((tmpValue - (float)((uint8_t)(tmpValue)))) * 100);		
		}
		else
		{
			// interpret as uint8_t by default
			regArray[LOW_BYTE_INDEX] = *(uint8_t*)(varInfo.pAddr);
		}
	}
	
	regArray[PARAMETER_ID_INDEX] = i2cIndex;	// i2cIndex was set in the preceeding write, whether write was enabled or not
	regArray[CRC_INDEX] = calcCRC(regArray, PROTOCOL_BUFR_SIZE - 1);
	/*
	dbg_info("\n[i2c slv txdat: %x %x %x %x %x]",
		regArray[0], regArray[1], regArray[2], regArray[3], regArray[4]);
	*/
	/* Write buffer to master */
	i2c_slave_write_packet_job(module, &wr_packet);

}

// write request from master
// master writes, slave receives
// slave reads data from master off of bus
void i2c_write_request_callback(struct i2c_slave_module *const module)
{
	/* Init i2c packet. */
	rd_packet.data_length = RX_BUFR_SIZE;
	rd_packet.data = i2cRxBufr;
	
	/* Read buffer from master */
	if (i2c_slave_read_packet_job(module, &rd_packet) != STATUS_OK) {
	}
	
}

void i2c_buffer_full_request_callback(struct i2c_slave_module *const module)
{
	
}

// master writes, slave has finished receiving all data
// parse data and store to memory
void i2c_buffer_ready_request_callback(struct i2c_slave_module *const module)
{
	//float tmpValue = 0.0f;

	i2cBytesRecevied  = rd_packet.data_length;
	/*
	dbg_info("\n[i2c slv rxdat: %x %x %x %x %x]",
		i2cRxBufr[0], i2cRxBufr[1], i2cRxBufr[2], i2cRxBufr[3], i2cRxBufr[4]);
	*/
	if (checkCRC(i2cRxBufr, i2cBytesRecevied))
	{
		i2cZone = (i2cRxBufr[FLAGS_INDEX] & (ZONE1_FLAG)); // | ZONE2_FLAG));	// 1, 2, or both = 3
		i2cIndex = i2cRxBufr[PARAMETER_ID_INDEX];  // index will be used for following read, whether write is done here or not
		if ((i2cRxBufr[FLAGS_INDEX] & WRITE_ENABLE_FLAG) != 0)
		{
			// index mac addr
			if (i2cIndex>=IDX_MAC_ADDR && i2cIndex<=IDX_MAC_ADDR+MAC_ADDR_SIZE-1)
			{
				MAC_address[i2cIndex-IDX_MAC_ADDR] = i2cRxBufr[LOW_BYTE_INDEX];
				eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
				eepromPageData[12] = MAC_address[0];
				eepromPageData[13] = MAC_address[1];
				eepromPageData[14] = MAC_address[2];
				eepromPageData[15] = MAC_address[3];
				eepromPageData[16] = MAC_address[4];
				eepromPageData[17] = MAC_address[5];
				eepromStatus |= EEPROM_MACADDR_WRITTEN;
				eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
				eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
				eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
				eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
				eeprom_emulator_commit_page_buffer();
			}
			else if (i2cIndex>=IDX_UIFWVER && i2cIndex<=IDX_UIFWVER+UIFWVER_SIZE-2)
			{
				strUiFwVer[i2cIndex-IDX_UIFWVER] = i2cRxBufr[LOW_BYTE_INDEX];
				eeprom_emulator_read_page(EEPROM_PAGE1, eepromPageData);
				eepromPageData[13] = strUiFwVer[0];
				eepromPageData[14] = strUiFwVer[1];
				eepromPageData[15] = strUiFwVer[2];
				eepromPageData[16] = strUiFwVer[3];
				eepromPageData[17] = strUiFwVer[4];
				eepromPageData[18] = strUiFwVer[5];
				//eepromPageData[19] = strUiFwVer[6];				
				eepromStatus |= EEPROM_UIFWVER_WRITTEN;
				eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
				eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
				eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
				eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
				eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
				eeprom_emulator_commit_page_buffer();
			}			
			else if (uiSetpointControl==false && i2cIndex==IDX_SETPOINT)
			{
				return;
			}
			else
			{			
				struct _varInfo varInfo = paramLookup(i2cIndex);
	
				if (varInfo.size == sizeof(uint8_t))
				{
					*(uint8_t*)(varInfo.pAddr) = i2cRxBufr[LOW_BYTE_INDEX];
				}
				else if (varInfo.size == sizeof(uint16_t))
				{
					uint16_t tmpValue = regArray[LOW_BYTE_INDEX];
					tmpValue += ((uint16_t)regArray[HIGH_BYTE_INDEX])<<8;
					*(uint16_t*)(varInfo.pAddr) = tmpValue;
				}
				else if (varInfo.size == sizeof(float))
				{
					float tmpValue = ((float)(i2cRxBufr[LOW_BYTE_INDEX]) + (float)((float)(i2cRxBufr[DECIMAL_BYTE_INDEX])/(100)));
					if ((i2cRxBufr[FLAGS_INDEX] & SIGN_NEGATIVE_FLAG) != 0)
					{
						tmpValue *= -1;
					}
					*(float*)(varInfo.pAddr) = tmpValue;
				}
				else
				{
					// interpret as uint8_t by default
					*(uint8_t*)(varInfo.pAddr) = i2cRxBufr[LOW_BYTE_INDEX];
				}
			
				if (varInfo.nonvolatileFlag == true)
				{
					writeToEepromFlag = true;
					eepromDataInfo = varInfo;
				}
			}
		}
	}
	else
	{
		errorCRC++;
	}
	/*
	dbg_info(" [errorCRC: %i]", errorCRC);
	*/
	module->buffer_remaining = module->buffer_length;
	
}

void i2c_error_request_callback(struct i2c_slave_module *const module)
{
	//put something in here to alert of I2C error
	
}

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
void configure_i2c_master(void)
{
	/* Initialize config structure and software module. */
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	
	/* Change buffer timeout to something longer. */
	config_i2c_master.buffer_timeout = 10000;
	config_i2c_master.generator_source = GCLK_GENERATOR_0;
	config_i2c_master.baud_rate = I2C_MASTER_BAUD_RATE_400KHZ;
	config_i2c_master.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0;
	config_i2c_master.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1;

	/* Initialize and enable device with config. */
	i2c_master_init(&i2c_master_instance, SERCOM1, &config_i2c_master);
	i2c_master_enable(&i2c_master_instance);
}

void configure_i2c_slave(void)
{
	/* Initialize config structure and module instance. */
	struct i2c_slave_config config_i2c_slave;
	i2c_slave_get_config_defaults(&config_i2c_slave);
	/* Change address and address_mode. */
	config_i2c_slave.address = SLAVE_ADDRESS;
	config_i2c_slave.address_mode = I2C_SLAVE_ADDRESS_MODE_MASK;
	config_i2c_slave.buffer_timeout = 20000;
	config_i2c_slave.sda_hold_time = I2C_SLAVE_SDA_HOLD_TIME_50NS_100NS;
	
	/* Initialize and enable device with config. */
	i2c_slave_init(&i2c_slave_instance, SERCOM5, &config_i2c_slave);
	i2c_slave_enable(&i2c_slave_instance);
}

void configure_i2c_slave_callbacks(void)
{
	/* Register and enable callback functions */
	i2c_slave_register_callback(&i2c_slave_instance, i2c_read_request_callback,
	I2C_SLAVE_CALLBACK_READ_REQUEST);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_READ_REQUEST);
	i2c_slave_register_callback(&i2c_slave_instance, i2c_write_request_callback,
	I2C_SLAVE_CALLBACK_WRITE_REQUEST);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_WRITE_REQUEST);
	i2c_slave_register_callback(&i2c_slave_instance, i2c_buffer_ready_request_callback,
	I2C_SLAVE_CALLBACK_WRITE_COMPLETE);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_WRITE_COMPLETE);
	i2c_slave_register_callback(&i2c_slave_instance, i2c_buffer_full_request_callback,
	I2C_SLAVE_CALLBACK_READ_COMPLETE);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_READ_COMPLETE);
	i2c_slave_register_callback(&i2c_slave_instance, i2c_error_request_callback,
	I2C_SLAVE_CALLBACK_ERROR);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_ERROR);
}

