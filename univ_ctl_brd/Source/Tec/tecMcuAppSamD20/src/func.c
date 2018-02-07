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

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
uint8_t ovErrorCount = 0;
uint8_t uvErrorCount = 0;
uint8_t VccErrorCount = 0;
bool doorOpenPresentStatus = CLOSED;
bool doorOpenPreviousStatus = CLOSED;
bool inhibitChamberFan = OFF;
uint32_t inhibitChamberFanTimestamp = 0;
uint32_t inhibitChamberFanTimeoutTimestamp = 0;
float chamberTempDifference = 0;
uint16_t cha1SnsErrCnt = 0;
uint16_t cha2SnsErrCnt = 0;
uint16_t rejSnsErrCnt = 0;
uint16_t rejCritErrCnt = 0;
uint16_t maxSnsErrorCount = MAX_SENSOR_ERROR_COUNT;
uint32_t doorOpenCount_Tec = 0;
uint16_t doorOpenCount_1Hr = 0;
uint16_t doorOpenCount1HrLimit = DEFAULT_DOOR_OPEN_CNT_1HR_LIMIT;
bool enablePwrCap = false;
bool previousEnablePwrCap = false;
uint32_t enablePwrCapTimeoutTimestamp = 0;
float acceptHtxTemperature = 0;
uint8_t bulk_voltage_max = BULK_VOLTAGE_48V_MAX;
uint8_t bulk_voltage_nom = BULK_VOLTAGE_48V_NOM;
uint8_t bulk_voltage_min = BULK_VOLTAGE_48V_MIN;
uint8_t boost_idx_offset = 0;
const char *psNameString[3] = {"Phononic_mw1000_48", "Phononic_mw500_41",	"Phononic_ma350_36"};
uint8_t sensor_state_tcha = STATE_RESET;
uint8_t prev_sensor_state_tcha = STATE_RESET;
uint8_t sensor_state_trej = STATE_RESET;
uint8_t prev_sensor_state_trej = STATE_RESET;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
uint32_t presentTimeInSec()
{
	uint32_t seconds = 0;
	rtc_calendar_get_time(&currentTime);
	seconds = currentTime.day * 24 * 60 * 60;
	seconds += currentTime.hour * 60 * 60;
	seconds += currentTime.minute * 60;
	seconds += currentTime.second;
	return seconds;
}

uint32_t updateTimeInSec(uint32_t last_time)
{
	uint32_t present_time = 0;
	uint32_t ntime = 0;
	
	present_time = presentTimeInSec();
	if (present_time >= last_time)
	{
		ntime = present_time - last_time;
	}
	else
	{
		ntime = present_time;
	}	
	
	return ntime;
}

// brief read a single-ended ADC pin
// reads ADC pin using average mode
// param ADC pin to be read.
uint16_t read_adc_pin(uint8_t pin)
{	
	uint16_t adc_result = 0;
	uint32_t adc_avg = 0;
	
	adc_set_positive_input(&adc_inst, pin);
	delay_ms(1);

	for (int n=0; n<16; n++)
	{
		/* Start an ADC conversion */
		adc_start_conversion(&adc_inst);
		while (adc_read(&adc_inst, &adc_result) != STATUS_OK) {
		}
		//adc_result = adc_result >> 4;
		delay_ms(1);
		adc_avg += adc_result;
	}
	adc_avg /= 16;
	
	return (uint16_t)adc_avg;
}

void readSensors(void)
{
    float tempVal = 0;

	/////////////////////////////////////
	// standby voltage (VCC)
	/////////////////////////////////////
	// Avref may decrease when operating on battery power
	// correction factor must be used to get accurate adc readings
	adc_disable(&adc_inst);
	configure_adc(ADC_REFERENCE_INT1V);
	tempVal = read_adc_pin(ADC_POSITIVE_INPUT_SCALEDIOVCC);	// 0.25*ADC supply voltage
	tempVal *= VCCMON_SCALING_FACTOR;
	adc_disable(&adc_inst);
	configure_adc(ADC_REF_VOLTAGE);
	
	/*
	if ( (tempVal>MAX_VCC_VOLTAGE) || (tempVal<0) )
	{
		VccErrorCount++;
		ps1.standbyVoltage = 3.08;
	}
	else
	{
		VccErrorCount = 0;
	}
	*/
	
	// reduce granularity of standby voltage
	if (tempVal > 0.0)
	{
		tempVal = ( round(tempVal * 100.0) / 100 );
		ps1.standbyVoltage = tempVal;
	}
	else
	{
		ps1.standbyVoltage = 0;
	}
	
	// DEGUB !!!!!!!!!!!!!
	//ps1.standbyVoltage = 3.08;
	// DEGUB !!!!!!!!!!!!!

	/////////////////////////////////////
	// temperatures
	/////////////////////////////////////		
	if (!enableTempOverride)
    {
		tempVal = read_adc_pin(PIN_TCHA1);
		tempVal = ADCtoTemperature(tempVal, chamberThermistorBeta);
		chamber1.chamberTempRaw = tempVal;
		tempVal = tempVal + chamber1.chamberOffset;
		chamber1.chamberTemp = tempVal;
		
		tempVal = read_adc_pin(PIN_TCHA2);
		tempVal = ADCtoTemperature(tempVal, chamberThermistorBeta);
		chamber2.chamberTempRaw = tempVal;		
		tempVal = tempVal + chamber2.chamberOffset;
		chamber2.chamberTemp = tempVal;
				
		tempVal = read_adc_pin(PIN_TGLYC1);
		tempVal = ADCtoTemperature(tempVal, BCOEFFICIENT_TGLYCOL);
		tempVal = tempVal + chamber1.glycolOffset;
		chamber1.glycolTemp = tempVal;
		
		tempVal = read_adc_pin(PIN_TREJ1);
		tempVal = ADCtoTemperature(tempVal, rejectThermistorBeta);
		chamber1.rejectTemp = tempVal;
		
		tempVal = read_adc_pin(PIN_TREJ2);
		tempVal = ADCtoTemperature(tempVal, rejectThermistorBeta);
		chamber2.rejectTemp = tempVal;
		
		tempVal = read_adc_pin(PIN_COLD2_SNS);
		tempVal = ADCtoTemperature(tempVal, rejectThermistorBeta);
		condensateTemp = tempVal;			
    }	

	/////////////////////////////////////
	// php/output voltage
	/////////////////////////////////////
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
	
	/////////////////////////////////////
	// php/output current
	/////////////////////////////////////	
	adc_disable(&adc_inst);
	configure_adc(ADC_REFERENCE_INT1V);
	/*
	tempVal = read_adc_pin(PIN_IPHP1);
	tempVal = tempVal * IPHP1_SCALING_FACTOR;// * ps1.standbyVoltage;
	ps1.outputCurrent = tempVal;
	tempVal = read_adc_pin(PIN_IPHP2);
	tempVal = tempVal * IPHP2_SCALING_FACTOR;// * ps1.standbyVoltage;
	ps2.outputCurrent = tempVal;
	*/
	adc_disable(&adc_inst);
	configure_adc(ADC_REF_VOLTAGE);
		
	/////////////////////////////////////
	// bulk voltage
	/////////////////////////////////////	
	tempVal	= read_adc_pin(PIN_VBULK1);
	tempVal = tempVal * VBULK1_SCALING_FACTOR * ps1.standbyVoltage;
	ps1.bulkVoltage = tempVal;
	
	/////////////////////////////////////
	// logic voltage (3V3)
	/////////////////////////////////////	
	/*
	tempVal = read_adc_pin(PIN_3V3MON);
	tempVal = tempVal * LOGIC3V3_SCALING_FACTOR * ps1.standbyVoltage;
	if (tempVal > 0.0)
	{
		tempVal = ( round(tempVal * 100.0) / 100 );
		ps1.logicVoltage = tempVal;
	}
	else
	{
		ps1.logicVoltage = 0;
	}
	*/
		
	// DEBUG !!!!!!!!!!!!!!!!!
	//ps1.logicVoltage = 0;//3.3;  // set this high to prevent false slp pin alarm
	// until sensors are read on rev2 ps hw
	// DEBUG !!!!!!!!!!!!!!!!!	

	/////////////////////////////////////
	// battery voltage (VBAT)
	/////////////////////////////////////	
	//port_pin_set_output_level_EX(PIN_ENBL_VBATMON, HIGH, enableManualOverride);
	//delay_us(1);
	//tempVal = read_adc_pin(PIN_VBATMON);
	//port_pin_set_output_level_EX(PIN_ENBL_VBATMON,  LOW, enableManualOverride);
	//tempVal = tempVal * VBAT_SCALING_FACTOR * ps1.standbyVoltage;	
	//if ( tempVal > 0.0)
	//{
	//	tempVal = ( round(tempVal * 100.0) / 100 );
	//	ps1.batteryVoltage = tempVal;
	//}
	//else
	//{
		ps1.batteryVoltage = 0;
	//}

	/////////////////////////////////////
	// humidity
	/////////////////////////////////////
	/*
	tempVal	= read_adc_pin(PIN_HS_HUM);
	tempVal = tempVal * HUMIDITY_SCALING_FACTOR * ps1.standbyVoltage;
	tempVal = (0.057 * (tempVal * 1000)) - 31.0;
	tempVal = round(tempVal);
	if (tempVal > 100.0)
	{
		tempVal = 100.0;
	}
	humidityPercentage = tempVal;
	*/	

	/////////////////////////////////////
	// output power calculation
	/////////////////////////////////////
	ps1.outputPower = ps1.phpVoltage * ps1.outputCurrent;
	ps2.outputPower = ps2.phpVoltage * ps2.outputCurrent;
	
	/////////////////////////////////////
	// uniformity
	/////////////////////////////////////
	chamberTempDifference = fabs(chamber2.chamberTempRaw - chamber1.chamberTempRaw);
	acceptHtxTemperature = 0;//chamber1.chamberTemp;
			
	return;
}

