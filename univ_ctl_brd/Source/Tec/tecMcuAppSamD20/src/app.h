/*
* PHONONIC END-USER SOFTWARE LICENSE AGREEMENT
*
* LICENSE GRANT.  Phononic grants to the Customer (and/or it's agents), free of charge, the
* non-exclusive, non-transferable, non-sublicensable right (1) to use the Software, (2)
* to reproduce the Software, (3) to prepare derivative works of the Software, and (4)
* to distribute the Software and derivative works thereof in object (machine-readable)
* form as part of a programmable processing unit (e.g. a microcontroller) to be used in
* the exclusive support of thermoelectric device(s), heat pump or cooling system as supplied
* directly or indirectly from Phononic ("Phononic System").
*
* COPYRIGHT.  The Software is licensed to the Customer, not sold.  Phononic owns the Software,
* and United States copyright laws and international treaty provisions protect the Software.
* Therefore, the Customer must treat the Software like any other copyrighted material
* (e.g. a book or musical recording).  The Customer may not use or copy the Software for any
* other purpose than what is described in the Agreement.  Except as expressly provided herein,
* Phononic does not grant to the Customer any express or implied rights under any Phononic or
* third party patents, copyrights, trademarks, or trade secrets.  Additionally, the Customer
* must reproduce and apply any copyright or other proprietary rights notices included on
* or embedded in the Software to any copies made thereof, in whole or in part, if any.
* the Customer may not remove any copyright notices of Phononic incorporated in the Software.
*/

/************************************************************************/
/* FW VERSION                                                           */
/************************************************************************/
#define FIRMWARE_VERSION	0xff	// customer-facing fw version #
#define FIRMWARE_REVISION	0x03	// internal fw version, rev, or build #

/************************************************************************/
/* Feature Control                                                      */
/************************************************************************/
//#define ENABLE_SLEEP_WAKE				// enable sleep/wake for ui

#define ENABLE_DEBUG_CMD_INTERFACE		// allow dbg cmds

//--- PHPs --------------------------------------------------------------
//--- define only one ---------------------------------------------------
//#define PHP_DEFAULT_POWER_SPLIT		// default php drive
//#define PHP_ENERGY_OPTIMIZATION		// reduced php2 duty in highcop
//#define PHP_EQUAL_POWER_SPLIT			// equal duty/power to each php
//#define PHP_ENERGY_OPTIMIZATION_2		// new table to limit condensation
#define PHP_STEEP_TUNING_TABLE
//#define PHP_GENTLE_TUNING_TABLE		
//#define PHP_SMOOTH_TUNING_TABLE

//--- Performance -------------------------------------------------------
//--- define only one ---------------------------------------------------
#define HIGH_PERFORMANCE

/************************************************************************/
/* Change History                                                       */
/************************************************************************/

// TODO:	+ add eeprom storage for defrostCancelTemp, defrost freezeup variables
//          + re-enable sensor EDFI

// vff 03	2018-01-11	> all P gains from 9 to 5

// vff 02	2018-01-09	> updated fan limits table
//                      > modified psHwControl to shut off below mininum Vset

// vff 01   2017-10-29	> ENABLE_SLEEP_WAKE disabled
//						> added support for fans 5, 6, 7, 8
//						> set PIN_IPHP2 = PIN_IPHP1
//						> update chamber and reject snsor EDFI functions
//						> add fan_id and fan limit table

// started from 156L_HP_REF_TEC v01a9 TMO GENERAL_PURPOSE

/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include <asf.h>
#include <stdio_serial.h>
#include <string.h>
#include "conf_test.h"
#include <math.h>
#include <nvm.h>
#include "i2c_slave.h"

