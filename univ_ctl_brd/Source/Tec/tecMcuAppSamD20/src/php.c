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
uint16_t maxPhpErrorCount = MAX_PHP_ERROR_COUNT;
uint16_t php1TotalErrorCount = 0;
uint16_t php2TotalErrorCount = 0;
float phpNominalResistance = PHP125_NOMINAL_RESISTANCE;
float phpResTolPercentage = PHP125_RES_TOL_PERCENTAGE;
uint32_t task8Time = 0;
uint32_t task9TriggerTime = 0;
uint32_t task9Time = 0;
uint32_t task10TriggerTime = 0;
uint32_t task10Time = 0;
uint32_t task8LastTime = 0;
uint32_t task9LastTime = 0;
uint32_t task10LastTime = 0;
float php1CurrentLwrLim = DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT;
float php2CurrentLwrLim = DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT;
float php1CurrentLwrLimSetting = DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT;
float php2CurrentLwrLimSetting = DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT;
float php1CurrentDiffLim = DEFAULT_PHP1_DETECT_DIFF_CURRENT_LIMIT;
float php2CurrentDiffLim = DEFAULT_PHP2_DETECT_DIFF_CURRENT_LIMIT;
bool enablePhpCurrentToleranceCheck = false;
float phpChk_lastI1lim = 0;
float phpChk_lastI1 = 0;
float phpChk_lastV1 = 0;
float phpChk_lastI2lim = 0;
float phpChk_lastI2 = 0;
float phpChk_lastV2 = 0;
uint16_t phpChkTimeIntervalInSecs = TASK8_SEC;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
void initPhpSelfCheck()
{
	task8LastTime = presentTimeInSec();
	task9LastTime = presentTimeInSec();
	task10LastTime = presentTimeInSec();
	task8Time = 0;
	task9Time = 0;
	task10Time = 0;	
	php1CurrentLwrLim = DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT;
	php2CurrentLwrLim = DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT;
	php1CurrentLwrLimSetting = DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT;
	php2CurrentLwrLimSetting = DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT;
	php1CurrentDiffLim = DEFAULT_PHP1_DETECT_DIFF_CURRENT_LIMIT;
	php2CurrentDiffLim = DEFAULT_PHP2_DETECT_DIFF_CURRENT_LIMIT;	
	enablePhpCurrentToleranceCheck = false;	
	phpChkTimeIntervalInSecs = TASK8_SEC;
	task9TriggerTime = phpChkTimeIntervalInSecs + TASK9_SEC;
	task10TriggerTime = phpChkTimeIntervalInSecs + TASK10_SEC;
}

//void phpSelfCheck()
//{
	//// EVENT SLOT8 ///////////////////////////////////////////////
	//if (task8Time > phpChkTimeIntervalInSecs) {
		//task8LastTime = presentTimeInSec();
		////task9TriggerTime = task8Time + TASK9_SEC;
		////task10TriggerTime = task8Time + TASK10_SEC;
			//
			//checkForPhpFaults();
	//}
//
	//// EVENT SLOT9 ///////////////////////////////////////////////
	//if (task9Time > task9TriggerTime) {
		//task9LastTime = task8LastTime;
			//
			//checkForPhpFaults();
	//}
//
	//// EVENT SLOT10 //////////////////////////////////////////////
	//if (task10Time > task10TriggerTime) {
		//task10LastTime = task8LastTime;
//
			//checkForPhpFaults();
	//}
		//
	//// UPDATE TIME ///////////////////////////////////////////////			
	//task8Time = updateTimeInSec(task8LastTime);
	//task9Time = updateTimeInSec(task9LastTime);
	//task10Time = updateTimeInSec(task10LastTime);		
//}
//
//void checkForPhpFaults()
//{
	//if (maxPhpErrorCount > 0)
	//{
		//phpCheckCurrentOutOfBounds();
	//
		//if (php1TotalErrorCount >= maxPhpErrorCount)
		//{
			//errorRegister |= ERROR_PHP1;
		//}
		//else
		//{
			//errorRegister &= ~ERROR_PHP1;
		//}
		//
		//if (php2TotalErrorCount >= maxPhpErrorCount)
		//{
			//errorRegister |= ERROR_PHP2;
		//}
		//else
		//{
			//errorRegister &= ~ERROR_PHP2;
		//}
	//}
//}

//
//void phpCheckCurrentOutOfBounds()
//{
	//uint32_t boostDutyVal = BOOST_OFF_DUTY;
	//bool php1Fail = false;
	//bool php2Fail = false;
	//float vsetTemp = MAX_OUTPUT_VOLTAGE_DEFAULT;