// convert ADC sensor reading (bits) to temperature (degC)
float ADCtoTemperature(float average, uint16_t nWhichBeta)
{
	// convert the value to resistance
	average = 4095 / average - 1;
	average = SERIESRESISTOR / average;
	
	// convert resistance to temperature
	float steinhart;
	steinhart = average / THERMISTORNOMINAL;            // (R/Ro)
	steinhart = log(steinhart);                         // ln(R/Ro)
	steinhart /= nWhichBeta;                            // 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);   // + (1/To)
	steinhart = 1.0 / steinhart;                        // Invert
	steinhart -= 273.15;                                // convert to C

	return steinhart;
}

void psHwController()
{	
	psHwControl_27V_freezer();
}

void modeStateMachine(struct pid_instance *pid_inst, struct ps_vars *ps_inst, struct chamber_vars *ch_inst)
{
	float rejectSense = 0.0;

	// select which reject sensor is used as indicator for foldback/shutdown
	#ifdef HIGH_PERFORMANCE
	rejectSense = max(chamber1.rejectTemp, chamber2.rejectTemp);
	#endif
	
	// if both chamber sensors are insane or reject fan failed, then run cooling
	if ( ( ( (errorRegister & ERROR_INSANE_TCHA1 ) == ERROR_INSANE_TCHA1 )   &&
	       ( (errorRegister & ERROR_INSANE_TCHA2 ) == ERROR_INSANE_TCHA2 ) ) )
	{
		ch_inst->mode_state = STATE_HIGH_COP;
		ch_inst->previousModeState = STATE_HIGH_COP;
		#ifdef HIGH_PERFORMANCE
		ps_inst->desiredVoltage = VMAXCOP_DEFAULT;
		#endif
		return;
	}
	
	// auto control mechanism
	//
	//if (ch_inst->deactivateAutoControl == false) {

	// state machine to determine operating mode
	//
	// reset //////////////////////////////////
	if (ch_inst->mode_state == STATE_RESET) {
		ch_inst->mode_state = STATE_IDLE;
		ps_inst->desiredVoltage = 0;
		//ch_inst->fanState = OFF;
		ch_inst->previousModeState = STATE_RESET;
		
	// idle ///////////////////////////////////
	} else if (ch_inst->mode_state == STATE_IDLE) {
		if ( ((controlTemperature > ch_inst->setpoint-ch_inst->deadbandLow)) && (rejectSense < ch_inst->rejectLowLimit) ) {
			ch_inst->mode_state = STATE_HIGH_COP;
		}
		if (ch_inst->previousModeState != STATE_IDLE) {
			//pid_inst->inAuto = false;
			/*
			pidSetTunings(pid_inst, pid_inst->kp_copq, pid_inst->ki_copq, pid_inst->kd_copq);
			pidSetOutputLimits(pid_inst,
			MINIMUM_ALLOWED_VOLTAGE - ch_inst->VmaxCOP,
			maxOutputVoltage - ch_inst->VmaxCOP//BULK_VOLTAGE_MAX - ch_inst->VmaxCOP
			);
			*/
			//ch_inst->fanState = OFF;
		}
		ps_inst->desiredVoltage = 0;
		ch_inst->previousModeState = STATE_IDLE;
		
	// high q /////////////////////////////////
	} else if (ch_inst->mode_state == STATE_HIGH_Q) {
		if ( (rejectSense >= ch_inst->rejectLimitCritical) ) {
			ch_inst->mode_state = STATE_IDLE;
		} else if ( (rejectSense >= ch_inst->rejectLimit) ) {
			ch_inst->mode_state = STATE_VAR_Q;			
		} else if ( (controlTemperature <= ch_inst->setpoint) ) {
			ch_inst->mode_state = STATE_HIGH_COP;
		}
		if (ch_inst->previousModeState != STATE_HIGH_Q) {
			//pid_inst->inAuto = true;
			pidSetTunings(pid_inst, pid_inst->kp_highq, pid_inst->ki_highq, pid_inst->kd_highq);
			pidSetOutputLimits(pid_inst,
			MINIMUM_ALLOWED_VOLTAGE - ch_inst->VmaxCOP,
			maxOutputVoltage - ch_inst->VmaxCOP
			);
			//ch_inst->fanState = ON;
		}
		if (ch_inst->operatingMode==BANG_BANG_MODE) {
			ps_inst->desiredVoltage = maxOutputVoltage;
			} else {
			ps_inst->desiredVoltage = ch_inst->VmaxCOP + pid_inst->Output;
		}
		ch_inst->previousModeState = STATE_HIGH_Q;
		
	// variable q /////////////////////////////
	} else if (ch_inst->mode_state == STATE_VAR_Q) {
		if ( (rejectSense >= ch_inst->rejectLimitCritical) ) {
			ch_inst->mode_state = STATE_IDLE;
		} else if ( (rejectSense < ch_inst->rejectLowLimit) ) {
			ch_inst->mode_state = STATE_HIGH_Q;			
		} else if ( (controlTemperature <= ch_inst->setpoint) ) {
			ch_inst->mode_state = STATE_HIGH_COP;
		}
		if (ch_inst->previousModeState != STATE_VAR_Q) {
			//pid_inst->inAuto = true;
			pidSetTunings(pid_inst, pid_inst->kp_varq, pid_inst->ki_varq, pid_inst->kd_varq);
			pidSetOutputLimits(pid_inst,
			MINIMUM_ALLOWED_VOLTAGE - ch_inst->VmaxCOP,
			ch_inst->Vvarq - ch_inst->VmaxCOP
			);
		}
		if (ch_inst->operatingMode==BANG_BANG_MODE) {
			ps_inst->desiredVoltage = maxOutputVoltage;
			} else {
			ps_inst->desiredVoltage = ch_inst->VmaxCOP + pid_inst->Output;
		}
		ch_inst->previousModeState = STATE_VAR_Q;
		
	// high cop ///////////////////////////////
	} else if (ch_inst->mode_state == STATE_HIGH_COP) {
		if ( (rejectSense >= ch_inst->rejectLimitCritical) || (controlTemperature < ch_inst->setpoint-ch_inst->deadbandLow-0.05) ) {
			ch_inst->mode_state = STATE_IDLE;
		} else if ( (controlTemperature >= ch_inst->setpoint+ch_inst->deadband) ) {
			ch_inst->mode_state = STATE_HIGH_Q;
		}
		if (ch_inst->previousModeState != STATE_HIGH_COP) {
			//pid_inst->inAuto = true;
			pidSetTunings(pid_inst, pid_inst->kp_copq, pid_inst->ki_copq, pid_inst->kd_copq);
			pidSetOutputLimits(pid_inst,
			MINIMUM_ALLOWED_VOLTAGE - ch_inst->VmaxCOP,
			maxOutputVoltage - ch_inst->VmaxCOP
			);
			//ch_inst->fanState = OFF;
		}
		if (enablePwrCap == false)
		{
			pidSetOutputLimits(pid_inst,
			MINIMUM_ALLOWED_VOLTAGE - ch_inst->VmaxCOP,
			maxOutputVoltage - ch_inst->VmaxCOP
			);
		}
		else //if (enablePwrCap == true)
		{
			pidSetOutputLimits(pid_inst,
			MINIMUM_ALLOWED_VOLTAGE - ch_inst->VmaxCOP,
			bulk_voltage_max - ch_inst->VmaxCOP
			);
		}
		if (ch_inst->operatingMode==BANG_BANG_MODE) {
			ps_inst->desiredVoltage = bulk_voltage_nom;
			} else {
			ps_inst->desiredVoltage = ch_inst->VmaxCOP + pid_inst->Output;
		}
		ch_inst->previousModeState = STATE_HIGH_COP;

	// other /////////////////////////////////
	} else {
		//pid_inst->inAuto = true;
		ch_inst->mode_state = ch_inst->previousModeState;
	}
	//}
		
	// reduce granularity of setpoint voltage to whole number
	if (ps_inst->desiredVoltage > 0.0)
	{
		ps_inst->desiredVoltage = floor( (ps_inst->desiredVoltage*2) + 0.5 ) / 2; //( round(ps_inst->desiredVoltage*10.0) / 10 );
	}
	
	return;
}

