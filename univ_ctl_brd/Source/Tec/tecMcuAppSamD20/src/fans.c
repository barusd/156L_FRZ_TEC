/**
 * \file
 *
 * \brief SAM D20 Analog to Digital Converter (ADC) Unit test
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
#include "i2cProtocol.h"
#include "dbgCmds.h"
#include "extern.h"

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
uint16_t fan1ErrorCount = 0;
uint16_t fan2ErrorCount = 0;
uint16_t fan4ErrorCount = 0;
uint16_t fan1LowSpdErrCnt = 0;
uint16_t fan2LowSpdErrCnt = 0;
uint16_t fan4LowSpdErrCnt = 0;
//float fanRpmErrorPercentage = DEFAULT_FAN_RPM_ERROR_PERCENTAGE;
uint16_t maxFanErrorCount = MAX_FAN_ERROR_COUNT;
float fanSpdTolerance = DEFAULT_FAN_SPD_TOLERANCE;

/************************************************************************/
/* Functions                                                            */
/************************************************************************/

uint8_t i2cWrite_emc2305(uint8_t numBytes, uint8_t fan_chip)
{
	uint8_t error = 0;

	// build packet to send
	uint16_t timeout = 0;
	struct i2c_packet packet = {
		.address = F_SLAVE_ADDRESS,
		.data_length = numBytes,
		.data = i2c_txdat,
	};
	
	if (fan_chip == FAN_CHIP2)
	{
		packet.address = F_SLAVE_ADDRESS_2;
	}
	
	// send byte to set internal register address
	while (i2c_master_write_packet_wait(&i2c_master_instance, &packet) != STATUS_OK) {
		if (timeout++ == F_I2C_COMM_TIMEOUT) {
			error = 1;
			break;
		}
	}
	
	// exit
	return error;
}

uint8_t i2cWriteRead_emc2305(uint8_t numBytes, uint8_t fan_chip)
{
	uint8_t error = 0;
	
	// build packet to send
	uint16_t timeout = 0;
	struct i2c_packet packet = {
		.address = F_SLAVE_ADDRESS,
		.data_length = numBytes,
		.data = i2c_txdat,
	};

	if (fan_chip == FAN_CHIP2)
	{
		packet.address = F_SLAVE_ADDRESS_2;
	}
	
	// send byte to set internal register address
	while (i2c_master_write_packet_wait(&i2c_master_instance, &packet) != STATUS_OK) {
		if (timeout++ == F_I2C_COMM_TIMEOUT) {
			error = 1;
			break;
		}
	}

	// read register if no prior write error
	if (!error)
	{
		timeout=0;
		packet.data = i2c_rxdat;
		while (i2c_master_read_packet_wait(&i2c_master_instance, &packet) != STATUS_OK) {
			if (timeout++ == F_I2C_COMM_TIMEOUT) {
				break;
			}
		}
	}
	
	// *** ADD CHECK OF DATA AGAINST WHAT WAS SENT !!!!! *** //
	
	// exit
	return error;
}

