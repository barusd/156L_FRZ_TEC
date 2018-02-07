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
uint32_t condHeaterStartTime = 0;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
uint8_t lookupDoorHeaterDutyVal(float setpoint)
{
	uint16_t dutyVal = 0;
	uint8_t idx = 0;
	float sp = 0;
	
	if (setpoint >= HTR_UPPER_TCHA_LIMIT)
	{
		setpoint = HTR_UPPER_TCHA_LIMIT;
	}
	sp = round(setpoint * 10) / 10;
	idx = (uint8_t) ( (2*sp) - (2*HTR_LOWER_TCHA_LIMIT) );
	if (idx < 0)
	{
		idx = 0;
	}
	else if (idx > DOOR_HEATER_DUTY_LOOKUP_TABLE_SIZE)
	{
		idx = DOOR_HEATER_DUTY_LOOKUP_TABLE_SIZE-1;
	}
	dutyVal = door_heater_duty_lookup_table[idx];
	if (dutyVal >= 100)
	{
		dutyVal = 100;
	}
	else if (dutyVal <= 0)
	{
		dutyVal = 0;
	}
	return dutyVal;
}

uint8_t condHeaterStateMachine()
{
	uint8_t htrDuty = 0;
	
	#ifdef HIGH_PERFORMANCE

	// OFF ////////////////////////////////
	if (condHeater.sm_state==STATE_CONDHTR_OFF)
	{
		if (condensateTemp<TSNS_INSANE_LOW_LIMIT)
		{
			condHeater.sm_state = STATE_CONDHTR_S1;
		}
		else if (condensateTemp>TSNS_INSANE_LOW_LIMIT)
		{
			if (highUsageModeFlag==true)
			{
				condHeater.sm_state = STATE_CONDHTR_S4;
			}
			else
			{
				condHeater.sm_state = STATE_CONDHTR_S2;
			}
		}
		
		htrDuty = COND_HTR_OFF_DUTY;
		condHeater.prev_sm_state =  STATE_CONDHTR_OFF;
	}
	
	// S1 /////////////////////////////////
	else if (condHeater.sm_state==STATE_CONDHTR_S1)
	{
		if (condensateTemp>TSNS_INSANE_LOW_LIMIT)
		{
			condHeater.sm_state = STATE_CONDHTR_S2;
		}
		
		htrDuty = COND_HTR_S1_DUTY;
		condHeater.prev_sm_state =  STATE_CONDHTR_S1;
	}
	
	// S2 /////////////////////////////////
	else if (condHeater.sm_state==STATE_CONDHTR_S2)
	{
		if (condensateTemp<TSNS_INSANE_LOW_LIMIT)
		{
			condHeater.sm_state = STATE_CONDHTR_S1;
		}
		else if ( condensateTemp >= (min(COND_HTR_OFF_TEMP_LIMIT, chamber1.rejectTemp+5)) )
		{
			condHeater.sm_state = STATE_CONDHTR_S3;
			condHeaterStartTime = presentTimeInSec();
		}
		else if ( (condensateTemp>TSNS_INSANE_LOW_LIMIT) && (highUsageModeFlag==true) )
		{
			condHeater.sm_state = STATE_CONDHTR_S4;
		}
		
		htrDuty = COND_HTR_S2_DUTY;
		condHeater.prev_sm_state =  STATE_CONDHTR_S2;
	}

	// S3 /////////////////////////////////
	else if (condHeater.sm_state==STATE_CONDHTR_S3)
	{
		if ( (presentTimeInSec() > condHeaterStartTime+COND_HTR_OFFTIME_IN_SEC) || (highUsageModeFlag==true) )
		{
			condHeater.sm_state = STATE_CONDHTR_OFF;
			condHeaterStartTime = 0;
		}
		
		htrDuty = COND_HTR_OFF_DUTY;
		condHeater.prev_sm_state =  STATE_CONDHTR_S3;
	}

	// S4 /////////////////////////////////
	else if (condHeater.sm_state==STATE_CONDHTR_S4)
	{
		if (condensateTemp<TSNS_INSANE_LOW_LIMIT)
		{
			condHeater.sm_state = STATE_CONDHTR_S1;
		}		
		else if ( condensateTemp >= (min(COND_HTR_OFF_TEMP_LIMIT, chamber1.rejectTemp+15)) )
		{
			condHeater.sm_state = STATE_CONDHTR_S3;
			condHeaterStartTime = presentTimeInSec();
			if (highUsageModeFlag==true)
			{
				highUsageModeFlag = false;
			}			
		}
		
		htrDuty = COND_HTR_HTR_S4_DUTY;
		condHeater.prev_sm_state =  STATE_CONDHTR_S4;
	}
		
	#endif
	
	//condHeater.duty = htrDuty;
	return htrDuty;
}

void setHeaterState(struct heater_vars *htr_inst, uint8_t val)
{
	uint8_t pin;
	
	htr_inst->duty = val;
	
	if (htr_inst == &condHeater)
	{
		pin = PIN_ENBL_VHTR1;
	}
	else if (htr_inst == &doorHeater)
	{
		pin = PIN_ENBL_VHTR2;
	}
	else if (htr_inst == &htxAHeater)
	{
		pin = PIN_ENBL_OUTPUT1;
	}
	else if (htr_inst == &htxBHeater)
	{
		pin = PIN_ENBL_OUTPUT2;
	}
	
	if ( (val>=100) || (val==0) )
	{
		// gpio mode
		
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
		port_pin_set_output_level(pin, val);
		
		// update status
		htr_inst->output_state = val;
	}
	else
	{
		// pwm mode
		
		if (pin == PIN_ENBL_VHTR1)
		{
			// configure as tc7 pwm output
			struct system_pinmux_config config_pinmux;
			system_pinmux_get_config_defaults(&config_pinmux);
			config_pinmux.mux_position = PINMUX_PB22F_TC7_WO0;
			config_pinmux.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
			system_pinmux_pin_set_config(pin, &config_pinmux);
			tc_enable(&tc7_instance);
			
			// set duty
			pwm7_1_cmp_val = (val * PWM_MAX_DUTY_VAL) / 100;
			
			// set status
			htr_inst->output_state = ON;
		}
		if (pin == PIN_ENBL_VHTR2)
		{
			// configure as tc7 pwm output
			struct system_pinmux_config config_pinmux;
			system_pinmux_get_config_defaults(&config_pinmux);
			config_pinmux.mux_position = PINMUX_PB23F_TC7_WO1;
			config_pinmux.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
			system_pinmux_pin_set_config(pin, &config_pinmux);
			tc_enable(&tc7_instance);
			
			// set duty
			pwm7_2_cmp_val = (val * PWM_MAX_DUTY_VAL) / 100;
			
			// set status
			htr_inst->output_state = ON;
		}
	}
}