void psHwControl_rsp1000_48_freezer()
{
	// output pwm is utilized below 48v
	
	// [djb]... uint32_t bulkDutyVal = BULK_OFF_DUTY;
	uint32_t boostDutyVal = BOOST_OFF_DUTY;
	
	// control PWM and power supply signals to achieve desired PHP voltage
	//
	// out-of-bounds voltage setpoint
	if (ps1.desiredVoltage > maxOutputVoltage) {
		ps1.desiredVoltage = maxOutputVoltage;
	
	/////////////////////////////////////////////////////////////////////	
	// output disabled
	} if (ps1.desiredVoltage <= 0) {
		ps1.desiredVoltage = 0;					  // don't allow negative voltages
		setBoostEnable(&ps1, HIGH);					  // disable boost converter *
		setBoostVoltage(&ps1, BOOST_OFF_DUTY);
		ps1.boostPwmState = OFF;
		
		// [djb] there is no bulk dac-ing when using rsp1000_48 ots ps
		///*
		//if (ps1.bulkPwmState == ON) {
			//turnOffBulkPwm(&ps1);					  // set bulk to nominal
			//ps1.bulkPwmState = OFF;
			//delay_s(3);
		//}
		//*/
		setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, OFF);	  // hot
		setOutputSwitch(PIN_ENBL_OUTPUT2, GPIO_MODE, OFF);	  // cold
		
	/////////////////////////////////////////////////////////////////////
	// variable output from boost regulator
//	} else if ( (ps1.desiredVoltage > bulk_voltage_max) &&
//		(ps1.desiredVoltage <= maxOutputVoltage) ) {
		
		// [djb] there is no bulk dac-ing when using rsp1000_48 ots ps
		///*		
		//if (ps1.bulkPwmState == ON) {
			//turnOffBulkPwm(&ps1);					  // set bulk to nominal
			//ps1.bulkPwmState = OFF;
			//delay_s(3);			
		//}
		//*/
		
//		boostDutyVal = lookupBoostPwmVal(ps1.desiredVoltage);

//		setBoostVoltage(&ps1, boostDutyVal);
//		delay_ms(10);                                      // wait  for duty to settle
//		setBoostEnable(&ps1, LOW);					   // enable boost converter **
//		delay_ms(10);
//		ps1.boostPwmState = ON;
		
		///*[djb]... don't use walk-in routine
		//if (ps1.outputStatus == OFF) {
			//outputWalkin(ps_inst);
			//setOutputEnable(ps_inst, HIGH);				   // enable output to php
			//ps_inst->outputStatus = ON;
		//}
		//*/
		///*
		//setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, ON);	  // enable output to php1
		//
		//if (!inhibitOutput)
		//{
			//setOutputSwitch(PIN_ENBL_OUTPUT3, GPIO_MODE, ON);	  // enable output to php2
		//}
		//*/
		
//		setOutputDutyVals(ps1.desiredVoltage);
//		setOutputSwitch(PIN_ENBL_OUTPUT1, PWM_MODE, ps1.outputDuty);	  // enable output to php1
		
//		if (!inhibitOutput)
//		{
//			setOutputSwitch(PIN_ENBL_OUTPUT3, PWM_MODE, ps2.outputDuty);	  // enable output to php2
//		}
	
	/////////////////////////////////////////////////////////////////////	
	// constant flyback output
	} else if ( (ps1.desiredVoltage >= bulk_voltage_nom) ) {//&&
		//	    (ps1.desiredVoltage > bulk_voltage_min) ) {
		setBoostEnable(&ps1, HIGH);					   // disable boost controller **
		setBoostVoltage(&ps1, BOOST_OFF_DUTY);
		ps1.boostPwmState = OFF;

		// [djb] there is no bulk dac-ing when using rsp1000_48 ots ps
		///*		
		//if (ps1.bulkPwmState == ON) {
			//turnOffBulkPwm(&ps1);					  // set bulk to nominal
			//ps1.bulkPwmState = OFF;
			//delay_s(3);
		//}
		//*/
		//
		///*[djb]... don't use walk-in routine
		//if (ps_inst->outputStatus == OFF) {
			//outputWalkin(ps_inst);
			//setOutputEnable(ps_inst, HIGH);				   // enable output to php
			//ps_inst->outputStatus = ON;
		//}
		//*/ // ...[djb]
		///*
		//// use following routine that only turns on force air accept in high_cop mode
		
				//
		if (!inhibitOutput)
		{
			setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, ON);	  // hot
			setOutputSwitch(PIN_ENBL_OUTPUT2, GPIO_MODE, ON);	  // cold
		}
		//*/
						