uint16_t fanEmc2305Init(uint8_t fan_chip)
{
	if (fan_chip == FAN_CHIP1)
	{		
		// 'wake-up' emc2305 fan controller by writing twice
		i2c_txdat[0] = 0x00;
		i2cWrite_emc2305(1, FAN_CHIP1);
		if (i2cWrite_emc2305(1, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// read fan configs
		if (fanReadConfig_emc2305(&fan1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		if (fanReadConfig_emc2305(&fan2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		if (fanReadConfig_emc2305(&fan3) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		if (fanReadConfig_emc2305(&fan4) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// invert pwm output polarity
		i2c_txdat[0] = 0x2a;
		i2c_txdat[1] = 0x09;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK)  { return ERROR_FAN_CONTROLLER; }
	
		// push-pull pwm outputs
		i2c_txdat[0] = 0x2b;
		i2c_txdat[1] = 0x0f;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
	
		//////////////////////////////////
		
		// fast pwm freq of ps fan1
		i2c_txdat[0] = 0x31;
		i2c_txdat[1] = 0x08; // 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// slow pwm freq of ps fan2
		i2c_txdat[0] = 0x41;
		i2c_txdat[1] = 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// slow pwm freq of ps fan3
		i2c_txdat[0] = 0x51;
		i2c_txdat[1] = 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		
		// fast pwm freq of ps fan4
		i2c_txdat[0] = 0x61;
		i2c_txdat[1] = 0x08; // 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }			
						
		//////////////////////////////////
						
		// set min speed of ps fan1 to 500rpm
		i2c_txdat[0] = 0x32;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// set min speed of ps fan2 to 500rpm
		i2c_txdat[0] = 0x42;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// set min speed of ps fan3 to 500rpm
		i2c_txdat[0] = 0x52;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// set min speed of ps fan4 to 500rpm
		i2c_txdat[0] = 0x62;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		
		/* [djb] removed for pepsi poc							
		// set min speed of chamber fan4 to 500rpm and 4poles
		i2c_txdat[0] = 0x62;
		i2c_txdat[1] = 0x1b;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }	
		*/
		
		// enable ramp rate control of fan1, 2, 3, and 4
		i2c_txdat[0] = 0x33;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		i2c_txdat[0] = 0x43;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		i2c_txdat[0] = 0x53;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }			
		i2c_txdat[0] = 0x63;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP1) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		
	}
	else
	{
		// 'wake-up' emc2305 fan controller by writing twice
		i2c_txdat[0] = 0x00;
		i2cWrite_emc2305(1, FAN_CHIP2);
		if (i2cWrite_emc2305(1, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// read fan configs
		if (fanReadConfig_emc2305(&fan5) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		if (fanReadConfig_emc2305(&fan6) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		if (fanReadConfig_emc2305(&fan7) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		if (fanReadConfig_emc2305(&fan8) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// invert pwm output polarity
		i2c_txdat[0] = 0x2a;
		i2c_txdat[1] = 0x00;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK)  { return ERROR_FAN_CONTROLLER; }
	
		// push-pull pwm outputs
		i2c_txdat[0] = 0x2b;
		i2c_txdat[1] = 0x0f;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
	
		//////////////////////////////////
		
		// slow pwm freq of ps fan1
		i2c_txdat[0] = 0x31;
		i2c_txdat[1] = 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// slow pwm freq of ps fan2
		i2c_txdat[0] = 0x41;
		i2c_txdat[1] = 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// slow pwm freq of ps fan3
		i2c_txdat[0] = 0x51;
		i2c_txdat[1] = 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		
		// slow pwm freq of ps fan4
		i2c_txdat[0] = 0x61;
		i2c_txdat[1] = 0x80;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }			
						
		//////////////////////////////////
						
		// set min speed of ps fan1 to 500rpm
		i2c_txdat[0] = 0x32;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// set min speed of ps fan2 to 500rpm
		i2c_txdat[0] = 0x42;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// set min speed of ps fan3 to 500rpm
		i2c_txdat[0] = 0x52;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }

		// set min speed of ps fan4 to 500rpm
		i2c_txdat[0] = 0x62;
		i2c_txdat[1] = 0x0b;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		
		/* [djb] removed for pepsi poc							
		// set min speed of chamber fan4 to 500rpm and 4poles
		i2c_txdat[0] = 0x62;
		i2c_txdat[1] = 0x1b;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }	
		*/
		
		// enable ramp rate control of fan1, 2, 3, and 4
		i2c_txdat[0] = 0x33;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		i2c_txdat[0] = 0x43;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		i2c_txdat[0] = 0x53;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
		i2c_txdat[0] = 0x63;
		i2c_txdat[1] = 0x68;
		if (i2cWrite_emc2305(2, FAN_CHIP2) != STATUS_OK) { return ERROR_FAN_CONTROLLER; }
						
	}
	return ERROR_NONE;
}

uint8_t fanSetSpeed_emc2305(struct fan_vars *fan_inst, uint8_t speedPercent)
{
	uint8_t numBytes = 2;
	uint8_t fan_chip = FAN_CHIP1;
	
	if (fan_inst == &fan1)
	{
		i2c_txdat[0] = 0x30;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan2)
	{
		i2c_txdat[0] = 0x40;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan3)
	{
		i2c_txdat[0] = 0x50;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan4)
	{
		i2c_txdat[0] = 0x60;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan5)
	{
		i2c_txdat[0] = 0x30;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan6)
	{
		i2c_txdat[0] = 0x40;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan7)
	{
		i2c_txdat[0] = 0x50;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan8)
	{
		i2c_txdat[0] = 0x60;
		fan_chip = FAN_CHIP2;
	}
	
	i2c_txdat[1] = (speedPercent * 0xff) / 100;
	
	// ** ADD bounds CHECKING HERE ** //

	return i2cWrite_emc2305(numBytes, fan_chip);
}

void fanStart_emc2305(struct fan_vars *fan_inst)
{
	if (fan_inst == &fan1)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN1, HIGH, enableManualOverride);
	}
	else if (fan_inst == &fan2)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN2, HIGH, enableManualOverride);
	}
	else if (fan_inst == &fan3)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN3, HIGH, enableManualOverride);
	}
	else if (fan_inst == &fan4)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN4, HIGH, enableManualOverride);
	}		
	else if (fan_inst == &fan5)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN5, HIGH, enableManualOverride);
	}
	else if (fan_inst == &fan6)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN6, HIGH, enableManualOverride);
	}
	else if (fan_inst == &fan7)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN7, HIGH, enableManualOverride);
	}
	else if (fan_inst == &fan8)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN8, HIGH, enableManualOverride);
	}
	
	fanSetSpeed_emc2305(fan_inst, fan_inst->targetDutyPercent);
	//delay_ms(1000);
	fan_inst->state = ON;
}