//
	//if (enablePsDbgMsgs == true)
	//{
		//rtc_calendar_get_time(&time_report.time);
		//
		//dbg_info("\n[php self-check %2dy%2dd:%2dh:%2dm:%2ds ",
			//time_report.time.year,
			//time_report.time.day,
			//time_report.time.hour,
			//time_report.time.minute,
			//time_report.time.second);
	//}
		//
	//psHwController();
		//
	//setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, ON);	  
	//setOutputSwitch(PIN_ENBL_OUTPUT3, GPIO_MODE, ON); 
	//
	//delay_ms(3000);
//
	//readSensors();
	//
	//if (enablePhpCurrentToleranceCheck==true)	
	//{
		//php1CurrentLwrLim = ps1.phpVoltage / ( phpNominalResistance + (phpNominalResistance * (phpResTolPercentage/100)) );
		//php2CurrentLwrLim = ps2.phpVoltage / ( phpNominalResistance + (phpNominalResistance * (phpResTolPercentage/100)) );		
	//}
	//else
	//{
		//php1CurrentLwrLim = php1CurrentLwrLimSetting; //DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT;
		//php2CurrentLwrLim = php2CurrentLwrLimSetting; //DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT;
	//}
	//
	//if (enablePsDbgMsgs == true)
	//{
		//dbg_info(" v1a=%4.2f i1a=%4.2f", ps1.phpVoltage, ps1.outputCurrent);
	//}
	//if (ps1.outputCurrent < php1CurrentLwrLim)
	//{
		//php1Fail = true;	
	//}
	//#ifdef HIGH_PERFORMANCE
	//if (enablePsDbgMsgs == true)
	//{
		//dbg_info(" v2a=%4.2f i2a=%4.2f", ps2.phpVoltage, ps2.outputCurrent);
	//}
	//if (ps2.outputCurrent < php2CurrentLwrLim)
	//{
		//php2Fail = true;
	//}
	//#endif
	//
	//if (php1Fail==true || php2Fail==true)
	//{
		////if (ps1.desiredVoltage <= BULK_VOLTAGE_NOM)
		////{
			//vsetTemp = MAX_OUTPUT_VOLTAGE_DEFAULT-15;
			//boostDutyVal = lookupBoostPwmVal(vsetTemp);
			//setBoostVoltage(&ps1, boostDutyVal);
			//delay_ms(10);                                
			//setBoostEnable(&ps1, LOW);					
			//delay_ms(10);
			//ps1.boostPwmState = ON;
			//delay_ms(3000);
			//vsetTemp = MAX_OUTPUT_VOLTAGE_DEFAULT;
			//boostDutyVal = lookupBoostPwmVal(vsetTemp);
			//setBoostVoltage(&ps1, boostDutyVal);			
		////}
		///*
		//else if (ps1.desiredVoltage > BULK_VOLTAGE_NOM)
		//{
			//vsetTemp = BULK_VOLTAGE_NOM;
			//setBoostEnable(&ps1, HIGH);					  
			//setBoostVoltage(&ps1, BOOST_OFF_DUTY);
			//ps1.boostPwmState = OFF;
		//}
		//*/
		//
		//delay_ms(3000);
		//
		//readSensors();
//
		//if (enablePhpCurrentToleranceCheck==true)
		//{			
			//php1CurrentLwrLim = ps1.phpVoltage / ( phpNominalResistance + (phpNominalResistance * (phpResTolPercentage/100)) );
			//php2CurrentLwrLim = ps2.phpVoltage / ( phpNominalResistance + (phpNominalResistance * (phpResTolPercentage/100)) );
		//}
		//else
		//{
			//php1CurrentLwrLim = php1CurrentLwrLimSetting; //DEFAULT_PHP1_DETECT_LOWER_CURRENT_LIMIT;
			//php2CurrentLwrLim = php2CurrentLwrLimSetting; //DEFAULT_PHP2_DETECT_LOWER_CURRENT_LIMIT;
		//}
		//
		//if (enablePsDbgMsgs == true)
		//{
			//dbg_info(" v1b=%4.2f i1b=%4.2f", ps1.phpVoltage, ps1.outputCurrent);		
		//}
		//if (ps1.outputCurrent < php1CurrentLwrLim)
		//{
			//php1Fail = true;
		//}
		//else
		//{
			//php1Fail = false;
		//}
		//#ifdef HIGH_PERFORMANCE
		//if (enablePsDbgMsgs == true)
		//{
			//dbg_info(" v2b=%4.2f i2b=%4.2f", ps2.phpVoltage, ps2.outputCurrent);
		//}
		//if (ps2.outputCurrent < php2CurrentLwrLim)
		//{
			//php2Fail = true;
		//}	
		//else
		//{
			//php2Fail = false;
		//}	
		//#endif
		//
		//if (php1Fail==true || php2Fail==true)
		//{
			//// store previous current readings
			//ps1.previousOutputCurrent = ps1.outputCurrent;
			//ps2.previousOutputCurrent = ps2.outputCurrent;
