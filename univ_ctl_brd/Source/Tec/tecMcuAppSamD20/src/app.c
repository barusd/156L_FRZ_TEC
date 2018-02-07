/**
 * \file
 *
 * Copyright (C) 2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
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
#include "extern.h"
#include "i2cProtocol.h"
#include "dbgCmds.h"

/************************************************************************/
/* Global Variables                                                     */
/************************************************************************/
// sensors
//float humidityPercentage = 100.0;

// program control
bool enablePsDbgMsgs = false;
bool enableManualOverride = false;
bool enableTempOverride = false;
bool phpDetected = false;
bool inhibitOutput = true;
bool enableFanOverride = false;
bool enableDebugCmds = true;
bool deactivateAutoHeaterControl = false;
bool highUsageModeFlag = false;
bool enableSensorEDFI = false; //true;
uint8_t fanbank = 1;
bool enableSideAPcp = true;
bool enableSideBPcp = true;
bool enableSideAFans = true;
bool enableSideBFans = true;
bool bFanASyncFlag = OFF;
bool bFanBSyncFlag = OFF;

// timing
uint32_t task1Time = 0;
uint32_t task2Time = 0;
uint32_t task3Time = 0;
// task4 used for defrost
// task5 used for defrost	
uint32_t task6Time = 0;	
uint32_t task7Time = 0;	
// task8 used for php self-check
// task9 used for php self-check
// task10 used for php self-check
uint32_t task11Time = 0;
uint32_t task12Time = 0;

uint32_t task1LastTime = 0;	
uint32_t task2LastTime = 0;
uint32_t task3LastTime = 0;
// task4 used for defrost
// task5 used for defrost
uint32_t task6LastTime = 0;
uint32_t task7LastTime = 0;
// task8 used for php self-check
// task9 used for php self-check
// task10 used for php self-check
uint32_t task11LastTime = 0;
uint32_t task12LastTime = 0;

// temperature
float maxAllowedChamberTemp = DEFAULT_MAX_ALLOWED_TCHA;
float minAllowedChamberTemp = DEFAULT_MIN_ALLOWED_TCHA;
float pidInputTemperature = 0;
float controlTemperature = 25;
uint16_t chamberThermistorBeta = BCOEFFICIENT_TCHAMBER;
uint16_t rejectThermistorBeta = BCOEFFICIENT_TGLYCOL;
float condensateTemp = 0;
float chamberTempUniformityTarget = UNIFORMITY_TARGET;

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