void fanStop_emc2305(struct fan_vars *fan_inst)
{
	if (fan_inst == &fan1)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN1, LOW, enableManualOverride);
	}
	else if (fan_inst == &fan2)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN2, LOW, enableManualOverride);
	}
	else if (fan_inst == &fan3)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN3, LOW, enableManualOverride);
	}
	else if (fan_inst == &fan4)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN4, LOW, enableManualOverride);
	}
	else if (fan_inst == &fan5)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN5, LOW, enableManualOverride);
	}	
	else if (fan_inst == &fan6)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN6, LOW, enableManualOverride);
	}	
	else if (fan_inst == &fan7)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN7, LOW, enableManualOverride);
	}	
	else if (fan_inst == &fan8)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN8, LOW, enableManualOverride);
	}	
	
	fanSetSpeed_emc2305(fan_inst, 0);		
	fan_inst->state = OFF;
}

uint8_t fanReadRpm_emc2305(struct fan_vars *fan_inst)
{	
	uint8_t err = 0;
	uint8_t tach_high_byte = 0;
	uint8_t tach_low_byte = 0;
	uint32_t k = 0;
	uint16_t tach = 0;
	uint8_t fan_chip = FAN_CHIP1;
	
	// read tach count
	if (fan_inst == &fan1)
	{
		i2c_txdat[0] = 0x3e;
		k = 2;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan2)
	{
		i2c_txdat[0] = 0x4e;
		k = 2;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan3)
	{
		i2c_txdat[0] = 0x5e;
		k = 2;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan4)
	{
		i2c_txdat[0] = 0x6e;
		k = 2;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan5)
	{
		i2c_txdat[0] = 0x3e;
		k = 2;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan6)
	{
		i2c_txdat[0] = 0x4e;
		k = 2;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan7)
	{
		i2c_txdat[0] = 0x5e;
		k = 2;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan8)
	{
		i2c_txdat[0] = 0x6e;
		k = 2;
		fan_chip = FAN_CHIP2;
	}	
			
	err = i2cWriteRead_emc2305(1, fan_chip);
	if (err != STATUS_OK)
	{
		return err;
	}
	tach_high_byte = i2c_rxdat[0];
	i2c_txdat[0]++;
	err = i2cWriteRead_emc2305(1, fan_chip);
	if (err != STATUS_OK)
	{
		return err;
	}
	tach_low_byte = i2c_rxdat[0];
	tach = (tach_high_byte<<5) + (tach_low_byte>>3);
	
	fan_inst->rpm = (uint32_t) ( (float) (k*FAN_SAMPLE_FREQ*60) / tach );	
	
	return err;
}