//		setOutputDutyVals(ps1.desiredVoltage);
//		setOutputSwitch(PIN_ENBL_OUTPUT1, PWM_MODE, ps1.outputDuty);	  // enable output to php1
		
//		if (!inhibitOutput)
//		{
//			setOutputSwitch(PIN_ENBL_OUTPUT2, PWM_MODE, ps2.outputDuty);	  // enable output to php2
//		}

	/////////////////////////////////////////////////////////////////////
	// variable duty cycle output at minimum bulk
//	} else if ( (ps1.desiredVoltage < bulk_voltage_min) ) {
//		setBoostEnable(&ps1, HIGH);					   // disable boost controller **
//		setBoostVoltage(&ps1, BOOST_OFF_DUTY);
//		ps1.boostPwmState = OFF;

		// [djb] there is no bulk dac-ing when using rsp1000_48 ots ps
		///*		
		//bulkDutyVal = lookupBulkPwmVal(OUTPUT_PWM_VOLTAGE_THRESHOLD);	// calc bulk duty setting
//
		//setBulkVoltage(&ps1, bulkDutyVal);					// set the dac value
		//delay_ms(10);                                      // wait  for duty to settle
		//
		//if (ps1.bulkPwmState == OFF) {					// turn on pwm if not on
			//turnOnBulkPwm(&ps1);
			//setBulkVoltage(&ps1, bulkDutyVal);
			//ps1.bulkPwmState = ON;
			//delay_s(3);
		//}
		//delay_ms(10);
		//*/
				
//		setOutputDutyVals(ps1.desiredVoltage);
//		setOutputSwitch(PIN_ENBL_OUTPUT1, PWM_MODE, ps1.outputDuty);	  // enable output to php1
		
//		if (!inhibitOutput)
//		{
//			setOutputSwitch(PIN_ENBL_OUTPUT3, PWM_MODE, ps2.outputDuty);	  // enable output to php2
//		}		
		
	}
	
	return;
}

void psHwControl_27V_freezer()
{
	// out-of-bounds voltage setpoint
	if (ps1.desiredVoltage > maxOutputVoltage) {
		ps1.desiredVoltage = maxOutputVoltage;
		
	/////////////////////////////////////////////////////////////////////
	// output disabled
	} if ( (ps1.desiredVoltage <= 0) || (ps1.desiredVoltage < PCP_COLD_STAGE_MIN_OUTPUT_VOLTAGE) ) {
		
		ps1.desiredVoltage = 0;
		
		setBoostVoltage(&ps1, BOOST_OFF_DUTY);
		ps1.boostPwmState = OFF;
		
		setBoostVoltage(&ps2, BOOST_OFF_DUTY);
		ps2.boostPwmState = OFF;
		
		setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, OFF);	// cold, sideA
		setOutputSwitch(PIN_ENBL_OUTA_HOT , GPIO_MODE, OFF);	// hot,  sideA
		setOutputSwitch(PIN_ENBL_OUTB_COLD, GPIO_MODE, OFF);	// cold, sideB
		setOutputSwitch(PIN_ENBL_OUTB_HOT , GPIO_MODE, OFF);	// hot,  sideB
				
	/////////////////////////////////////////////////////////////////////
	// variable output from boost regulator
	} else {
		
		lookupBoostPwmVal(&ps1, ps1.desiredVoltage);			// cold & hot
		
		setBoostVoltage(&ps1, ps1.boostPwm);
		delay_ms(10);
		ps1.boostPwmState = ON;

		setBoostVoltage(&ps2, ps2.boostPwm);
		delay_ms(10);
		ps2.boostPwmState = ON;
		
		if (!inhibitOutput && enableSideAPcp)
		{
			setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, ON);		// cold, sidaA
			setOutputSwitch(PIN_ENBL_OUTA_HOT , GPIO_MODE, ON);		// hot,  sideA		
		}
		else
		{
			setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, OFF);	// cold, sideA
			setOutputSwitch(PIN_ENBL_OUTA_HOT , GPIO_MODE, OFF);	// hot,  sideA			
		}
		
		if (!inhibitOutput && enableSideBPcp)
		{
			setOutputSwitch(PIN_ENBL_OUTB_COLD, GPIO_MODE, ON);		// cold, sidaA
			setOutputSwitch(PIN_ENBL_OUTB_HOT , GPIO_MODE, ON);		// hot,  sideA
		}
		else
		{
			setOutputSwitch(PIN_ENBL_OUTB_COLD, GPIO_MODE, OFF);	// cold, sideA
			setOutputSwitch(PIN_ENBL_OUTB_HOT , GPIO_MODE, OFF);	// hot,  sideA
		}
	}
	
	return;
}

void setOutputDutyVals(float voltage)
{
	uint8_t idx = 0;
	uint8_t idx_php1 = 0;
	uint8_t idx_php2 = 0;
	float sp = 4;	
	uint8_t sp_idx = 0;
	
	if (voltage < 0)
	{
		voltage = 0;
	}
	if (voltage > maxOutputVoltage)
	{
		voltage = maxOutputVoltage;
	}

	sp = round(chamber1.setpoint * 10) / 10;
	sp_idx = (uint8_t) ( (2*sp) - (2*DEFAULT_MIN_ALLOWED_TCHA) );
	
	idx = round(voltage);

	// php1 (forced air)	
	idx_php1 = idx + round(ps1.phpOutputDutyOffset) + setpoint_power_adjust_offset_table[sp_idx];
	if (idx_php1 < 0)
	{
		idx_php1 = 0;
	}
	else if (idx_php1 > OUTPUT_DUTY_LOOKUP_TABLE_SIZE)
	{
		idx_php1 = OUTPUT_DUTY_LOOKUP_TABLE_SIZE-1;
	}
	ps1.outputDuty = php1_output_duty_lookup_table[idx_php1];
	if (ps1.outputDuty >= 100.0)
	{
		ps1.outputDuty = 100.0;
	}
	else if (ps1.outputDuty <= 0.0)
	{
		ps1.outputDuty = 0.0;
	}	
		
	// php2 (cold wall)
	idx_php2 = idx + round(ps2.phpOutputDutyOffset) + setpoint_power_adjust_offset_table[sp_idx];
	if (idx_php2 < 0)
	{
		idx_php2 = 0;
	}
	else if (idx_php2 > OUTPUT_DUTY_LOOKUP_TABLE_SIZE)
	{
		idx_php2 = OUTPUT_DUTY_LOOKUP_TABLE_SIZE-1;
	}
	ps2.outputDuty = php2_output_duty_lookup_table[idx_php2];
	if (ps2.outputDuty >= 100.0)
	{
		ps2.outputDuty = 100.0;
	}
	else if (ps2.outputDuty <= 0.0)
	{
		ps2.outputDuty = 0.0;
	}	
}