/************************************************************************/
/* Pin Mapping	                                                        */
/************************************************************************/
// XIN32												// 1   
// XOUT32												// 2   
#define PIN_VPHP2			ADC_POSITIVE_INPUT_PIN0		// 3   
// AVREF												// 4   
#define PIN_TGLYC1			ADC_POSITIVE_INPUT_PIN12	// 5   
#define PIN_VCCMON			ADC_POSITIVE_INPUT_PIN13	// 6   
// GNDANA												// 7   
// VDDANA												// 8   
#define PIN_VBOOST_SNS      ADC_POSITIVE_INPUT_PIN14	// 9   
#define PIN_COLD1_SNS       ADC_POSITIVE_INPUT_PIN15    // 10   
#define PIN_COLD2_SNS		ADC_POSITIVE_INPUT_PIN2	    // 11      
#define PIN_VBATMON			ADC_POSITIVE_INPUT_PIN3		// 12    
#define PIN_ENBL_OUTB_HOT 	PIN_PA04					// 17	//PIN_3V3MON			ADC_POSITIVE_INPUT_PIN4		// 13   
#define PIN_VBULK1			ADC_POSITIVE_INPUT_PIN5		// 14   
#define PIN_ENBL_FAN8		PIN_PA06				    // 15   									
#define PIN_ENBL_FAN7		PIN_PA07		            // 16
#define PIN_ENBL_OUTB_COLD	PIN_PA08					// 17	//PIN_HS_TEMP			ADC_POSITIVE_INPUT_PIN16    // 17   
#define PIN_HS_HUM			ADC_POSITIVE_INPUT_PIN17    // 18   
#define PIN_TCHA2			ADC_POSITIVE_INPUT_PIN18	// 19  
#define PIN_TREJ2			ADC_POSITIVE_INPUT_PIN19	// 20   
// VDDIO												// 21   
// GND													// 22   
#define PIN_ENBL_LAMP       PIN_PB10					// 23   
#define PIN_RST_TO_SLV_N	PIN_PB11					// 24   
#define PIN_ENBL_FAN4		PIN_PB12					// 25   
#define PIN_ENBL_BUCK1		PIN_PB13					// 26   
#define PIN_ENBL_OUTPUT1	PIN_PB14					// 27  
#define PIN_CON_SNS         PIN_PB15					// 28  
// PWM						PIN_PA12					// 29 
// PWM						PIN_PA13					// 30 
#define PIN_LED_0_PIN		PIN_PA14					// 31 
#define PIN_ENBL_OUTPUT2	PIN_PA15					// 32 
// GND													// 33 
// VCC													// 34 
#define PIN_F_SDA			PIN_PA16					// 35  
#define PIN_F_SCL			PIN_PA17					// 36  
#define PIN_ENBL_LOAD2		PIN_PA18					// 37  
#define PIN_ENBL_FAN3		PIN_PA19					// 38  
// I2C SDA PB16											// 39  
// I2C SCL PB17											// 40  
#define PIN_ENBL_FAN2		PIN_PA20					// 41  
#define PIN_ENBL_FAN1		PIN_PA21					// 42  
#define PIN_ENBL_FAN6		PIN_PA22					// 43   
#define PIN_ENBL_FAN5		PIN_PA23					// 44    
// UART MCU TXD PA24									// 45  
// UART MCU RXD PA24									// 46  
// GND													// 47  
// VCC													// 48  
#define  PIN_ENBL_VHTR1     PIN_PB22					// 49  
#define  PIN_ENBL_VHTR2     PIN_PB23                    // 50  
#define PIN_SLP			    PIN_PA27					// 51  
// RESET_N												// 52  
#define PIN_RDY_N			PIN_PA28					// 53  
// GND													// 54  
// VDDCORE												// 55  
// VDDIO												// 56  
// SWDCLK												// 57  
// SWDIO												// 58  
#define PIN_PB30_UNUSED		PIN_PB30					// 59  
#define PIN_ENBL_OUTA_COLD	PIN_PB31					// 60  
#define PIN_TCHA1			ADC_POSITIVE_INPUT_PIN8		// 61  
#define PIN_TREJ1			ADC_POSITIVE_INPUT_PIN9		// 62   
#define PIN_VPHP1			ADC_POSITIVE_INPUT_PIN10	// 63  
#define PIN_ENBL_OUTA_HOT 	PIN_PB03					// 64  