uint8_t fanReadConfig_emc2305(struct fan_vars *fan_inst)
{
	uint8_t err = 0;
	uint8_t fan_chip = FAN_CHIP1;

	// read fan config
	if (fan_inst == &fan1)
	{
		i2c_txdat[0] = 0x32;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan2)
	{
		i2c_txdat[0] = 0x42;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan3)
	{
		i2c_txdat[0] = 0x52;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan4)
	{
		i2c_txdat[0] = 0x62;
		fan_chip = FAN_CHIP1;
	}
	else if (fan_inst == &fan5)
	{
		i2c_txdat[0] = 0x32;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan6)
	{
		i2c_txdat[0] = 0x42;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan7)
	{
		i2c_txdat[0] = 0x52;
		fan_chip = FAN_CHIP2;
	}
	else if (fan_inst == &fan8)
	{
		i2c_txdat[0] = 0x62;
		fan_chip = FAN_CHIP2;
	}	
		
	err = i2cWriteRead_emc2305(1, fan_chip);
	if (err != STATUS_OK)
	{
		return err;
	}
	fan_inst->emc2305_fan_config = i2c_rxdat[0];
	//fan_poles = ((fan_config & 0x18)>>3) + 1;
	fan_inst->emc2305_fan_range = ((fan_inst->emc2305_fan_config & 0x60)>>5) * 2;
	
	return err;
}

uint8_t fanReadStallStatus_emc2305(void)
{
	uint8_t err = 0;
	//uint8_t fan_stall_bit = 0;
	
	i2c_txdat[0] = 0x25;
	err = i2cWriteRead_emc2305(1, FAN_CHIP1);
	if (err != STATUS_OK)
	{
		return err;
	}
	err = i2cWriteRead_emc2305(1, FAN_CHIP1);
	if (err != STATUS_OK)
	{
		return err;
	}
	
	fan1.stall = i2c_rxdat[0] & 0x01;
	fan2.stall = (i2c_rxdat[0] >> 1) & 0x01;
	fan3.stall = (i2c_rxdat[0] >> 2) & 0x01;
	fan4.stall = (i2c_rxdat[0] >> 3) & 0x01;

	err = 0;
	i2c_txdat[0] = 0x25;
	err = i2cWriteRead_emc2305(1, FAN_CHIP2);
	if (err != STATUS_OK)
	{
		return err;
	}
	err = i2cWriteRead_emc2305(1, FAN_CHIP2);
	if (err != STATUS_OK)
	{
		return err;
	}
	
	fan5.stall = i2c_rxdat[0] & 0x01;
	fan6.stall = (i2c_rxdat[0] >> 1) & 0x01;
	fan7.stall = (i2c_rxdat[0] >> 2) & 0x01;
	fan8.stall = (i2c_rxdat[0] >> 3) & 0x01;

	return err;
}

