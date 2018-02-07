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
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "dbgCmds.h"
#include "i2cProtocol.h"
#include "extern.h"
#include "string.h"

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
uint32_t self_test_status = 0x0000;
uint16_t self_test_status_hi = 0x00;
uint16_t self_test_status_lo = 0x00;
uint8_t selfTestIsRunning = 0;

bool test_results_array[SELF_TEST_RESULTS_ARRAY_SIZE] = {1};
 
 float testUprLimit[SELF_TEST_LIMITS_TABLE_SIZE] = {
	 3.7,	// vstby
	 3.7,	// vlogic
	 50.0,	// vbulk
	 3.7,	// vbat
	 100.0,	// trej1
	 100.0,	// trej2
	 100.0,	// tcond
	 100.0,	// tcha1
	 100.0,	// tcha2
	 100.0,	// tgly
	 #ifdef HIGH_PERFORMANCE
	 5000.0,// fan1
	 5000.0,// fan2
	 #endif
	 5000.0,// fan3
	 5000.0,// fan4
	 50.0,	// vphp1
	 50.0,	// vphp2
	 3.0,	// iphp1
	 3.0	// iphp2
 };

 float testLwrLimit[SELF_TEST_LIMITS_TABLE_SIZE] = {
	 2.8,	// vstby
	 3.0,	// vlogic
	 34.0,	// vbulk
	 2.0,	// vbat
	 TSNS_INSANE_LOW_LIMIT,	// trej1
	 TSNS_INSANE_LOW_LIMIT,	// trej2
	 TSNS_INSANE_LOW_LIMIT,	// tcond
	 TSNS_INSANE_LOW_LIMIT,	// tcha1
	 TSNS_INSANE_LOW_LIMIT,	// tcha2
	 TSNS_INSANE_LOW_LIMIT,	// tgly
	 1000.0,// fan1
	 1000.0,// fan2
	 1000.0,// fan3
	 1000.0,// fan4
	 34.0,	// vphp1
	 34.0,	// vphp2
	 DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT,	// iphp1
	 DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT	// iphp2
 };

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
bool selfTest()
{
    float tempVal = 0;
	uint8_t test_num = 0;
	
	dbg("\n[running self test]");
	
	selfTestIsRunning = 1;
		
	// reset status;	
	for (uint8_t n=0; n<SELF_TEST_RESULTS_ARRAY_SIZE; n++)
	{
		test_results_array[n] = 1;
	}
	self_test_status = 0x0000;	

	setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, OFF);	  // disable output to php1
	setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, OFF);	  // disable output to php2
				
	fanStop_emc2305(&fan1);
	fanStop_emc2305(&fan2);
	fanStop_emc2305(&fan3);
	fanStop_emc2305(&fan4);
	delay_s(5);		
	readSensors();

	/////////////////////////////////////
	// onboard sensors
	/////////////////////////////////////
	testParamLimits(1, ps1.standbyVoltage, test_num, "Vstby");
	testParamLimits(0, 0, ++test_num, "Vlogic"); //testParamLimits(1, ps1.logicVoltage, ++test_num, "Vlogic");		
	testParamLimits(1, ps1.bulkVoltage, ++test_num, "Vbulk");
	testParamLimits(1, ps1.batteryVoltage, ++test_num, "Vbatt");				
		// vboost
		// rst in
		// sleep
		// rdy_n	

	/////////////////////////////////////
	// reject harness
	/////////////////////////////////////	
	testParamLimits(1, chamber1.rejectTemp, ++test_num, "Trej1");
#ifdef HIGH_PERFORMANCE	
	testParamLimits(1, chamber2.rejectTemp, ++test_num, "Trej2");
#else
	testParamLimits(0, 0, ++test_num, "Trej2");
#endif

	/////////////////////////////////////
	// cond sensor
	/////////////////////////////////////
	testParamLimits(1, condensateTemp, ++test_num, "Tcond");

	/////////////////////////////////////
	// foamed-in harness
	/////////////////////////////////////		
	tempVal = read_adc_pin(PIN_TCHA1);
	tempVal = ADCtoTemperature(tempVal, chamberThermistorBeta);
	chamber1.chamberTempRaw = tempVal;
#ifdef HIGH_PERFORMANCE	
	testParamLimits(1, chamber1.chamberTempRaw, ++test_num, "Tcha1");
#else
	testParamLimits(1, chamber1.chamberTempRaw, ++test_num, "Tacc");