/************************************************************************/
/* Aliases                                                              */
/************************************************************************/
//#define PIN_IPHP2		PIN_IPHP1
#define PIN_ENBL_PSFAN  PIN_ENBL_LAMP	

/************************************************************************/
/* Defines                                                              */
/************************************************************************/
#define OFF			false
#define ON			true
#define CLOSED		false
#define OPEN		true
#define GPIO_MODE	ON
#define PWM_MODE	2

/* ADC Reference Voltage */
#define ADC_REF_VOLTAGE ADC_REFERENCE_AREFA
/* Number of ADC samples used in the test */
#define ADC_SAMPLES 128
/* Theoretical ADC result for DAC half-swing output */
#define ADC_VAL_DAC_HALF_OUTPUT 620//2047
/* Theoretical ADC result for DAC full-swing output */
#define ADC_VAL_DAC_FULL_OUTPUT 1240//4095
/* Offset due to ADC & DAC errors */
#define ADC_OFFSET              50

// eeprom emulator
#define EEPROM_PAGE0		0
#define EEPROM_PAGE1		1
#define EEPROM_PAGE2		2
#define EEPROM_PAGE3		3
#define EEPROM_NOT_WRITTEN						0x000000
#define EEPROM_CHAMBEROFFSET_WRITTEN			0x000001
#define EEPROM_GLYCOLOFFSET_WRITTEN				0x000002
#define EEPROM_DEFROSTFREQMAXHRS_WRITTEN		0x000004
#define EEPROM_DEFROSTFREQHRS_WRITTEN			0x000008
#define EEPROM_DEFROSTDURATIONMINS_WRITTEN		0x000010
#define EEPROM_MACADDR_WRITTEN					0x000020
#define EEPROM_SERIAL_WRITTEN					0x000040
#define EEPROM_MODEL_WRITTEN					0x000080
#define EEPROM_MFGTESTFLAG1_WRITTEN				0x000100
#define EEPROM_MFGTESTFLAG2_WRITTEN				0x000200
#define EEPROM_CHAMBER2OFFSET_WRITTEN			0x000400
#define EEPROM_CHAMBERBETA_WRITTEN				0x000800
#define EEPROM_REJECTBETA_WRITTEN				0x001000
#define EEPROM_UIFWVER_WRITTEN					0x002000
#define EEPROM_RUNSELFTESTFLAG_WRITTEN			0x004000
#define EEPROM_PS1OUTPUTDUTYOFFSET_WRITTEN		0x008000
#define EEPROM_PS2OUTPUTDUTYOFFSET_WRITTEN		0x010000
#define EEPROM_MAXALLOWEDTEMP_WRITTEN			0x020000
#define EEPROM_MINALLOWEDTEMP_WRITTEN			0x040000
#define EEPROM_DOOROPENLIFETIMECNT_WRITTEN		0x080000
#define EEPROM_DOOROPENCOUNT1HRLIMIT_WRITTEN	0x100000

#define DEFAULT_SERIAL							"XXXXXXXXXX1234"
#define DEFAULT_MODEL							"ELR055SSA002"
#define DEFAULT_UIFWVER							"XXXXXX"

// usart comm
#define MAX_RX_BUFFER_LENGTH	20
#define MAX_TX_BUFFER_LENGTH	16
#define RX_BUFR_SIZE			5

// i2c comm
#define SLAVE_ADDRESS			0x32; //0x90>>1
#define F_DATA_LENGTH			10
#define F_SLAVE_ADDRESS			(0x58>>1)	// EMC2305 w/ ADDR_SEL=10Kpu -> dev addr = 0101100x or 0x58 or 0x59
#define F_SLAVE_ADDRESS_2		(0x5E>>1)	// EMC2305 w/ ADDR_SEL=6.8Kpu -> dev addr = 0101111x or 0x5E or 0x5F
#define F_I2C_COMM_TIMEOUT		10
#define FAN_CHIP1				1
#define FAN_CHIP2				2