//
			//// turn off output
			//setOutputSwitch(PIN_ENBL_OUTPUT1, GPIO_MODE, OFF);
			//setOutputSwitch(PIN_ENBL_OUTPUT3, GPIO_MODE, OFF);			
					//
			//delay_ms(3000);
			//
			//readSensors();
//
			//if (enablePsDbgMsgs == true)
			//{
				//dbg_info(" v1c=%4.2f i1c=%4.2f", ps1.phpVoltage, ps1.outputCurrent);
			//}
			//if ( abs(ps1.previousOutputCurrent-ps1.outputCurrent) < php1CurrentDiffLim)
			//{
				//php1Fail = true;
			//}
			//else
			//{
				//php1Fail = false;
			//}
			//#ifdef HIGH_PERFORMANCE
			//if (enablePsDbgMsgs == true)
			//{
				//dbg_info(" v2c=%4.2f i2c=%4.2f", ps2.phpVoltage, ps2.outputCurrent);
			//}
			//if ( abs(ps2.previousOutputCurrent-ps2.outputCurrent) < php2CurrentDiffLim)
			//{
				//php2Fail = true;
			//}
			//else
			//{
				//php2Fail = false;
			//}
			//#endif
		//}
	//
	//}
	//
	//if (php1Fail == true)
	//{
		//php1TotalErrorCount++;
	//}
	//else
	//{
		//php1TotalErrorCount = 0;
	//}
	//
	//#ifdef HIGH_PERFORMANCE
	//if (php2Fail == true)
	//{
		//php2TotalErrorCount++;
	//}
	//else
	//{
		//php2TotalErrorCount = 0;
	//}	
	//#endif
//
	//if (enablePsDbgMsgs == true) 
	//{
		////dbg_info("i1lim=%2.1f i1=%2.1f v1=%2.1f i2lim=%2.1f i2=%2.1f v2=%2.1f]",
			////php1CurrentLwrLim,
			////ps1.outputCurrent,
			////ps1.phpVoltage,
			////php2CurrentLwrLim,
			////ps2.outputCurrent,
			////ps2.phpVoltage);
		//dbg("]");
	//}
		//
	//phpChk_lastI1lim = php1CurrentLwrLim;
	//phpChk_lastI1	 = ps1.outputCurrent;
	//phpChk_lastV1	 = ps1.phpVoltage;
	//phpChk_lastI2lim = php2CurrentLwrLim;
	//phpChk_lastI2	 = ps2.outputCurrent;
	//phpChk_lastV2	 = ps2.phpVoltage;		
			//
	//psHwController();	
//}

/*
void calculateChamberOperatingPoint(struct ps_vars *ps_inst, 
									struct chamber_vars *ch_inst)
{
	ch_inst->deltaT = ambientTemp - ch_inst->chamberTemp;
	ch_inst->Qc = ch_inst->heatLeak * ch_inst->deltaT;
	ps_inst->outputPower = ps_inst->phpVoltage * ps_inst->outputCurrent;
	ch_inst->COP = ch_inst->Qc / ps_inst->outputPower;
	if ((int)(ps_inst->inputPower) == 0)
	{
		ps_inst->efficiency = 0.0f;
	}
	else
	{
		//ps_inst->efficiency = (ps_inst->outputPower / ps_inst->inputPower) * 100;
		ps_inst->efficiency = ((ps1.outputPower + ps2.outputPower) / ps_inst->inputPower) * 100;
	}
}

void calculatePhpOperatingPoint(struct php_vars *php_inst)
{
	php_inst->deltaT = php_inst->Thot - php_inst->Tcold;
	
	php_inst->Qc  = S_MODULE * php_inst->Tcold * php_inst->I
					- 0.5 * php_inst->I * php_inst->I * R_MODULE
					- K_LEGS * php_inst->deltaT;
					 
	php_inst->W   = php_inst->I * php_inst->I * R_MODULE
					+ S_MODULE * php_inst->deltaT * php_inst->I;
					 
	php_inst->V   = php_inst->I * R_MODULE + S_MODULE * php_inst->deltaT;
	
	php_inst->COP = php_inst->Qc / php_inst->W;
}
*/