#endif
	
	tempVal = read_adc_pin(PIN_TCHA2);
	tempVal = ADCtoTemperature(tempVal, chamberThermistorBeta);
	chamber2.chamberTempRaw = tempVal;
	testParamLimits(1, chamber2.chamberTempRaw, ++test_num, "Tcha2");
	
	tempVal = read_adc_pin(PIN_TGLYC1);
	tempVal = ADCtoTemperature(tempVal, BCOEFFICIENT_TGLYCOL);
	chamber1.glycolTemp = tempVal;
	testParamLimits(1, chamber1.glycolTemp, ++test_num, "Tglyc");
		
	// turn on fans
	fanStart_emc2305(&fan4); 
	fanSetSpeed_emc2305(&fan4, SELF_TEST_FAN_SPD);
	fanReadRpm_emc2305(&fan4); 
	fanStart_emc2305(&fan3); 
	fanSetSpeed_emc2305(&fan3, SELF_TEST_FAN_SPD);			
	fanStart_emc2305(&fan2); 
	fanSetSpeed_emc2305(&fan2, SELF_TEST_FAN_SPD);
	fanReadRpm_emc2305(&fan2); 
	fanStart_emc2305(&fan1); 
	fanSetSpeed_emc2305(&fan1, SELF_TEST_FAN_SPD);
	delay_s(5); // wait
						
	/////////////////////////////////////
	// rej fans (fan1, fan2)
	/////////////////////////////////////		
	fanReadRpm_emc2305(&fan1); 
	testParamLimits(1, fan1.rpm, ++test_num, "Fan1");		
	fanReadRpm_emc2305(&fan2); 
	testParamLimits(1, fan2.rpm, ++test_num, "Fan2");
				
	/////////////////////////////////////
	// ps fan
	/////////////////////////////////////
	/*
	fanReadRpm_emc2305(&fan3); 
	testParamLimits(fan3.rpm, ++test_num, "Fan3");
	*/
	testParamLimits(0, 0, ++test_num, "Fan3");
		
	/////////////////////////////////////
	// chamber fan
	/////////////////////////////////////
	fanReadRpm_emc2305(&fan4); 
	testParamLimits(1, fan4.rpm, ++test_num, "Fan4");

	/////////////////////////////////////
	// php/output voltage
	/////////////////////////////////////
	ps1.desiredVoltage = 0;
	
	psHwController();
		
	setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, ON);	  // enable output to php1
	delay_s(5);
	tempVal = read_adc_pin(PIN_VPHP1);
	tempVal += 30;
	tempVal = tempVal * VTEC_SCALING_FACTOR * ps1.standbyVoltage;
	if (tempVal > 0.0)
	{
		tempVal = ( round(tempVal * 10.0) / 10.0 ); // reduce granularity of measured voltage
		ps1.phpVoltage = tempVal;
	}
	else
	{
		ps1.phpVoltage = 0;
	}
	testParamLimits(1, ps1.phpVoltage, ++test_num, "Vphp1");

#ifdef HIGH_PERFORMANCE		
	setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, ON);	  // enable output to php2
	delay_s(5);
	tempVal = read_adc_pin(PIN_VPHP2);
	tempVal += 30;
	tempVal = tempVal * VTEC_SCALING_FACTOR * ps1.standbyVoltage;
	if (tempVal > 0.0)
	{
		tempVal = ( round(tempVal * 10.0) / 10.0 ); // reduce granularity of measured voltage
		ps2.phpVoltage = tempVal;
	}
	else
	{
		ps2.phpVoltage = 0;
	}
	testParamLimits(1, ps2.phpVoltage, ++test_num, "Vphp2");
#else
	testParamLimits(0, 0, ++test_num, "Vphp2");
#endif
	
	/////////////////////////////////////
	// php/output current
	/////////////////////////////////////	
	adc_disable(&adc_inst);
	configure_adc(ADC_REFERENCE_INT1V);
	/*
	tempVal = read_adc_pin(PIN_IPHP1);
	tempVal = tempVal * IPHP1_SCALING_FACTOR;// * ps1.standbyVoltage;
	ps1.outputCurrent = tempVal;
	testParamLimits(1, ps1.outputCurrent, ++test_num, "Iphp1");
	*/

#ifdef HIGH_PERFORMANCE		
	/*	
	tempVal = read_adc_pin(PIN_IPHP2);
	tempVal = tempVal * IPHP2_SCALING_FACTOR;// * ps1.standbyVoltage;
	ps2.outputCurrent = tempVal;
	*/
	adc_disable(&adc_inst);
	configure_adc(ADC_REF_VOLTAGE);
	//testParamLimits(1, ps2.outputCurrent, ++test_num, "Iphp2");
#else
	testParamLimits(0, 0, ++test_num, "Iphp2");
#endif
	setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, OFF);	  // disable output to php1
	setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, OFF);	  // disable output to php2		
		
	/////////////////////////////////////
	// comm interfaces
	/////////////////////////////////////
		// i2c
		// uart

	/////////////////////////////////////
	// heaters
	/////////////////////////////////////
		// cond
		// door

	/*
	fanStop_emc2305(&fan1);
	fanStop_emc2305(&fan2);
	fanStop_emc2305(&fan3);
	fanStop_emc2305(&fan4);		
	*/		
		
	// update status
	for (uint8_t n=0; n<SELF_TEST_RESULTS_ARRAY_SIZE; n++)
	{
		self_test_status = self_test_status | (test_results_array[n]<<n);
	}
	self_test_status_hi = (self_test_status & 0xffff0000)>>16;
	self_test_status_lo = (self_test_status & 0x0000ffff);
	dbg_info("\n  test status is %#8lx", self_test_status);
	dbg("\n[self test complete]");
	
	selfTestIsRunning = 0;
		
	return 1;			
}

bool testParamLimits(bool run, float input, uint8_t test_num, const char * test_name)
{	
	float upr_lim_val = 0;
	float lwr_lim_val = 0;
	
	upr_lim_val = testUprLimit[test_num];
	lwr_lim_val = testLwrLimit[test_num];
	
	if (run)
	{
		if (input<=(upr_lim_val) && input>=lwr_lim_val)
			test_results_array[test_num] = 1;
		else
			test_results_array[test_num] = 0;
		
		dbg_info("\n  %-2d %s\t%s\t%4.2f\t(%4.2f, %4.2f)",
		 test_num, test_name, test_results_array[test_num]==true?"pass":"FAIL", input, lwr_lim_val, upr_lim_val);
	}
	else
	{
		test_results_array[test_num] = 1;
		dbg_info("\n  %-2d %s\tskip", test_num, test_name);
	}
	
	return 1;
}