// thermistor constants
//#define BETA_3892_USP10976		3892	   // Beta for US Sensor Corp USP10976 lug thermistor (reject)
//#define BETA_3970_APRON			3970	   // 3970 Beta for AMPRON APR-CWF103F3970FA378.5A (accept)
//#define BETA_3315_SN410P01		3315	   // 3315 Beta for LAE SN410P1 sealed cylinder thermistor (accept)
//#define BETA_3984_NTCALUG02A		3984	   // 3984 Beta for Vishay NTCALUG02A103F161 (reject)
//#define BETA_3977_NTCLP100		3977	   // 3977 Beta for Vishay NTCLP100E3103H (accept)
#define BETA_25_85_APRCWF103F3892GG146A		3974	// Beta 25/85 for AMPRON APR-CWF103F3892GG146A (reject)
#define BETA_0_50_APRCWF103F3970FA3785A		3879	// Beta 0/50 for AMPRON APR-CWF103F3970FA378.5A (accept)
#define BETA_NEG20_30_APRCWF103F3970FA3785A	3720	// Beta -20/30 for AMPRON APR-CWF103F3970FA378.5A (accept)
#define BETA_0_50_NTCLP100E3103H			3885	// Beta 0/50 for Vishay NTCLP100E3103H (accept)
#define BETA_0_50_WASON_GLYCOL				3904	// Beta 0/50 for WASON (glycol)

#define THERMISTORNOMINAL		10000      // resistance at 25 degrees C
#define SERIESRESISTOR			10000      // the value of the 'other' resistor
#define TEMPERATURENOMINAL		25         // temp. for nominal resistance (almost always 25 C)

#define BCOEFFICIENT_TREJ		BETA_25_85_APRCWF103F3892GG146A
#define BCOEFFICIENT_TCHAMBER   BETA_NEG20_30_APRCWF103F3970FA3785A
#define BCOEFFICIENT_TGLYCOL	BETA_0_50_WASON_GLYCOL

// php
#define PHP125_NOMINAL_RESISTANCE	24
#define PHP125_RES_TOL_PERCENTAGE	33
#define MAX_PHP_ERROR_COUNT			3		// must adjust timer scheduler slots to match this count
//#define NO_OF_TEMS_IN_CARTRIDGE 4
//#define R_MODULE                NO_OF_TEMS_IN_CARTRIDGE * 1.78           // device resistance
//#define S_MODULE                NO_OF_TEMS_IN_CARTRIDGE * 0.02592        // device seebeck voltage
//#define Z_MODULE                0.00236                                  // device Z (figure of merit)
//#define K_LEGS                  NO_OF_TEMS_IN_CARTRIDGE * 0.16           // leg thermal conductance

// timing
#define RTC_PERIOD_IN_MSEC				1000
#define TASK1_SEC						1
#define TASK2_SEC						5
#define TASK3_SEC						5
// task4 used for defrost
// task5 used for defrost
#define TASK6_SEC						10
#define TASK7_SEC						40
#define TASK8_SEC						3600			// in seconds
#define TASK9_SEC						60
#define TASK10_SEC						120
#define TASK11_SEC						3600
#define TASK12_SEC						2
//#define DEFAULT_DATALOG_INTERVAL_MINS	15
#define DEFAULT_PWR_CAP_TIMEOUT_MINS	45

// defrost
#define DEFAULT_DEFROST_DURATION_MINS	12
#define DEFROST_START					0
#define DEFROST_STOP					1
#define DEFROST_CANCEL					2
#define FRZ_UP_ACTIVATE_CNT_LIMIT		5
#define FRZ_UP_DEACTIVATE_CNT_LIMIT		5
#define DEFROST_MAX_INTERVAL_TIME_IN_HRS	168

// state machine
#define STATE_RESET						0
#define STATE_IDLE						1
#define STATE_HIGH_Q					2
#define STATE_VAR_Q						3
#define STATE_HIGH_COP					4
#define STATE_DEFROST					5
#define STATE_MANUAL					6
#define PID_MODE						0
#define BANG_BANG_MODE					1
#define MANUAL_MODE						2