uint16_t lookupBoostPwmVal(struct ps_vars *ps_inst, float voltage)
{
	uint16_t pwmVal = 0;
	float f_idx = 0.;
	uint8_t idx = 0;
	
	if (voltage <= ps_inst->minVoltage)
	{
		voltage = ps_inst->minVoltage;
	}
	if (voltage >= ps_inst->maxVoltage)
	{
		voltage = ps_inst->maxVoltage;
	}
	voltage = floor( (voltage*2) + 0.5 ) / 2;
	f_idx  = voltage - ps_inst->minVoltage;
	if (f_idx < 0) { f_idx = 0; }
	f_idx = 2 * f_idx;
	idx = (uint8_t) f_idx;
	
	if (idx < 0) { idx = 0; }
	
	if (idx >= PS_BOOST_PWM_LOOKUP_TABLE_SIZE)
	{
		idx = PS_BOOST_PWM_LOOKUP_TABLE_SIZE-1;
	}
	
	ps1.boostPwm = ps_reg1_boost_pwm_lookup_table[idx];
	ps2.boostPwm = ps_reg2_boost_pwm_lookup_table[idx];
	
	if (pwmVal >= PWM_MAX_DUTY_VAL)
	{
		pwmVal = PWM_MAX_DUTY_VAL;
	}
	else if (pwmVal <= 0)
	{
		pwmVal = 1;
	}
	return pwmVal;
}


uint16_t lookupBulkPwmVal(float voltage)
{
	uint16_t pwmVal = 0;
	uint8_t idx = 0;
	
	if (voltage >= bulk_voltage_nom)
	{
		voltage = bulk_voltage_nom;
	}
	else if (voltage <= MINIMUM_ALLOWED_VOLTAGE)
	{
		voltage = MINIMUM_ALLOWED_VOLTAGE;
	}
	voltage = round(voltage);
	idx = (uint8_t) (bulk_voltage_nom - voltage);
	if (idx < 0)
	{
		idx = 0;
	}
	else if (idx > PS_BULK_PWM_LOOKUP_TABLE_SIZE)
	{
		idx = PS_BULK_PWM_LOOKUP_TABLE_SIZE-1;
	}
	pwmVal = ps_bulk_pwm_lookup_table[idx];
	if (pwmVal >= PWM_MAX_DUTY_VAL)
	{
		pwmVal = PWM_MAX_DUTY_VAL;
	}
	else if (pwmVal <= 0)
	{
		pwmVal = 1;
	}
	return pwmVal;
}

void setOutputSwitch(uint8_t pin, uint8_t pwmMode, uint8_t val)
{
	if (pin == PIN_ENBL_OUTB_COLD || pin == PIN_ENBL_OUTB_HOT || pin == PIN_ENBL_OUTA_COLD || pin == PIN_ENBL_OUTA_HOT )
	{
		if ( (pwmMode==GPIO_MODE) || (val>=100) || (val==0) )
		{
			// gpio mode
		
			// disable pwm output
			//if (pin == PIN_ENBL_OUTPUT1)
			//{
				//tc_disable(&tc5_module);
			//}
			if (pin == PIN_ENBL_OUTA_COLD)
			{
				tc_disable(&tc0_module);
			}
			
			// if pin not configured as gpio, then configure pin as gpio output
			if (system_pinmux_pin_get_mux_position(pin) != SYSTEM_PINMUX_GPIO)
			{
				struct system_pinmux_config config_pinmux;
				system_pinmux_get_config_defaults(&config_pinmux);
				config_pinmux.mux_position = SYSTEM_PINMUX_GPIO;
				config_pinmux.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
				system_pinmux_pin_set_config(pin, &config_pinmux);
				struct port_config config_port_pin;
				port_get_config_defaults(&config_port_pin);
				config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
				port_pin_set_config(pin, &config_port_pin);			
			}
			
			// bound to boolean
			if (val > 0)
			{
				val = 1;
			}
		
			// set output pin to val1
			port_pin_set_output_level_EX(pin, val, enableManualOverride);
			
			// update status
			if (pin == PIN_ENBL_OUTB_HOT)
			{
				ps2.outputStatus = GPIO_MODE;
			}
			if (pin == PIN_ENBL_OUTB_COLD)
			{
				ps1.outputStatus = GPIO_MODE;
			}			
			if (pin == PIN_ENBL_OUTA_COLD)
			{
				ps1.outputStatus = GPIO_MODE;
			}
			if (pin == PIN_ENBL_OUTA_HOT )
			{
				ps2.outputStatus = GPIO_MODE;
			}			
		}
		else
		{
			// pwm mode
		
			//if (pin == PIN_ENBL_OUTPUT1)
			//{
				//// configure as tc5 pwm output
				//struct system_pinmux_config config_pinmux;
				//system_pinmux_get_config_defaults(&config_pinmux);
				//config_pinmux.mux_position = PINMUX_PB14E_TC5_WO0;
				//config_pinmux.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
				//system_pinmux_pin_set_config(pin, &config_pinmux);				
				//tc_enable(&tc5_module);
				//
				//// set duty
				//pwm5_cmp_val = (val * PWM_MAX_DUTY_VAL) / 100;	
				//
				//// set status
				//ps1.outputStatus = PWM_MODE;		
			//}
			if (pin == PIN_ENBL_OUTA_COLD)
			{
				// configure as tc0 pwm output
				struct system_pinmux_config config_pinmux;
				system_pinmux_get_config_defaults(&config_pinmux);
				config_pinmux.mux_position = PINMUX_PB31F_TC0_WO1;
				config_pinmux.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
				system_pinmux_pin_set_config(pin, &config_pinmux);
				
				// enable
				tc_enable(&tc0_module);								
				
				// set duty
				pwm0_cmp_val = (val * PWM_MAX_DUTY_VAL) / 100;	
				
				// set status
				ps2.outputStatus = PWM_MODE;
			}
		}
	}
}

void setBoostEnable(struct ps_vars *ps_inst, bool state)
{
	//bool bFlippedstate = ~state;
	
//	return;
	
	//flip logic from 6 bottle to next gen architecture 
	if (ps_inst==&ps1) {
		port_pin_set_output_level_EX(PIN_ENBL_BUCK1, state, enableManualOverride);
	}
/*
	 else {
		port_pin_set_output_level_EX(PIN_ENBL_BOOST2, bFlippedstate, enableManualOverride);
	}
*/	
	delay_ms(1);
	
	return;
}