void fanControl(struct fan_vars *fan_inst, float voltage)
{
	//if (inhibitOutput==false) {
	
	if (!enableFanOverride)
	{
		fan_inst->targetDutyPercent = fanCalcPwmDutyPercent(fan_inst, voltage);
	}
	
	if ( (fan_inst == &fan4) && (inhibitChamberFan == OFF || enableFanOverride == true) )
	{
		// if fan is currently off, then kick at max power to start
		if ( (fan4.targetDutyPercent > 0) && (fan4.state == OFF) )
		{
			fanStart_emc2305(&fan4);
		}
		else if ( (fan4.targetDutyPercent==0 && fan4.state != OFF) )
		{
			fanStop_emc2305(&fan4);
		}

		fan4.intDutyPercent = fan4.targetDutyPercent;
				
		// after fan has been kicked, then set to target speed
		fanSetSpeed_emc2305(&fan4, fan4.intDutyPercent);
	}	
	if ( (fan_inst == &fan1) && (inhibitChamberFan == OFF || enableFanOverride == true) )
	{
		// if fan is currently off, then kick at max power to start
		if ( (fan1.targetDutyPercent > 0) && (fan1.state == OFF) )
		{
			fanStart_emc2305(&fan1);
		}
		else if ( (fan1.targetDutyPercent==0 && fan1.state != OFF) )
		{
			fanStop_emc2305(&fan1);
		}

		fan1.intDutyPercent = fan1.targetDutyPercent;
		
		// after fan has been kicked, then set to target speed
		fanSetSpeed_emc2305(&fan1, fan1.intDutyPercent);
	}		
	else
	{
		// if fan is currently off, then kick at max power to start
		if ( (fan_inst->targetDutyPercent > 0) && (fan_inst->state == OFF) )
		{
			fanStart_emc2305(fan_inst);
		}
		else if ( (fan_inst->targetDutyPercent==0 && fan_inst->state != OFF) )
		{
			fanStop_emc2305(fan_inst);
		}
	}
	
	// add inertia to speed change
	//fan_inst->intDutyPercent = fanCalcIntRampVal(fan_inst, fan_inst->targetDutyPercent, TASK2_SEC);
	fan_inst->intDutyPercent = fan_inst->targetDutyPercent;

	// after fan has been kicked, then set to target speed
	fanSetSpeed_emc2305(fan_inst, fan_inst->intDutyPercent);
	//}
}

uint8_t fanStateMachine(struct fan_vars *fan_inst)
{
	uint8_t newSpeed;
	float vset = ps1.desiredVoltage;
	
	// S1 /////////////////////////////////
	if (fan_inst->fan_state == FAN_STATE_S1)
	{
		if		(vset < fan_inst->lim1)		{ fan_inst->fan_state = FAN_STATE_S2; }
		//else if (vset >= fan_inst->lim1)	{ fan_inst->fan_state = FAN_STATE_S1; }
			
		newSpeed = fan_inst->spd1;
	} 
	
	// S2 /////////////////////////////////
	else if (fan_inst->fan_state == FAN_STATE_S2)
	{
		if		(vset >= fan_inst->lim1p)	{ fan_inst->fan_state = FAN_STATE_S1; }
		//else if (vset < fan_inst->lim1p)	{ fan_inst->fan_state = FAN_STATE_S2; }
		else if (vset < fan_inst->lim2)		{ fan_inst->fan_state = FAN_STATE_S3; }
		//else if (vset >= fan_inst->lim2)	{ fan_inst->fan_state = FAN_STATE_S2; }		
			
		newSpeed = fan_inst->spd2;			
	}
	
	// S3 /////////////////////////////////
	else if (fan_inst->fan_state == FAN_STATE_S3)
	{
		if		(vset >= fan_inst->lim2p)	{ fan_inst->fan_state = FAN_STATE_S2; }
		//else if (vset < fan_inst->lim2p)	{ fan_inst->fan_state = FAN_STATE_S3; }
		else if (vset < fan_inst->lim3)		{ fan_inst->fan_state = FAN_STATE_S4; }
		//else if (vset >= fan_inst->lim3)	{ fan_inst->fan_state = FAN_STATE_S3; }
			
		newSpeed = fan_inst->spd3;			
	}	
	
	// S4 /////////////////////////////////
	else if (fan_inst->fan_state == FAN_STATE_S4)
	{
		if		(vset >= fan_inst->lim3p)	{ fan_inst->fan_state = FAN_STATE_S3; }
		//else if (vset < fan_inst->lim3p)	{ fan_inst->fan_state = FAN_STATE_S4; }
		else if (vset < fan_inst->lim4)		{ fan_inst->fan_state = FAN_STATE_S5; }
		//else if (vset >= fan_inst->lim4)	{ fan_inst->fan_state = FAN_STATE_S4; }
			
		newSpeed = fan_inst->spd4;		
	}	
	
	// S5 /////////////////////////////////
	else if (fan_inst->fan_state == FAN_STATE_S5)
	{
		if		(vset >= fan_inst->lim4p)	{ fan_inst->fan_state = FAN_STATE_S4; }
		//else if (vset < fan_inst->lim4p)	{ fan_inst->fan_state = FAN_STATE_S5; }
		else if	(vset < fan_inst->lim5)		{ fan_inst->fan_state = FAN_STATE_S6; }
		//else if (vset >= fan_inst->lim5)	{ fan_inst->fan_state = FAN_STATE_S5; }	
			
		newSpeed = fan_inst->spd5;	
	}	

	// S6 /////////////////////////////////
	else if (fan_inst->fan_state == FAN_STATE_S6)
	{
		if		(vset >= fan_inst->lim5p)	{ fan_inst->fan_state = FAN_STATE_S5; }
		//else if (vset < fan_inst->lim5p)	{ fan_inst->fan_state = FAN_STATE_S6; }
		
		newSpeed = fan_inst->spd6;
	}
		
	// DEFAULT ////////////////////////////
	else
	{
		fan_inst->fan_state = FAN_STATE_S1;
		newSpeed = fan_inst->spd1;
	}
	
	return newSpeed;
}