// chamber 
#define REJECT_LIMIT_DEFAULT            57
#define REJECT_LIMIT_CRITICAL_DEFAULT   60
#define REJECT_DEADBAND_DEFAULT         4
#define DEADBAND_DEFAULT                0.45
#define DEADBANDLOW_DEFAULT				0.45
#define HEATLEAK_DEFAULT				1.05
#define TSNS_INSANE_LOW_LIMIT			-40
#define DEFAULT_AMBIENT_TEMP			24.0
#define UNIFORMITY_TARGET				0.5
//#define STEADY_STATE_REJECT_TEMP		32

// sensors
#define VCCMON_SCALING_FACTOR			0.000977  // 0.001050
#define LOGIC3V3_SCALING_FACTOR			0.000488
#define VBAT_SCALING_FACTOR				0.000977
#define HUMIDITY_SCALING_FACTOR			0.000244
#define MAX_SENSOR_ERROR_COUNT			3

// power supply
#define PS_BULK_PWM_LOOKUP_TABLE_SIZE	12
#define OUTPUT_DUTY_LOOKUP_TABLE_SIZE	93 		
//#define	MAX_OUTPUT_CURRENT_DEFAULT	3.3		
#define BULK_OFF_DUTY					10
#define BOOST_OFF_DUTY					65535 //25816
#define PWM_MAX_DUTY_VAL				65535
#define MAX_VCC_VOLTAGE					3.2		// set below 3.3V
//#define MIN_CURRENT_AT_VPHP_NOM		0.7
#define VTEC_VOLTAGE_TOLERANCE			2
#define MAX_OV_ERROR_COUNT				6
#define MAX_UV_ERROR_COUNT				6
#define MAX_VCC_ERROR_COUNT				6
#define DEFAULT_PS1OUTPUTDUTYOFFSET_SETTING	0
#define DEFAULT_PS2OUTPUTDUTYOFFSET_SETTING 0
#define SP_PWR_ADJ_OFFSET_LOOKUP_TABLE_SIZE 13
#define BULK_VOLTAGE_DETECT_48V_THRESHOLD   44
#define BULK_VOLTAGE_DETECT_41V_THRESHOLD	38
//#define MINIMUM_ALLOWED_VOLTAGE         0
#define OUTPUT_PWM_VOLTAGE_THRESHOLD	30
#define VARQ_VOLTAGE_DEFAULT            62
#define BULK_VOLTAGE_48V_MAX            49
#define BULK_VOLTAGE_48V_NOM            48    // must be a whole number
#define BULK_VOLTAGE_48V_MIN            47
#define BULK_VOLTAGE_41V_MAX            42
#define BULK_VOLTAGE_41V_NOM            41    // must be a whole number
#define BULK_VOLTAGE_41V_MIN            40
#define BULK_VOLTAGE_36V_MAX            36
#define BULK_VOLTAGE_36V_NOM            35    // must be a whole number
#define BULK_VOLTAGE_36V_MIN            34
#define DEFAULT_PHP1_DETECT_DIFF_CURRENT_LIMIT	0.30
#define DEFAULT_PHP2_DETECT_DIFF_CURRENT_LIMIT	0.30

// pid controller
#define AUTOMATIC						1
#define MANUAL							0
#define DIRECT							0
#define REVERSE							1
#define KP_HIGHQ						5
#define KI_HIGHQ						0.000010
#define KD_HIGHQ						0
#define KP_VARQ							5
#define KI_VARQ							0.000010
#define KD_VARQ							0
#define KP_COPQ							5
#define KI_COPQ							0.000010
#define KD_COPQ							0