void setBoostVoltage(struct ps_vars *ps_inst, uint32_t dutyVal)
{
	if (dutyVal >= BOOST_OFF_DUTY) {
		dutyVal = BOOST_OFF_DUTY;
		} else if (dutyVal <= 1 ) {
		dutyVal = 1;
	}
	
	ps_inst->boostPwmTarget = dutyVal;
	ps_inst->boostPwm = dutyVal;
	
	// develop ramping routing HERE !!!!!!
	
	if (ps_inst == &ps1) {
		pwm2_0_cmp_val = ps_inst->boostPwm;
	} else {
		pwm2_1_cmp_val = ps_inst->boostPwm;
	}

	return;
}

void setBulkVoltage(struct ps_vars *ps_inst, uint32_t dutyVal)
{	
/*	
	if (dutyVal >= PWM_MAX_DUTY_VAL) {
		dutyVal = PWM_MAX_DUTY_VAL;
	} else if (dutyVal <= BULK_OFF_DUTY) {
		dutyVal = BULK_OFF_DUTY;
	}
	
	ps_inst->bulkPwmTarget = dutyVal;
	ps_inst->bulkPwm = dutyVal;
	
	// develop ramping routing HERE !!!!!!
	
//	if (ps_inst == &ps1) {
		//pwm5_cmp_val = ps_inst->bulkPwm; // was Bulk pwm setting now not used
		pwm1_1_cmp_val = ps_inst->bulkPwm;
//	} else {
		//pwm4_cmp_val = ps_inst->bulkPwm; // was not used now Bulk pwm setting
//		pwm1_1_cmp_val = ps_inst->bulkPwm;
//	}
	
	return;
*/
}


void setOutputEnable(struct ps_vars *ps_inst, bool state)
{
	//old version
	if (ps_inst==&ps1) {
		port_pin_set_output_level_EX(PIN_ENBL_OUTPUT1, state, enableManualOverride);
		port_pin_set_output_level_EX(PIN_ENBL_OUTA_COLD, state, enableManualOverride); // 156L 
	}
/*  else {
		port_pin_set_output_level_EX(PIN_ENBL_OUTPUT2, state, enableManualOverride);
	}
*/
	delay_ms(1);
	
	return;
}


void outputWalkin(struct ps_vars *ps_inst)
{
	uint8_t ld_pin = PIN_ENBL_LOAD2;
	uint8_t out_pin = PIN_ENBL_OUTPUT1;
	
//	if (ps_inst == &ps1) {
		ld_pin = PIN_ENBL_LOAD2;
		out_pin = PIN_ENBL_OUTPUT1;
		
/*	} else {
		ld_pin = PIN_ENBL_LOAD2;
		out_pin = PIN_ENBL_OUTPUT2;
	}
*/

	port_pin_set_output_level_EX(ld_pin, HIGH, enableManualOverride);		// enable dummy bulk load resistor
	port_pin_set_output_level_EX(out_pin, LOW, enableManualOverride);		// ensure output is disabled
	ps_inst->outputStatus = OFF;
	delay_ms(750);															// wait to exit burst mode
	for (int dly=1; dly<100; dly++) {
		port_pin_set_output_level_EX(out_pin, HIGH, enableManualOverride);
		delay_us(dly*25);													// incrementally increase high time
		port_pin_set_output_level_EX(out_pin, LOW, enableManualOverride);
		delay_ms(10);
	}
	port_pin_set_output_level_EX(out_pin, HIGH, enableManualOverride);		// end with output fully enabled
	ps_inst->outputStatus = ON;
	
//	if (ps_inst != &ps1) {
		port_pin_set_output_level_EX(ld_pin, LOW, enableManualOverride);	// disable bulk load resistor
//	}

	return;
}
/*
void outputWalkinEx(struct ps_vars *ps_inst, uint8_t nPin)
{
	uint8_t ld_pin = PIN_ENBL_LOAD2;
	uint8_t out_pin = PIN_ENBL_OUTPUT1;

//	if (ps_inst == &ps1) {
		ld_pin = PIN_ENBL_LOAD2;
		out_pin = PIN_ENBL_OUTPUT1;
//		} else {
//		ld_pin = PIN_ENBL_LOAD2;
//		out_pin = PIN_ENBL_OUTPUT2;
//	}

	port_pin_set_output_level_EX(ld_pin, HIGH, enableManualOverride);		// enable dummy bulk load resistor
	port_pin_set_output_level_EX(out_pin, LOW, enableManualOverride);		// ensure output is disabled
	ps_inst->outputStatus = OFF;
	delay_ms(750);															// wait to exit burst mode
	for (int dly=1; dly<100; dly++) {
		port_pin_set_output_level_EX(out_pin, HIGH, enableManualOverride);
		delay_us(dly*25);													// incrementally increase high time
		port_pin_set_output_level_EX(out_pin, LOW, enableManualOverride);
		delay_ms(10);
	}
	port_pin_set_output_level_EX(out_pin, HIGH, enableManualOverride);		// end with output fully enabled
	ps_inst->outputStatus = ON;
	
	if (ps_inst != &ps1) {
		port_pin_set_output_level_EX(ld_pin, LOW, enableManualOverride);	// disable bulk load resistor
	}
	
	return;
}
*/

void turnOffBulkPwm(struct ps_vars *ps_inst)
{
	// perform bulk pwm walk-down HERE !!!
	
	// for now...
	setBulkVoltage(ps_inst, BULK_OFF_DUTY);
	
	return;
}

void turnOnBulkPwm(struct ps_vars *ps_inst)
{
	// if necessary, perform bulk pwm walk-in HERE !!!
	// stop tc
	// configure tc output as dio
	// perform walk-in loop
	// configure tc output as pwm
	// set pwm value
	// reenable tc

	return;
}