/*
void detectPHP()
{
	float volts = LOW_POWER_LIMIT_VOLTAGE;
		
	dbg("\ndetecting PHP...");
	
	// set to bulk nominal and enable output
	setBoostEnable(&ps1, HIGH);					   // disable boost controller **
	setBoostVoltage(&ps1, BOOST_OFF_DUTY);
	ps1.boostPwmState = OFF;
	if (ps1.bulkPwmState == ON) {
		turnOffBulkPwm(&ps1);					   // set bulk to nominal
		ps1.bulkPwmState = OFF;
	}
	if (ps1.outputStatus == OFF) {
		outputWalkin(&ps1);
		setOutputEnable(&ps1, HIGH);				   // enable output to php
		ps1.outputStatus = ON;
	}
			
	// read current
	delay_ms(2000);
	readSensors();
	
	
	// [DEGUG] !!!!!!!!
	//ps1.logicVoltage = 3.3;
	// [DEGUG] !!!!!!!!
		
		
	// if PHP present, then perform PHP dcr detect
	// SHOULD FACTOR DELTA_T INTO THIS CONDITIONAL !!!
	if ( (ps1.outputCurrent>=MIN_CURRENT_AT_VPHP_NOM) &&
		 (ps1.logicVoltage > ps1.standbyVoltage) )
	{
		phpDetected = true;
		dbg("\nPHP detected.");
		dbg("\ndetecting maxOutputVoltage...");
		delay_ms(1000);

		for (volts=LOW_POWER_LIMIT_VOLTAGE; volts<=MAX_OUTPUT_VOLTAGE_DEFAULT; volts+=0.5)
		{
			if (ps1.bulkPwmState == ON) {
				turnOffBulkPwm(&ps1);				// set bulk to nominal
				ps1.bulkPwmState = OFF;
			}
			// R1=68000, R2=1000000 , R3=43200
			// VREF=0.7, RES=16bit, 27.0>VBOOST>14.0
			uint32_t boostDutyVal = 148189.7791	-1596.389744 * volts;
			

			//boostDutyVal -= 1000;						// apply offset
			setBoostVoltage(&ps1, boostDutyVal);
			delay_ms(10);                               // wait  for duty to settle
			setBoostEnable(&ps1, LOW);					// enable boost converter **
			delay_ms(10);
			ps1.boostPwmState = ON;
			if (ps1.outputStatus == OFF) {
				outputWalkin(&ps1);
				setOutputEnable(&ps1, HIGH);			// enable output to php
				ps1.outputStatus = ON;
			}
			
			delay_ms(1000);  //wait
			
			readSensors();
			
			dbg_info("\n%4.2fV, %4.2fA, %4.2fW", ps1.phpVoltage, ps1.outputCurrent, ps1.outputPower);
		
			if ( (ps1.outputPower>POWER_CAP) || (ps1.outputCurrent>MAX_OUTPUT_CURRENT_DEFAULT) )
			{
				maxOutputVoltage = volts - 0.5;
				break;
			}
			else
			{
				maxOutputVoltage = volts;
			}				
		}

		if (maxOutputVoltage>MAX_OUTPUT_VOLTAGE_DEFAULT)
		{
			maxOutputVoltage = MAX_OUTPUT_VOLTAGE_DEFAULT;
		}
		if (maxOutputVoltage<LOW_POWER_LIMIT_VOLTAGE)
		{
			maxOutputVoltage = LOW_POWER_LIMIT_VOLTAGE;
		}
	
		dbg_info("\nmaxOutputVoltage is %4.2f", maxOutputVoltage);
		dbg("\nPHP detect complete.");		
	}
	
	// if PHP not present, then indicate error and do not assert flag
	else
	{
		phpDetected = false;
		errorRegister |= ERROR_PHP_DETECT_FAIL;
		dbg_info("\nPHP current is %4.2fA.  Logic Voltage is %4.2fV.  PHP not detected!",
				 ps1.outputCurrent, ps1.logicVoltage);
	}
	
	// shut down output
	ps1.desiredVoltage = 0;					  // don't allow negative voltages
	setBoostEnable(&ps1, HIGH);					  // disable boost converter **
	setBoostVoltage(&ps1, BOOST_OFF_DUTY);
	ps1.boostPwmState = OFF;
	if (ps1.bulkPwmState == ON) {
		turnOffBulkPwm(&ps1);					  // set bulk to nominal
		ps1.bulkPwmState = OFF;
	}
	setOutputEnable(&ps1, LOW);					  // disable output to php
	ps1.outputStatus = OFF;	
	
	return;
}
*/