// fans
#define MIN_ALLOWABLE_FAN_RPM			400
#define DEFAULT_FAN_RPM_ERROR_PERCENTAGE	25
#define DEFAULT_FAN_SPD_TOLERANCE		500
#define FAN_STALLED						true
#define FAN_SPD_PERCENT_LOOKUP_TABLE_SIZE	93
#define FAN_RPM_LOOKUP_TABLE_SIZE		101
#define FAN_STATE_DEFAULT               OFF
#define FAN_STATE_S1					1
#define FAN_STATE_S2					2
#define FAN_STATE_S3					3
#define FAN_STATE_S4					4
#define FAN_STATE_S5					5
#define FAN_STATE_S6					6
#define FAN_STATE_REJECT_OT				7
#define FAN_STATE_DEFROST				8
#define MAX_FAN_PWM_DUTY_REG_VAL		65535
#define DEFAULT_FAN_VTOP				75 // 25
#define DEFAULT_FAN_VBOTTOM				48
#define DEFAULT_FAN_MAX_DUTY			100
#define DEFAULT_FAN_MIN_DUTY			0		
#define MAX_FAN_ERROR_COUNT				10
#define FAN_SAMPLE_FREQ					32768
#define MIN_CHAMBER_FAN_OFFTIME			10		// in seconds
#define CHAMBER_FAN_OFF_TIMEOUT			120		// in seconds
#define FAN_MAX_SPD_REJECT_TEMP_LIMIT	50
#define KP_FAN1							8
#define KI_FAN1							0
#define KD_FAN1							0
#define KP_FAN2							8
#define KI_FAN2							0
#define KD_FAN2							0
#define KP_FAN4							40
#define KI_FAN4							0.0025
#define KD_FAN4							0	
#define FAN_REJ_K_CONSTANT				4
#define FAN_PS_K_CONSTANT				8
#define FAN_CHA_K_CONSTANT				2 //4
#define FAN_RESPONSE_TIMECONSTANT		5
		
#define FAN_SM_PS_spd1					100
#define FAN_SM_PS_lim1p					50
#define FAN_SM_PS_lim1					45
#define FAN_SM_PS_spd2					0
#define FAN_SM_PS_lim2p					0
#define FAN_SM_PS_lim2					0
#define FAN_SM_PS_spd3					100
#define FAN_SM_PS_lim3p					0
#define FAN_SM_PS_lim3					0
#define FAN_SM_PS_spd4					100
#define FAN_SM_PS_lim4p					0
#define FAN_SM_PS_lim4					0
#define FAN_SM_PS_spd5					100
#define FAN_SM_PS_lim5p					0
#define FAN_SM_PS_lim5					0	
#define FAN_SM_PS_spd6					100

// heaters
#define HTR_UPPER_TCHA_LIMIT			8
#define HTR_LOWER_TCHA_LIMIT			2
#define DOOR_HEATER_DUTY_LOOKUP_TABLE_SIZE	13	
#define STATE_CONDHTR_OFF				0
#define STATE_CONDHTR_S1				1
#define STATE_CONDHTR_S2				2
#define STATE_CONDHTR_S3				3
#define STATE_CONDHTR_S4				4
#define COND_HTR_OFF_TEMP_LIMIT			45
#define COND_HTR_SAFETY_LIMIT			REJECT_LIMIT_DEFAULT
#define COND_HTR_OFFTIME_IN_SEC			3*60*60		// 3HRS	
#define COND_HTR_OFF_DUTY				0
#define COND_HTR_S1_DUTY				58
#define COND_HTR_S2_DUTY				67
#define COND_HTR_HTR_S4_DUTY			83

// errorRegister Error Codes
#define ERROR_NONE				0x0000
#define ERROR_PHP1				0x0001
#define ERROR_PHP2				0x0002
#define ERROR_CHA_FAN_SPEED		0x0004
#define ERROR_INSANE_TCHA1		0x0008
#define ERROR_INSANE_TCHA2		0x0010
#define ERROR_FAN_CONTROLLER	0x0020
#define ERROR_INSANE_TREJ		0x0040
#define ERROR_REJECT_CRITICAL	0x0080
#define ERROR_REJ_FAN_SPEED		0x0100
//#define ERROR_OUTPUT_UV		0x0200

// failure detection
#define DEFAULT_SELFTESTONPOR_SETTING	false
#define SELF_TEST_LIMITS_TABLE_SIZE 18
#define SELF_TEST_FAN_SPD			50
#define SELF_TEST_RESULTS_ARRAY_SIZE (sizeof(uint32_t)*8)
#define STATE_SNS_NORM_TCHA		1
#define STATE_SNS_FAIL1			2
#define STATE_SNS_FAIL2			3
#define STATE_SNS_FAIL3			4
#define STATE_SNS_FAIL4			5
#define STATE_SNS_NORM_TREJ		6
#define SNS_FAIL_VSET			23