float fanCalcPwmDutyPercent(struct fan_vars *fan_inst, float voltage)
{
	float fNewDutyPercent = DEFAULT_FAN_MAX_DUTY;
	//float voltage = ps1.desiredVoltage;
	//uint8_t idx = 0;

	if (voltage < 0)
	{
		voltage = 0;
	}
	else if (voltage >= maxOutputVoltage)
	{
		voltage = maxOutputVoltage;
	}
	voltage = round(voltage);
	
	//idx = (uint8_t)voltage;
	//if (idx < 0)
	//{
	//	idx = 0;
	//}
	//else if (idx > FAN_SPD_PERCENT_LOOKUP_TABLE_SIZE)
	//{
	//	idx = FAN_SPD_PERCENT_LOOKUP_TABLE_SIZE-1;
	//}		
		
	//if (fan_inst == &fan3)
	//{		
	//	fNewDutyPercent = fan3_spd_percent_lookup_table[idx];
	//}
	//else 
	if (fan_inst == &fan2 || fan_inst == &fan5 || fan_inst == &fan6)
	{
		//fNewDutyPercent = round(pidFan2.Output) + fan2_spd_percent_lookup_table[idx];

		if (max(chamber1.rejectTemp, chamber2.rejectTemp) > FAN_MAX_SPD_REJECT_TEMP_LIMIT)
		{
			fNewDutyPercent = 100.0;
			fan_inst->fan_state = FAN_STATE_REJECT_OT;
		}
		else
		{
			fNewDutyPercent = (float)fanStateMachine(fan_inst);
		}		
	}	
	/*
	else if (fan_inst == &fan3)
	{
		fNewDutyPercent = (float)fanStateMachine(&fan3);
	}
	*/
	else if (fan_inst == &fan1)
	{
		if (defrostActive)
		{
			fNewDutyPercent = 100.0;
			fan1.fan_state = FAN_STATE_DEFROST;
		}	
		else
		{
			fNewDutyPercent = (float)fanStateMachine(&fan1);
		}	
		
		fNewDutyPercent =  round(pidFan4.Output) + fNewDutyPercent;	
	}
	else if (fan_inst == &fan3 || fan_inst == &fan8 || fan_inst == &fan7)
	{
		//fNewDutyPercent = round(pidFan2.Output) + fan2_spd_percent_lookup_table[idx];

		if (max(chamber1.rejectTemp, chamber2.rejectTemp) > FAN_MAX_SPD_REJECT_TEMP_LIMIT)
		{
			fNewDutyPercent = 100.0;
			fan_inst->fan_state = FAN_STATE_REJECT_OT;
		}
		else
		{
			fNewDutyPercent = (float)fanStateMachine(fan_inst);
		}
	}
	else if (fan_inst == &fan4)
	{
		if (defrostActive)
		{
			fNewDutyPercent = 100.0;
			fan4.fan_state = FAN_STATE_DEFROST;
		}
		else
		{
			fNewDutyPercent = (float)fanStateMachine(&fan4);
		}
		
		fNewDutyPercent =  round(pidFan4.Output) + fNewDutyPercent;
	}	
	else
	{
		fNewDutyPercent = 100.0;
	}
	
	if (fNewDutyPercent >= DEFAULT_FAN_MAX_DUTY)
	{
		fNewDutyPercent = DEFAULT_FAN_MAX_DUTY;
	}
	else if (fNewDutyPercent <= 0)
	{
		fNewDutyPercent = 0;
	}	
	
	return fNewDutyPercent;
}