void checkForSensorFaults()
{	
	// insane sensor, chamber1 //////////////////////////
	if (chamber1.chamberTemp<TSNS_INSANE_LOW_LIMIT) 
	{
		cha1SnsErrCnt++;
	}
	else
	{
		cha1SnsErrCnt = 0;
	}
	
	// insane sensor, chamber2 //////////////////////////
	if (chamber2.chamberTemp<TSNS_INSANE_LOW_LIMIT)
	{
		cha2SnsErrCnt++;
	}
	else
	{
		cha2SnsErrCnt = 0;
	}	
	
	// insane sensor, reject 1 or 2 /////////////////////
	if (chamber1.rejectTemp<TSNS_INSANE_LOW_LIMIT || 
		chamber2.rejectTemp<TSNS_INSANE_LOW_LIMIT)
	{
		rejSnsErrCnt++;
	}	
	else
	{
		rejSnsErrCnt = 0;
	}

	// reject overtemp //////////////////////////////////	
	if (chamber1.rejectTemp>=REJECT_LIMIT_CRITICAL_DEFAULT || 
		chamber2.rejectTemp>=REJECT_LIMIT_CRITICAL_DEFAULT)
	{
		rejCritErrCnt++;
	}	
	else
	{
		rejCritErrCnt = 0;
	}	

	// update error register for fan errors /////////////
	if (maxSnsErrorCount > 0)
	{
		if (cha1SnsErrCnt >= maxSnsErrorCount)
		{
			errorRegister |= ERROR_INSANE_TCHA1;
		}
		else
		{
			errorRegister &= ~ERROR_INSANE_TCHA1;
		}
		
		if (cha2SnsErrCnt >= maxSnsErrorCount)
		{
			errorRegister |= ERROR_INSANE_TCHA2;
		}
		else
		{
			errorRegister &= ~ERROR_INSANE_TCHA2;
		}
		
		if (rejSnsErrCnt >= maxSnsErrorCount)
		{
			errorRegister |= ERROR_INSANE_TREJ;
		}
		else
		{
			errorRegister &= ~ERROR_INSANE_TREJ;
		}
		
		if (rejCritErrCnt >= maxSnsErrorCount)
		{
			errorRegister |= ERROR_REJECT_CRITICAL;
		}
		else
		{
			errorRegister &= ~ERROR_REJECT_CRITICAL;
		}		
	}	
}

void handleDoorEvents()
{
	doorOpenPresentStatus = (bool) doorOpenStatus;

	// door opening event
	if (doorOpenPresentStatus == OPEN && doorOpenPreviousStatus == CLOSED)
	{
		doorOpenCount_Tec++;
		doorOpenCount_1Hr++;
		
		// set chamber fan inhibit
		inhibitChamberFan = ON;
		inhibitChamberFanTimestamp = 0;		
		fanStop_emc2305(&fan4);
		
		// set power cap
		enablePwrCap = true;
		
		// start timeout counters
		inhibitChamberFanTimeoutTimestamp = presentTimeInSec();
		enablePwrCapTimeoutTimestamp = presentTimeInSec();
	}
	
	// door open
	else if (doorOpenPresentStatus == OPEN && doorOpenPreviousStatus == OPEN)
	{	
		// clear timeout when expired
		if (presentTimeInSec() > CHAMBER_FAN_OFF_TIMEOUT+inhibitChamberFanTimeoutTimestamp)
		{
			inhibitChamberFan = OFF;
			fanStart_emc2305(&fan4);
			inhibitChamberFanTimeoutTimestamp = 0;
		}			
	}
	
	// door closing event
	else if (doorOpenPresentStatus == CLOSED && doorOpenPreviousStatus == OPEN)
	{
		// start inhibit timer
		inhibitChamberFanTimestamp = presentTimeInSec();
		
		// clear timeout
		inhibitChamberFanTimeoutTimestamp = 0;
	}

	// door closed
	else if (doorOpenPresentStatus == CLOSED && doorOpenPreviousStatus == CLOSED)
	{
		// clear inhibit timer when expired
		if (presentTimeInSec() > MIN_CHAMBER_FAN_OFFTIME+inhibitChamberFanTimestamp)
		{
			inhibitChamberFan = OFF;
			fanStart_emc2305(&fan4);
			inhibitChamberFanTimestamp = 0;
		}
	}	
	
	if (presentTimeInSec() > ((uint32_t)pwrCapTimeoutInMins*60)+enablePwrCapTimeoutTimestamp)
	{
		enablePwrCap = false;
		enablePwrCapTimeoutTimestamp = 0;
	}
	
	doorOpenPreviousStatus = doorOpenPresentStatus;	
	previousEnablePwrCap = enablePwrCap;
}

/*
void chamberSensorEDFI(void)
{
	#ifdef HIGH_PERFORMANCE
	if (chamber2.chamberTemp<TSNS_INSANE_LOW_LIMIT)
	{
		controlTemperature = chamber1.chamberTemp;
		sensor_state_tcha==STATE_SNS_NORM_TCHA;
	}
	else
	{
		controlTemperature = chamber2.chamberTemp;
		sensor_state_tcha==STATE_SNS_FAIL1;
	}
	#endif
				
	#ifdef GENERAL_PURPOSE
	if (sensor_state_tcha==STATE_RESET)
	{
		sensor_state_tcha = STATE_SNS_NORM_TCHA;
		prev_sensor_state_tcha = STATE_RESET;
	}
	else if (sensor_state_tcha==STATE_SNS_NORM_TCHA)
	{
		if (prev_sensor_state_tcha!=STATE_SNS_NORM_TCHA)
		{
			chamber1.deactivateAutoControl = false;
		}
		if (chamber2.chamberTemp<TSNS_INSANE_LOW_LIMIT)
		{
			sensor_state_tcha = STATE_SNS_FAIL1;
		}
		controlTemperature = chamber2.chamberTemp;
		prev_sensor_state_tcha = STATE_SNS_NORM_TCHA;
	}
	else if (sensor_state_tcha==STATE_SNS_FAIL1)
	{
		if (prev_sensor_state_tcha!=STATE_SNS_FAIL1)
		{
			chamber1.deactivateAutoControl = false;
		}
		if (chamber2.chamberTemp>TSNS_INSANE_LOW_LIMIT)
		{
			sensor_state_tcha = STATE_SNS_NORM_TCHA;
		}
		else if ( (acceptHtxTemperature+2)<TSNS_INSANE_LOW_LIMIT )
		{
			sensor_state_tcha = STATE_SNS_FAIL2;
		}
		controlTemperature = acceptHtxTemperature+2;
		prev_sensor_state_tcha = STATE_SNS_FAIL1;
	}
	else if (sensor_state_tcha==STATE_SNS_FAIL2)
	{
		if (prev_sensor_state_tcha!=STATE_SNS_FAIL2)
		{
			chamber1.deactivateAutoControl = true;
			ps1.desiredVoltage = SNS_FAIL_VSET;
		}
		if (chamber2.chamberTemp>TSNS_INSANE_LOW_LIMIT)
		{
			sensor_state_tcha = STATE_SNS_NORM_TCHA;
		}
		else if ( (acceptHtxTemperature+2)>TSNS_INSANE_LOW_LIMIT )
		{
			sensor_state_tcha = STATE_SNS_FAIL1;
		}
		prev_sensor_state_tcha = STATE_SNS_FAIL2;
	}
	else
	{
		controlTemperature = chamber2.chamberTemp;
	}
	#endif	
}
*/
/*
void rejectSensorEDFI(void)
{
	bool bFan1Failure = false;
	bool bFan2Failure = false;
	
	(fan1ErrorCount>=maxFanErrorCount)?(bFan1Failure=true):(bFan1Failure=false);
	(fan2ErrorCount>=maxFanErrorCount)?(bFan2Failure=true):(bFan2Failure=false);

	#ifdef HIGH_PERFORMANCE
	if (chamber1.rejectTemp<TSNS_INSANE_LOW_LIMIT && chamber2.rejectTemp<TSNS_INSANE_LOW_LIMIT)
	{
		sensor_state_trej==STATE_SNS_NORM_TREJ;
	}
	else
	{
		sensor_state_trej==STATE_SNS_FAIL3;
	}
	#endif
				
	#ifdef GENERAL_PURPOSE
	if (sensor_state_trej==STATE_RESET)
	{
		sensor_state_trej = STATE_SNS_NORM_TREJ;
		prev_sensor_state_trej = STATE_RESET;
	}
	else if (sensor_state_trej==STATE_SNS_NORM_TREJ)
	{
		if (prev_sensor_state_trej!=STATE_SNS_NORM_TREJ)
		{
			enableFanOverride = false;
			chamber1.deactivateAutoControl = false;
			//errorRegister &= ~ERROR_REJ_FAN_SPEED;
		}
		if ( (chamber1.rejectTemp<TSNS_INSANE_LOW_LIMIT) || bFan1Failure || bFan2Failure )
		{
			sensor_state_trej = STATE_SNS_FAIL3;
		}
		prev_sensor_state_trej = STATE_SNS_NORM_TREJ;
	}
	else if (sensor_state_trej==STATE_SNS_FAIL3)
	{
		if (prev_sensor_state_trej!=STATE_SNS_FAIL3)
		{
			enableFanOverride = true;
			fan1.targetDutyPercent = 100;
			fanControl(&fan1, SNS_FAIL_VSET);
			fan2.targetDutyPercent = 100;
			fanControl(&fan2, SNS_FAIL_VSET);
			chamber1.deactivateAutoControl = false;
			//errorRegister |= ERROR_REJ_FAN_SPEED;
		}
		if (bFan1Failure && bFan2Failure)
		{
			sensor_state_trej = STATE_SNS_FAIL4;
		}
		else if ( (chamber1.rejectTemp>TSNS_INSANE_LOW_LIMIT) && !bFan1Failure && !bFan2Failure )
		{
			sensor_state_trej = STATE_SNS_NORM_TREJ;
		}
		prev_sensor_state_trej = STATE_SNS_FAIL3;
	}
	else if (sensor_state_trej==STATE_SNS_FAIL4)
	{
		if (prev_sensor_state_trej!=STATE_SNS_FAIL4)
		{
			enableFanOverride = true;
			fan1.targetDutyPercent = 100;
			fanControl(&fan1, SNS_FAIL_VSET);
			fan2.targetDutyPercent = 100;
			fanControl(&fan2, SNS_FAIL_VSET);
			chamber1.deactivateAutoControl = true;
			ps1.desiredVoltage = SNS_FAIL_VSET;
			//errorRegister |= ERROR_REJ_FAN_SPEED;
		}
		if (!bFan1Failure && !bFan2Failure)
		{
			sensor_state_trej = STATE_SNS_FAIL3;
		}
		prev_sensor_state_trej = STATE_SNS_FAIL4;
	}
	else
	{
		enableFanOverride = false;
		chamber1.deactivateAutoControl = false;
		//errorRegister &= ~ERROR_REJ_FAN_SPEED;
	}
	#endif	
}
*/