int main(void)
{
	initMain();

	while (true) {

#ifdef ENABLE_SLEEP_WAKE
		if (port_pin_get_input_level(PIN_SLP)==HIGH)
		{
			delay_us(1);
			if (port_pin_get_input_level(PIN_SLP)==HIGH)
			{
				// indicate "not ready" to UI
				//port_pin_set_output_level_EX(PIN_RDY_N, HIGH, enableManualOverride);  // de-assert RDY pin
				readSensors();
				// no 3V3 present (battery power)
				if (ps1.logicVoltage < ps1.standbyVoltage)
				{
					lowPowerModeEnter();
				}
				// 3V3 is present (normal power)
				else
				{
					//dbg("\n[ignoring sleep request]\n");
					// don't allow sleep with 3V3 present
					//port_pin_set_output_level_EX(PIN_RDY_N, LOW, enableManualOverride);  // de-assert RDY pin
					// do nothing
				}
			}
		}
#endif
					
		// eeprom write handler
		if (writeToEepromFlag == true)
		{
			eepromWriteParams();		
		}
				
		// i2c Function handler
		if (i2cFunctionReg != 0x00)
		{	
			i2cCommandHandler();
			i2cFunctionReg = 0x00;
		}			
		
		// check usart rx buffer
		usart_read_buffer_job(&cdc_uart_module,	(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);
			
		// handle any commands
		if (cmdFlag==true && enableDebugCmds==true) {
			usartCommandHandler();
			cmdFlag = false;
		}							
		
		// door open/close events
		if (!enableFanOverride)
		{
			handleDoorEvents();
		}	
		
		// EVENT SLOT1 ////////////////////////////////////////////////
		if (task1Time>TASK1_SEC) {
			task1LastTime = presentTimeInSec();
	
			// refresh sensor data
			readSensors();
					
			//calculateChamberOperatingPoint(&ps1, &chamber1);

			// bound the setpoint
			if (chamber1.setpoint > maxAllowedChamberTemp)
			{
				chamber1.setpoint = maxAllowedChamberTemp;
				dbg_info("\nsetpoint limited to %f", maxAllowedChamberTemp);
			}
			else if (chamber1.setpoint < minAllowedChamberTemp)
			{
				chamber1.setpoint = minAllowedChamberTemp;
				dbg_info("\nsetpoint limited to %f", minAllowedChamberTemp);
			}
			
			// heater1 ///////////////////////////////////
			// condensate tray heater
			//////////////////////////////////////////////
			if (deactivateAutoHeaterControl==false)
			{
				setHeaterState(&condHeater, condHeaterStateMachine());
			}
			
			// heater2 ///////////////////////////////////
			// door heater
			//////////////////////////////////////////////
			// set the door heater based on setpoint
			if (deactivateAutoHeaterControl==false)
			{
				setHeaterState(&doorHeater, lookupDoorHeaterDutyVal(chamber1.setpoint));
			}
			
			//////////////////////////////////////////////
			// chamber sensor EDFI 
			//////////////////////////////////////////////
			if (enableSensorEDFI==true)
			{
				//chamberSensorEDFI();
				
			}
			else
			{
				controlTemperature = chamber2.chamberTemp;
			}				

			pidLoop1.Input = controlTemperature;
			pidLoop1.Setpoint = chamber1.setpoint;
			
			pidFan4.Input = chamberTempDifference;
			pidFan4.Setpoint = chamberTempUniformityTarget;
			
			//////////////////////////////////////////////
			// reject sensor EDFI
			//////////////////////////////////////////////	
			if (enableSensorEDFI==true)
			{			
				//rejectSensorEDFI();
			}
			
		}
		
		// EVENT SLOT2 ///////////////////////////////////////////////	
		if (task2Time>TASK2_SEC) {
			task2LastTime = presentTimeInSec();

			// determine operating mode
			if (chamber1.deactivateAutoControl==false)
			{
				modeStateMachine(&pidLoop1, &ps1, &chamber1);	// 'ps1' is control voltage
			}
			else
			{
				if (defrostActive == true)
				{
					chamber1.mode_state = STATE_DEFROST;
				}
				else
				{
					chamber1.mode_state = STATE_MANUAL;
				}
			}
			
			if (phpDetected==true)
			{				
				psHwController();
			}
		}
		
		// EVENT SLOT12 ///////////////////////////////////////////////
		if (task12Time>TASK12_SEC) {
			task12LastTime = presentTimeInSec();			

			if (fanReadRpm_emc2305(&fan1) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan2) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan3) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan4) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan5) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan6) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan7) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			if (fanReadRpm_emc2305(&fan8) != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
			
			if (fanReadStallStatus_emc2305() != STATUS_OK) { errorRegister |= ERROR_FAN_CONTROLLER; dbg("\n...fan controller read failure\n"); }
							
			//////////////////////////////////////////////
			// sideA Fans
			//////////////////////////////////////////////

			if (fan1.state==ON && fan2.state==ON && fan5.state==ON && fan6.state==ON) { bFanASyncFlag = ON; }
			else { bFanASyncFlag = OFF; }
				
			if(bFanASyncFlag == ON) {
				fanControl(&fan1, ps1.desiredVoltage);	// chamberA
				fanControl(&fan2, ps1.desiredVoltage);	// reject1A
				fanControl(&fan5, ps1.desiredVoltage);	// reject2A
				fanControl(&fan6, ps1.desiredVoltage);	// reject3A
			} else {
				if (fanbank==1) { fanControl(&fan1, ps1.desiredVoltage); }	// chamberA
				if (fanbank==3) { fanControl(&fan2, ps1.desiredVoltage); }	// reject1A
				if (fanbank==5) { fanControl(&fan5, ps1.desiredVoltage); }	// reject2A
				if (fanbank==7) { fanControl(&fan6, ps1.desiredVoltage); }	// reject3A				
			}

			//////////////////////////////////////////////
			// sideB Fans
			//////////////////////////////////////////////
						
			if (fan4.state==ON && fan3.state==ON && fan8.state==ON && fan7.state==ON) { bFanBSyncFlag = ON; }
			else { bFanBSyncFlag = OFF; }
			
			if(bFanBSyncFlag == ON) {			
				fanControl(&fan4, ps1.desiredVoltage);	// chamberB
				fanControl(&fan3, ps1.desiredVoltage);	// reject1B
				fanControl(&fan8, ps1.desiredVoltage);	// reject2B
				fanControl(&fan7, ps1.desiredVoltage);	// reject3B	
			} else {
				if (fanbank==2) { fanControl(&fan4, ps1.desiredVoltage); }	// chamberB
				if (fanbank==4) { fanControl(&fan3, ps1.desiredVoltage); }	// reject1B
				if (fanbank==6) { fanControl(&fan8, ps1.desiredVoltage); }	// reject2B
				if (fanbank==8) { fanControl(&fan7, ps1.desiredVoltage); }	// reject3B								
			}	

			fanbank++;
			if (fanbank>8)	{ fanbank = 1; }
			
		}
		
		// EVENT SLOT3 ///////////////////////////////////////////////
		if (task3Time>TASK3_SEC) {
			task3LastTime = presentTimeInSec();

			// print debug msgs
			if (enablePsDbgMsgs == true) 
			{														
				printDbgMsgs();
			}
		}
		
		// EVENT SLOT4 AND 5 USED FOR DEFROST ////////////////////////
		defrostStateMachine();
		
		// EVENT SLOT6 ///////////////////////////////////////////////
		if (task6Time>TASK6_SEC) {
			task6LastTime = presentTimeInSec();
			
			if (mfgTestModeFlag1==false)
			{
				checkForSensorFaults();
				checkForFanFaults();	
			}
		}		
		
		// EVENT SLOT7 ///////////////////////////////////////////////
		if (task7Time>TASK7_SEC) {
			//task7LastTime = presentTimeInSec();
			
			inhibitOutput = false;	
		}			

		// EVENT SLOT8, 9, 10 USED FOR PHP SELF-CHECK ////////////////
		if (inhibitOutput==false && mfgTestModeFlag1==false && defrostActive==false)
		{
			//phpSelfCheck();
		}
		
		// EVENT SLOT11 //////////////////////////////////////////////
		if (task11Time>TASK11_SEC) {
			task11LastTime = presentTimeInSec();

			doorOpenLifetimeCnt += doorOpenCount_1Hr;
			eepromDataInfo.pAddr = &doorOpenLifetimeCnt;
			eepromDataInfo.size = sizeof(doorOpenLifetimeCnt);
			eepromDataInfo.nonvolatileFlag = true;
			writeToEepromFlag = true;
						
			if (doorOpenCount_1Hr > doorOpenCount1HrLimit)
			{
				highUsageModeFlag = true;
			}

			doorOpenCount_1Hr = 0;
		}		
									
		// UPDATE TIME ///////////////////////////////////////////////	
		task1Time = updateTimeInSec(task1LastTime);
		task2Time = updateTimeInSec(task2LastTime);
		task3Time = updateTimeInSec(task3LastTime);
		// task4 used for defrost
		// task5 used for defrost
		task6Time = updateTimeInSec(task6LastTime);
		task7Time = updateTimeInSec(task7LastTime);
		// task8 used for php self-check
		// task9 used for php self-check
		// task10 used for php self-check
		task11Time = updateTimeInSec(task11LastTime);
		task12Time = updateTimeInSec(task12LastTime);	
		
	} // end while;
} // end main;


