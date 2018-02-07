/*
 * dbgCmds.h
 *
 * Created: 4/10/2014 8:48:34 AM
 *  Author: Daniel.Barus
 */ 

#ifndef PROTO_H_INCLUDED
#define PROTO_H_INCLUDED

#include "app.h"

//#define SINGLE_ZONE 1

/************************************************************************/
/* Protocol Constants                                                   */
/************************************************************************/
#define SETPOINT_DEFAULT_P				0
#define REJECT_LIMIT_DEFAULT_P          1
#define REJECT_LIMIT_CRITICAL_DEFAULT_P 2
#define REJECT_DEADBAND_DEFAULT_P       3
#define DEADBAND_DEFAULT_P		        4
#define DEADBANDLOW_DEFAULT_P			5
#define GLYCOL_TEMP_DEFAULT_P			6
#define CHAMBER_TEMP_DEFAULT_P			7
#define REJECT_TEMP_DEFAULT_P			8
#define TGLY_DEFAULT_P					9
#define TCHA_OFFSET_P					10
#define MODE_P							11
#define FAN_SPEED_P						0
#define FAN_VTOP_P						1
#define FAN_VBOT_P						2
#define FAN_MIN_DUTY_P					3
#define FAN_MAX_DUTY_P					4
#define FAN_TACH_P						5
#define FAN_STATE_P						6


    static const int D20Pins[] = {
		PIN_ENBL_OUTPUT1,
		//PIN_ENBL_OUTPUT2,
		PIN_ENBL_OUTA_COLD,
		//PIN_ENBL_OUTPUT4,
		//PIN_ENBL_BUCK1,
		//PIN_ENBL_AC1,
		PIN_ENBL_LOAD2,
		PIN_RST_TO_SLV_N,
		//PIN_ENBL_PS1_ORING,
		//PIN_PB12,		// unused on 40B P1 hw
		//PIN_ENBL_AC1, //ps1 power supply Enable
	};
	
    static const int D20Chamber[] = {
		SETPOINT_DEFAULT_P,
		REJECT_LIMIT_DEFAULT_P,
		REJECT_LIMIT_CRITICAL_DEFAULT_P,
		REJECT_DEADBAND_DEFAULT_P,
		DEADBAND_DEFAULT_P,
		DEADBANDLOW_DEFAULT_P,
		GLYCOL_TEMP_DEFAULT_P,
        CHAMBER_TEMP_DEFAULT_P,
        REJECT_TEMP_DEFAULT_P,
        TGLY_DEFAULT_P,
		TCHA_OFFSET_P,
		MODE_P,
	};
	
    static const int D20POWERSUPPLY[] = {
		VARQ_VOLTAGE_DEFAULT,
		VMAXCOP_DEFAULT,
		MAX_OUTPUT_VOLTAGE_DEFAULT,
		3, // phpVoltage1
		4, // phpCurrent1
		5, // bulkVoltage
		6, // desiredVoltage
		7, // logicVoltage
		8, // standbyVoltage
		9, // batteryVoltage
		10, // phpVoltage2
		11, // phpCurrent2
		12,	// enableSideXPcp
    };

	static const int D20PID[] = {
		KP_HIGHQ,
		KI_HIGHQ,
		KD_HIGHQ,
		KP_VARQ,
		KI_VARQ,
		KD_VARQ,
		KP_COPQ,
		KI_COPQ,
		KD_COPQ,
	};
	
	static const int D20FAN[] = {
		0,	//FAN_SPEED_P,
		1,	//FAN_VTOP_P,
		2,	//FAN_VBOT_P,
		3,	//FAN_MIN_DUTY_P,
		4,	//FAN_MAX_DUTY_P,
		5,	//FAN_TACH_P,
		6,	//FAN_STATE_P,
		7,	//FAN_SPEED_P,
		8,	//FAN_VTOP_P,
		9,	//FAN_VBOT_P,
		10,	//FAN_MIN_DUTY_P,
		11,	//FAN_MAX_DUTY_P,
		12,	//FAN_TACH_P,
		13,	//FAN_STATE_P,
		14, // fan3 speed
		15, // fan4 speed
	};
	
	static const int D20CONTROL[] = {
		0,
		1,
		2,	// fNoAction
		3,	// chamberThermistorBeta
		4,	// rejectThermistorBeta
		5,	// ps1.phpOutputDutyOffset
		6,	// ps2.phpOutputDutyOffset
		7,  // maxSnsErrorCount
		8,  // maxFanErrorCount
		9,  // maxPhpErrorCount
		10,	// maxAllowedChamberTemp
		11,	// minAllowedChamberTemp
		12, // doorOpenCount1HrLimit
	};
	
	
	//typedef void (*set_setPoint)(float fNewValue);
	//typedef struct D20Points
	//{
	//  	set_setPoint setPoint;
	//}setChamber;
	float (*fChamber[32])(float, bool, int);
	float (*fPower[32])(float, bool, int);
	float (*fPid[32])(float, bool, int);
	float (*fFan[32])(float, bool, int);
	float (*fControl[32])(float, bool, int);
	
	static inline bool port_pin_set_output_level_EX(
	const uint8_t gpio_pin,
	const int nLevel, const bool ManualOverride)
	{
		//check mux settings
		if (nLevel)
		{  
/*		  switch (gpio_pin)
		  {
			  case PIN_ENBL_OUTPUT1:
			  if (port_pin_get_output_level(PIN_ENBL_OUTPUT4))
			  {
				  return false;
			  }
			  break;
			  case PIN_ENBL_OUTPUT2:
#ifndef SINGLE_ZONE
			  if (port_pin_get_output_level(PIN_ENBL_OUTPUT3))
			  {
				  return false;
			  }
#else
				  return false;
#endif
			  break;
			  case PIN_ENBL_OUTPUT3:
			  if (port_pin_get_output_level(PIN_ENBL_OUTPUT2))
			  {
				  return false;
			  }
			  break;
			  case PIN_ENBL_OUTPUT4:
#ifndef SINGLE_ZONE			  
			  if (port_pin_get_output_level(PIN_ENBL_OUTPUT1))
			  {
				  return false;
			  }
#else
			  return false;
#endif
			  break;
			  default:
			  break;
		  }
*/			
		}
	
		if (!ManualOverride)
		{
		  bool bLevel = (nLevel != 0);
		  port_pin_set_output_level(gpio_pin, bLevel);
		}
		
		return true;
	}
	
	#endif
