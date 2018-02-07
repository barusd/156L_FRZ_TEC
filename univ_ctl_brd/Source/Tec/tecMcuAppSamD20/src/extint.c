/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "extern.h"

/************************************************************************/
/* Callback Functions                                                   */
/************************************************************************/
#ifdef ENABLE_SLEEP_WAKE
void extint_slp_callback(uint32_t channel)
{
	/*
	REG_PM_APBAMASK = 0x0000007f;
	REG_PM_APBBMASK = 0x0000001f;
	REG_PM_APBCMASK = 0x0001b2a2;
	*/
	
	// delay to let edge settle
	delay_us(1);

	if (port_pin_get_input_level(PIN_SLP)==0)
	{
		lowPowerModeExit();
	}
	
	// clear interrupt flag
	REG_EIC_INTFLAG = 0x8000;
}
#endif

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
#ifdef ENABLE_SLEEP_WAKE
void configure_extint(void)
{
	// configure external interrupt controller
	struct extint_chan_conf extint_chan_config;
	extint_chan_get_config_defaults(&extint_chan_config);
	/*
	// PA22 is fan1 tach input
	extint_chan_config.gpio_pin = PIN_PA22A_EIC_EXTINT6;
	extint_chan_config.gpio_pin_mux = MUX_PA22A_EIC_EXTINT6;
	//extint_chan_config.gpio_pin_pull = SYSTEM_PINMUX_PIN_PULL_UP;
	extint_chan_config.detection_criteria = EXTINT_DETECT_HIGH;
	extint_chan_config.wake_if_sleeping = false;
	extint_chan_set_config(6, &extint_chan_config);
	
	// PA23 is fan2 tach input
	extint_chan_config.gpio_pin = PIN_PA23A_EIC_EXTINT7;
	extint_chan_config.gpio_pin_mux = MUX_PA23A_EIC_EXTINT7;
	//extint_chan_config.gpio_pin_pull = SYSTEM_PINMUX_PIN_PULL_UP;
	extint_chan_config.detection_criteria = EXTINT_DETECT_HIGH;
	extint_chan_config.wake_if_sleeping = false;	
	extint_chan_set_config(7, &extint_chan_config);
	*/
	// PA27 is sleep enable input
	extint_chan_config.gpio_pin = PIN_PA27A_EIC_EXTINT15;
	extint_chan_config.gpio_pin_mux = MUX_PA27A_EIC_EXTINT15;
	//extint_chan_config.gpio_pin_pull = SYSTEM_PINMUX_PIN_PULL_UP;
	extint_chan_config.filter_input_signal = false;
	extint_chan_config.detection_criteria = EXTINT_DETECT_FALLING;
	extint_chan_config.wake_if_sleeping = true;
	extint_chan_set_config(15, &extint_chan_config);
	
	extint_enable();
	
	// clear false interrupt created by config of SLP pin OUTSET reg
	REG_EIC_INTFLAG = 0xffff; 
	
	// configure external interrupt module to be event generator
	struct extint_events extint_event_conf;
	extint_event_conf.generate_event_on_detect[6] = true;
	extint_event_conf.generate_event_on_detect[7] = true;
	//extint_event_conf.generate_event_on_detect[15] = true;
	extint_enable_events(&extint_event_conf);
}

void configure_extint_callbacks(void)
{
	extint_register_callback(extint_slp_callback, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(15,	EXTINT_CALLBACK_TYPE_DETECT);
}
#endif
