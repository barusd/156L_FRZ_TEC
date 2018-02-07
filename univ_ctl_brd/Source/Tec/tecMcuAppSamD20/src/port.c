/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "dbgCmds.h"
#include "extern.h"

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
void configure_port_pins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);

	// inputs, internal pull-ups enabled
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(PIN_RST_TO_SLV_N, &config_port_pin);
	port_pin_set_config(PIN_SLP,		  &config_port_pin);
	
	// write OUTSET reg to 1 to set input pull direction to internal pull-up
	//port_pin_set_output_level_EX(PIN_SLP, HIGH, enableManualOverride);
	
	// unused pins, inputs, internal pull-ups enabled
	//port_pin_set_config(PIN_PA07_UNUSED, &config_port_pin);
	//port_pin_set_config(PIN_PA22_UNUSED, &config_port_pin);
	//port_pin_set_config(PIN_PA23_UNUSED, &config_port_pin);
	port_pin_set_config(PIN_PB30_UNUSED, &config_port_pin);

	// inputs, internal pull-ups disabled
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	//port_pin_set_config(PIN_TACH_FAN1, &config_port_pin);
	//port_pin_set_config(PIN_TACH_FAN2, &config_port_pin);
	//port_pin_set_config(PIN_PB17,	   &config_port_pin);	// i2c scl to input as default
	//port_pin_set_config(PIN_PB16,	   &config_port_pin);	// i2c sda to input as default
	//port_pin_set_config(PIN_PA23,	   &config_port_pin);	// i2c F_scl to input as default
	//port_pin_set_config(PIN_PA22,	   &config_port_pin);	// i2c F_sda to input as default
	
	// set default output values
	port_pin_set_output_level_EX(PIN_ENBL_BUCK1,   LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_OUTPUT1, LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_OUTPUT2, LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_OUTA_COLD, LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_OUTA_HOT , LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_LED_0_PIN,    LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_RDY_N,       HIGH, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_LOAD2,   LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN1,	   LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN2,    LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN3,	   LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN4,    LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN5,	   LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN6,    LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN7,	   LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_FAN8,    LOW, enableManualOverride);	
	port_pin_set_output_level_EX(PIN_ENBL_LAMP,    LOW, enableManualOverride);
	//port_pin_set_output_level_EX(PIN_ENBL_VBATMON, LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_OUTB_COLD,  LOW, enableManualOverride);
	port_pin_set_output_level_EX(PIN_ENBL_OUTB_HOT ,  LOW, enableManualOverride);	
	
	// outputs, internal pull-ups disabled
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(PIN_ENBL_BUCK1,   &config_port_pin);
	port_pin_set_config(PIN_ENBL_OUTPUT1, &config_port_pin);
	port_pin_set_config(PIN_ENBL_OUTPUT2, &config_port_pin);
	port_pin_set_config(PIN_ENBL_OUTA_COLD, &config_port_pin);
	port_pin_set_config(PIN_ENBL_OUTA_HOT , &config_port_pin);	
	port_pin_set_config(PIN_LED_0_PIN,    &config_port_pin);
	port_pin_set_config(PIN_RDY_N,		  &config_port_pin);
	port_pin_set_config(PIN_ENBL_LOAD2,   &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN1,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN2,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN3,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN4,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN5,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN6,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN7,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_FAN8,    &config_port_pin);
	port_pin_set_config(PIN_ENBL_LAMP,    &config_port_pin);	
	//port_pin_set_config(PIN_ENBL_VBATMON, &config_port_pin);
	port_pin_set_config(PIN_ENBL_OUTB_COLD,  &config_port_pin);
	port_pin_set_config(PIN_ENBL_OUTB_HOT ,  &config_port_pin);	
}