void lowPowerModeEnter(void)
{
	// power down everything
	dbg("\n[sleep]\n");	
	
	port_pin_set_output_level(PIN_LED_0_PIN, HIGH);
			
	port_pin_set_output_level_EX(PIN_RDY_N, HIGH, enableManualOverride);
			
	// shut down output
	ps1.desiredVoltage = 0;					  
	setBoostEnable(&ps1, HIGH);					  // disable boost converter **
	setBoostVoltage(&ps1, BOOST_OFF_DUTY);
	ps1.boostPwmState = OFF;
	if (ps1.bulkPwmState == ON) {
		turnOffBulkPwm(&ps1);					  // set bulk to nominal
		ps1.bulkPwmState = OFF;
	}
	setOutputEnable(&ps1, LOW);					  // disable output to php
	ps1.outputStatus = OFF;
			
	system_interrupt_disable(SERCOM1_IRQn);
	system_interrupt_disable(SERCOM3_IRQn);
	system_interrupt_disable(RTC_IRQn);
	system_interrupt_disable(TC1_IRQn);
	system_interrupt_disable(TC4_IRQn);
	system_interrupt_disable(TC5_IRQn);
	system_interrupt_disable(TC7_IRQn);
	system_interrupt_disable(ADC_IRQn);
	system_interrupt_disable(SERCOM5_IRQn);
			
	/*
	REG_PM_APBAMASK = 0x00000000;
	REG_PM_APBBMASK = 0x00000000;
	REG_PM_APBCMASK = 0x00000000;
	*/
			
	system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
	system_sleep();	
}

void lowPowerModeExit(void)
{
	port_pin_set_output_level_EX(PIN_LED_0_PIN, LOW, false);
		
	dbg("\n[wake]\n");
		
	// function as remote "adc" for UI
	// reduce cpu freq??? to save power ???
		
	//system_interrupt_enable(SERCOM3_IRQn);
	system_interrupt_enable(RTC_IRQn);
	//system_interrupt_enable(TC1_IRQn);
	//system_interrupt_enable(TC4_IRQn);
	//system_interrupt_enable(TC5_IRQn);
	//system_interrupt_enable(TC7_IRQn);
	system_interrupt_enable(ADC_IRQn);
	system_interrupt_enable(SERCOM5_IRQn);
		
	readSensors();
		
	port_pin_set_output_level_EX(PIN_RDY_N, LOW, enableManualOverride);   // assert RDY pin
		
	// DEBUG !!!!!!
	//ps1.logicVoltage = 3.3;
	// DEBUG !!!!!!
		
	// 3V3 is present (normal power)
	if (ps1.logicVoltage >= ps1.standbyVoltage)
	{
/*
		// if trying to come out of sleep with 3V3 present, then
		// re-init everything or reset mcu
		system_interrupt_enable(SERCOM3_IRQn);
		//system_interrupt_enable(RTC_IRQn);
		system_interrupt_enable(TC1_IRQn);
		system_interrupt_enable(TC4_IRQn);
		system_interrupt_enable(TC5_IRQn);
		system_interrupt_enable(TC7_IRQn);
		//system_interrupt_enable(ADC_IRQn);
		system_interrupt_enable(SERCOM5_IRQn);

		// tc1 cmp1 is boost pwm out
		// tc1 cmp2 is bulk pwm out
		configure_tc1();
		configure_tc1_callbacks();

		// tc4 cap1 is fan1 tachometer capture
		configure_tc4();
		configure_tc4_callbacks();
	
		// tc5 cap1 is fan2 tachometer capture
		configure_tc5();
		configure_tc5_callbacks();

		// tc7 cmp1 is fan1 pwm out
		// tc7 cmp2 is fan2 pwm out
		configure_tc7();
		configure_tc7_callbacks();
	
		configure_extint();
		configure_extint_callbacks();
		configure_event_sys();
		system_interrupt_disable(EIC_IRQn);
		system_interrupt_enable_global();
		tc_enable(&tc4_module);
		tc_enable(&tc5_module);
		configure_i2c_slave();
		configure_i2c_slave_callbacks();
		REG_EIC_INTFLAG = 0x8000;			// clear interrupt flag
		system_interrupt_enable(EIC_IRQn);
*/
		system_reset();
/*	
		chamber1.deactivateAutoControl = false;
		ps1.desiredVoltage = 0;
		chamber1.mode_state = STATE_RESET;
*/
	}


}