// protocol
#define bool	_Bool
#define true	1
#define false	0

#define PROTOCOL_BUFR_SIZE	5	// protocol buffer is fixed size
#define PARAM_SIZE_DEF		4	// size of all of the entries in the global parameters table
#define PARAM_SIZE_UINT8	0
#define PARAM_SIZE_UINT16	1
#define PARAM_SIZE_FLOAT	3

#define ZONE1_FLAG			0x01
#define ZONE2_FLAG			0x02
//#define READ_FLAG			0x04
#define WRITE_ENABLE_FLAG	0x08
#define SIGN_NEGATIVE_FLAG	0x10

#define LOW_BYTE_INDEX		0
#define HIGH_BYTE_INDEX		1	// same position used for either High Byte or Decimal Hundredths depending on Uint16 or Float type
#define DECIMAL_BYTE_INDEX	1	// same position used for either High Byte or Decimal Hundredths depending on Uint16 or Float type
#define PARAMETER_ID_INDEX	2
#define FLAGS_INDEX			3	// byte at which the above defined flags are to be located

#define CRC_INDEX (PROTOCOL_BUFR_SIZE - 1)	// because of the checkCRC function, this index must be last

#define IDX_SETPOINT		1
#define IDX_MAC_ADDR		50
#define IDX_SERIAL			56
#define IDX_MODEL			71
#define SERIAL_SIZE			IDX_MODEL-IDX_SERIAL //16
#define MODEL_SIZE			13
#define MAC_ADDR_SIZE		IDX_SERIAL-IDX_MAC_ADDR //7
#define REGARRAY_SIZE		12
#define IDX_UIFWVER			122
#define	UIFWVER_SIZE		7

#define DEFAULT_DOOR_OPEN_CNT_1HR_LIMIT		10






#ifdef HIGH_PERFORMANCE

#define SETPOINT_DEFAULT				-18
#define DEFAULT_MAX_ALLOWED_TCHA		-15
#define DEFAULT_MIN_ALLOWED_TCHA		-25

#define IPHP1_SCALING_FACTOR			0.00163 //0.0006965745 //0.001349//0.000483
#define IPHP2_SCALING_FACTOR			0.00127
#define VBULK1_SCALING_FACTOR			0.012661 //0.004199  // (R1+R2)/(2^N * R2), R2 bot resist, N bits adc res
#define VTEC_SCALING_FACTOR				0.022401 //0.021701//0.023509//0.0169362100//.015500314//0.003834  // (R1+R2)/(2^N * R2), R2 bot resist, N bits adc res

#define DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT 1.30
#define DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT 1.30

#define PS_BOOST_PWM_LOOKUP_TABLE_SIZE     12
#define PCP_HOT_STAGE_MIN_OUTPUT_VOLTAGE   15.75
#define PCP_HOT_STAGE_MAX_OUTPUT_VOLTAGE   25.5
#define PCP_COLD_STAGE_MIN_OUTPUT_VOLTAGE  11.5
#define PCP_COLD_STAGE_MAX_OUTPUT_VOLTAGE  17

#define MAX_OUTPUT_VOLTAGE_DEFAULT		PCP_COLD_STAGE_MAX_OUTPUT_VOLTAGE
#define MINIMUM_ALLOWED_VOLTAGE			0 //PCP_COLD_STAGE_MIN_OUTPUT_VOLTAGE

#define VMAXCOP_DEFAULT					PCP_COLD_STAGE_MIN_OUTPUT_VOLTAGE

#define DEFAULT_FAN_MAX_RPM				4400

#define DEFAULT_DEFROST_FREQ_HRS		24
#define DEFROST_ACTIVATE_TEMPERATURE		-5.0
#define DEFROST_DEACTIVATE_TEMPERATURE		+5.0
#define DEFROST_FREEZEUP_TIMEOUT_IN_MINS	3

#endif