/*
uint32_t fanSetPwmDutyReg(struct fan_vars *fan_inst, float fNewDuty)
{
	if (fNewDuty <= 0)
	{
		fan_inst->speedPwmDutyRegVal = 1;
		//fan_inst->state = OFF;				
	}
	else
	{
		fan_inst->speedPwmDutyRegVal = (uint32_t)(fNewDuty/100 * MAX_FAN_PWM_DUTY_REG_VAL);
		if (fan_inst->speedPwmDutyRegVal > MAX_FAN_PWM_DUTY_REG_VAL) {
			fan_inst->speedPwmDutyRegVal = MAX_FAN_PWM_DUTY_REG_VAL;
		}
		if (fan_inst->speedPwmDutyRegVal <= 0) {
			fan_inst->speedPwmDutyRegVal = 1;
		}
		//fan_inst->state = ON;		
	}

	if (fan_inst == &fan1)
	{
		pwm7_1_cmp_val = fan_inst->speedPwmDutyRegVal;
	}
	else if (fan_inst == &fan2)
	{
		pwm7_2_cmp_val = fan_inst->speedPwmDutyRegVal;
	}
	
	return fan_inst->speedPwmDutyRegVal;
}
*/
/*
void fanStart(struct fan_vars *fan_inst)
{
	if (fan_inst == &fan1)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN1, HIGH, enableManualOverride);
		tc_enable(&tc4_module);
	}
	else if (fan_inst == &fan2)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN2, HIGH, enableManualOverride);
		tc_enable(&tc5_module);
	}
	fanSetPwmDutyReg(fan_inst, 100);
	delay_ms(1000);
	fan_inst->state = ON;
}

void fanStop(struct fan_vars *fan_inst)
{
	if (fan_inst == &fan1)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN1, LOW, enableManualOverride);
		//tc_disable(&tc4_module);
		//fan1.rpm = 0;
	}
	else if (fan_inst == &fan2)
	{
		port_pin_set_output_level_EX(PIN_ENBL_FAN2, LOW, enableManualOverride);
		//tc_disable(&tc5_module);
		//fan2.rpm = 0;
	}
	fanSetPwmDutyReg(fan_inst, 0);
	fan_inst->state = OFF;
}
*/
/*
// implement iir sp lpf to calc intermediate fan duty values
uint8_t fanCalcIntRampVal(struct fan_vars *fan_inst, uint8_t target_setpoint, uint8_t dt)
{
	float a;
	float int_spd_setting;
	
	a = (float)dt / (fan_inst->rc + dt);
	
	int_spd_setting = fan_inst->prevSpdSetting + ( a * (target_setpoint - fan_inst->prevSpdSetting) );
	
	if (int_spd_setting >= DEFAULT_FAN_MAX_DUTY)
	{
		int_spd_setting = DEFAULT_FAN_MAX_DUTY;
	}
	else if (int_spd_setting <= 0)
	{
		int_spd_setting = 0;
	}

	fan_inst->prevSpdSetting = round(int_spd_setting);
	
	return int_spd_setting;
}
*/
uint8_t fanCheckRpmOutOfBounds(struct fan_vars *fan_inst)
{
	uint8_t idx = 0;
	
	idx = fan_inst->intDutyPercent;
	
	if (idx >= FAN_RPM_LOOKUP_TABLE_SIZE) { idx = FAN_RPM_LOOKUP_TABLE_SIZE; }
	else if (idx <= 0) { idx = 0; }
	
	if (fan_inst->fanType==Sanyo_9ga0912p4g03)
	{
		fan_inst->fanRpmSetting = fan_sanyo_9ga0912p4g03_rpm_lookup_table[idx];
	}
	else if (fan_inst->fanType==Sanyo_9s0912p4f011)
	{
		fan_inst->fanRpmSetting = fan_sanyo_9ga0912p4g03_rpm_lookup_table[idx];
	}
	else if (fan_inst->fanType==Sanyo_9ga0912p4j03)
	{
		fan_inst->fanRpmSetting = fan_sanyo_9ga0912p4j03_rpm_lookup_table[idx];
	}
	else if (fan_inst->fanType==Nidec_v12e12bmm9)
	{
		fan_inst->fanRpmSetting = fan_nidec_v12e12bmm9_rpm_lookup_table[idx];
	}
		
	//fan_inst->fanSpdUprLim = fan_inst->fanRpmSetting + (fan_inst->fanRpmSetting * ((float)fanRpmErrorPercentage/100) );
	//fan_inst->fanSpdLwrLim = fan_inst->fanRpmSetting - (fan_inst->fanRpmSetting * ((float)fanRpmErrorPercentage/100) );
	fan_inst->fanSpdLwrLim = fan_inst->fanRpmSetting - fanSpdTolerance;
	
	if ( (fan_inst->rpm <= fan_inst->fanSpdLwrLim) )
	{
		return 1;
	}
	/*
	else if ( (fan_inst->rpm >= fan_inst->fanSpdUprLim) )
	{
		// may be loss of pwm control 
	}
	*/
	else
	{
		return 0;
	}
}

void checkForFanFaults()
{
	// fan stall, reject 1 //////////////////////////////
	if ( (fanCheckRpmOutOfBounds(&fan1)==true) && (fan1.state==ON ) )
	{
		fan1LowSpdErrCnt++;
		
		if (fan1.stall==FAN_STALLED)
		{
			fan1ErrorCount++;
		}
	}
	else
	{
		fan1LowSpdErrCnt = 0;
		fan1ErrorCount = 0;
	}
	
	// fan stall, reject 2 //////////////////////////////
	if ( (fanCheckRpmOutOfBounds(&fan2)==true) && (fan2.state==ON) )
	{
		fan2LowSpdErrCnt++;
		
		if (fan2.stall==FAN_STALLED)
		{
			fan2ErrorCount++;
		}
	}
	else
	{
		fan2LowSpdErrCnt = 0;
		fan2ErrorCount = 0;
	}
	
	// fan stall, chamber ///////////////////////////////
	if ( (fanCheckRpmOutOfBounds(&fan4)==true) && (fan4.state==ON) )
	{
		fan4LowSpdErrCnt++;
		
		if (fan4.stall==FAN_STALLED)
		{
			fan4ErrorCount++;
		}
	}
	else
	{
		if (doorOpenPresentStatus==CLOSED)
		{
			fan4LowSpdErrCnt = 0;
			fan4ErrorCount = 0;
		}
	}

	// update error register for fan errors /////////////
	if (maxFanErrorCount > 0)
	{
		if (fan1ErrorCount >= maxFanErrorCount)
		{
			errorRegister |= ERROR_REJ_FAN_SPEED;
		}
		else
		{
			errorRegister &= ~ERROR_REJ_FAN_SPEED;
		}
		
		if (fan2ErrorCount >= maxFanErrorCount)
		{
			errorRegister |= ERROR_REJ_FAN_SPEED;
		}
		else
		{
			errorRegister &= ~ERROR_REJ_FAN_SPEED;
		}
		
		if (fan4ErrorCount >= maxFanErrorCount)
		{
			errorRegister |= ERROR_CHA_FAN_SPEED;
		}
		else
		{
			errorRegister &= ~ERROR_CHA_FAN_SPEED;
		}
	}
}