/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "extern.h"

/************************************************************************/
/* Module Instances                                                     */
/************************************************************************/
//struct tc_module tc1_instance;
struct tc_module tc0_module;
struct tc_module tc2_instance;
struct tc_module tc4_module;
struct tc_config tc4_config;
struct tc_module tc5_module;
struct tc_config tc5_config;
struct tc_module tc7_instance;

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
uint32_t pwm2_0_cmp_val = 1;
uint32_t pwm2_1_cmp_val = 1;
uint32_t pwm4_cmp_val = 1;
uint32_t pwm0_cmp_val = 0x7fff;
uint32_t pwm5_cmp_val = 0x7fff;
uint32_t pwm7_1_cmp_val = 1;
uint32_t pwm7_2_cmp_val = 1;
uint32_t tach0_capture = 0;
uint32_t tach1_capture = 0;

/************************************************************************/
/* Callback Functions                                                   */
/************************************************************************/
static void tc2_callback_to_change_duty_cycle(struct tc_module *const module_inst)
{
	tc_set_compare_value(&tc2_instance, TC_COMPARE_CAPTURE_CHANNEL_0, pwm2_0_cmp_val);  
}

static void tc2_1_callback_to_change_duty_cycle(struct tc_module *const module_inst)
{
	tc_set_compare_value(&tc2_instance, TC_COMPARE_CAPTURE_CHANNEL_1, pwm2_1_cmp_val); 
}

static void tc0_callback_to_change_duty_cycle(struct tc_module *const module_inst)
{
	tc_set_compare_value(&tc0_module, TC_COMPARE_CAPTURE_CHANNEL_1, pwm0_cmp_val);
}

static void tc5_callback_to_change_duty_cycle(struct tc_module *const module_inst)
{
	tc_set_compare_value(&tc5_module, TC_COMPARE_CAPTURE_CHANNEL_0, pwm5_cmp_val);
}
/*
static void tach0_capture_callback_function(struct tc_module *const module_inst)
{
	tach0_capture = tc_get_capture_value(&tc4_module, TC_COMPARE_CAPTURE_CHANNEL_0);
	tach0_capture += 2000;  // correction factor to account for isr() delay
	fan1.rpm = 30000000 / tach0_capture;  // assumes two ticks per revolution
}

static void tach1_capture_callback_function(struct tc_module *const module_inst)
{
	tach1_capture = tc_get_capture_value(&tc5_module, TC_COMPARE_CAPTURE_CHANNEL_0);
	tach1_capture += 2000;  // correction factor to account for isr() delay
	fan2.rpm = 30000000 / tach1_capture;  // assumes two ticks per revolution
}
*/

static void tc7_callback_to_change_duty_cycle(struct tc_module *const module_inst)
{
	tc_set_compare_value(&tc7_instance, TC_COMPARE_CAPTURE_CHANNEL_0, pwm7_1_cmp_val);
	tc_set_compare_value(&tc7_instance, TC_COMPARE_CAPTURE_CHANNEL_1, pwm7_2_cmp_val);
}

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
// TC0 cmp1 is switch fabric output3
void configure_tc0(void)
{
	struct tc_config config_tc0;
	tc_get_config_defaults(&config_tc0);
	config_tc0.clock_source	   = GCLK_GENERATOR_5;
	config_tc0.clock_prescaler = TC_CLOCK_PRESCALER_DIV1024;
	config_tc0.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc0.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc0.channel_pwm_out_enabled[1] = true;
	config_tc0.channel_pwm_out_pin[1]     = PIN_PB31F_TC0_WO1;
	config_tc0.channel_pwm_out_mux[1]     = MUX_PB31F_TC0_WO1;
				
	/*
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_ENBL_OUTPUT1,   &config_port_pin);
	*/
				
	// set duty
	config_tc0.size_specific.size_16_bit.compare_capture_channel[1] = pwm0_cmp_val;
					
	tc_init(&tc0_module, TC0, &config_tc0);
	//tc_disable(&tc0_module);
}

void configure_tc0_callbacks(void)
{
	tc_register_callback(
	&tc0_module,
	tc0_callback_to_change_duty_cycle,
	TC_CALLBACK_CC_CHANNEL1);
}

// TC5 cmp0 is switch fabric output1
void configure_tc5(void)
{
	struct tc_config config_tc5;
	tc_get_config_defaults(&config_tc5);
	config_tc5.clock_source	   = GCLK_GENERATOR_5;
	config_tc5.clock_prescaler = TC_CLOCK_PRESCALER_DIV1024;
	config_tc5.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc5.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc5.channel_pwm_out_enabled[0] = true;
	config_tc5.channel_pwm_out_pin[0]     = PIN_PB14E_TC5_WO0;
	config_tc5.channel_pwm_out_mux[0]     = MUX_PB14E_TC5_WO0;
				
	/*
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_ENBL_OUTPUT1,   &config_port_pin);
	*/
				
	// set duty
	config_tc5.size_specific.size_16_bit.compare_capture_channel[0] = pwm5_cmp_val;
					
	tc_init(&tc5_module, TC5, &config_tc5);
	//tc_disable(&tc5_module);
}

void configure_tc5_callbacks(void)
{
	tc_register_callback(
	&tc5_module,
	tc5_callback_to_change_duty_cycle,
	TC_CALLBACK_CC_CHANNEL0);	
}

// TC2 is boost and bulk reference pwm's
void configure_tc2(void)
{	
	struct tc_config config_tc2;
	tc_get_config_defaults(&config_tc2);
	config_tc2.clock_source	   = GCLK_GENERATOR_1;
	config_tc2.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
	config_tc2.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc2.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;

	config_tc2.size_specific.size_16_bit.compare_capture_channel[0] = 0xFFFF;//RER
	config_tc2.channel_pwm_out_enabled[0] = true;
	config_tc2.channel_pwm_out_pin[0]     = PIN_PA12E_TC2_WO0;
	config_tc2.channel_pwm_out_mux[0]     = MUX_PA12E_TC2_WO0;

	config_tc2.size_specific.size_16_bit.compare_capture_channel[1] = 0xFFFF;//RER
	config_tc2.channel_pwm_out_enabled[1] = true;
	config_tc2.channel_pwm_out_pin[1]     = PIN_PA13E_TC2_WO1;
	config_tc2.channel_pwm_out_mux[1]     = MUX_PA13E_TC2_WO1;

	tc_init(&tc2_instance, TC2, &config_tc2);
	tc_enable(&tc2_instance);
}

void configure_tc2_callbacks(void)
{
	tc_register_callback(&tc2_instance, tc2_callback_to_change_duty_cycle, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL0);
	tc_register_callback(&tc2_instance,	tc2_1_callback_to_change_duty_cycle, TC_CALLBACK_CC_CHANNEL1);
	tc_enable_callback(&tc2_instance, TC_CALLBACK_CC_CHANNEL1);
}


/*
// TC4 is fan1 tachometer input
static void configure_tc4(void)
{
	// configure tc4 module for capture
	//tc_reset(&tc4_module);
	tc_get_config_defaults(&tc4_config);
	tc4_config.clock_source = GCLK_GENERATOR_4;
	tc4_config.clock_prescaler = TC_CLOCK_PRESCALER_DIV2;
	tc4_config.enable_capture_on_channel[0] = true;
	tc4_config.invert_event_input = true;
	tc4_config.event_action = TC_EVENT_ACTION_PPW;
	tc_init(&tc4_module, TC4, &tc4_config);
	struct tc_events tc4_event = { .generate_event_on_compare_channel[0] = true,
								   .generate_event_on_compare_channel[1] = true,
								   .on_event_perform_action = true };
	tc_enable_events(&tc4_module, &tc4_event);
}

// TC4 is fan1 tachometer input
static void configure_tc4_callbacks(void)
{
	tc_register_callback(&tc4_module, tach0_capture_callback_function, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc4_module, TC_CALLBACK_CC_CHANNEL0);	
}

// TC5 is fan2 tachometer input
static void configure_tc5(void)
{
	// configure tc5 module for capture
	//tc_reset(&tc5_module);
	tc_get_config_defaults(&tc5_config);
	tc5_config.clock_source = GCLK_GENERATOR_4;
	tc5_config.clock_prescaler = TC_CLOCK_PRESCALER_DIV2;
	tc5_config.enable_capture_on_channel[0] = true;
	tc5_config.invert_event_input = true;
	tc5_config.event_action = TC_EVENT_ACTION_PPW;
	tc_init(&tc5_module, TC5, &tc5_config);
	struct tc_events tc5_event = { .generate_event_on_compare_channel[0] = true,
		.generate_event_on_compare_channel[1] = true,
	.on_event_perform_action = true };
	tc_enable_events(&tc5_module, &tc5_event);
}

// TC5 is fan2 tachometer input
static void configure_tc5_callbacks(void)
{
	tc_register_callback(&tc5_module, tach1_capture_callback_function, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc5_module, TC_CALLBACK_CC_CHANNEL0);
}
*/
// TC7 is heater1 and 2 PWM outputs
void configure_tc7(void)
{
	struct tc_config config_tc7;
	tc_get_config_defaults(&config_tc7);

	config_tc7.clock_source	   = GCLK_GENERATOR_3; 
	config_tc7.clock_prescaler = TC_CLOCK_PRESCALER_DIV16;
	config_tc7.counter_size    = TC_COUNTER_SIZE_16BIT;
	config_tc7.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM;
	config_tc7.size_specific.size_16_bit.compare_capture_channel[0] = 0x8000;
	config_tc7.channel_pwm_out_enabled[0] = true; 
	config_tc7.channel_pwm_out_pin[0]     = PIN_PB22F_TC7_WO0;
	config_tc7.channel_pwm_out_mux[0]     = MUX_PB22F_TC7_WO0;	
	//config_tc7.waveform_invert_output     = TC_WAVEFORM_INVERT_OUTPUT_CHANNEL_0;
	
	config_tc7.size_specific.size_16_bit.compare_capture_channel[1] = 0x8000;
	config_tc7.channel_pwm_out_enabled[1] = true;
	config_tc7.channel_pwm_out_pin[1]     = PIN_PB23F_TC7_WO1;
	config_tc7.channel_pwm_out_mux[1]     = MUX_PB23F_TC7_WO1;
	//config_tc7.waveform_invert_output     = TC_WAVEFORM_INVERT_OUTPUT_CHANNEL_1;

	tc_init(&tc7_instance, TC7, &config_tc7);
}

void configure_tc7_callbacks(void)
{
	tc_register_callback(&tc7_instance,	tc7_callback_to_change_duty_cycle,	TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc7_instance, TC_CALLBACK_CC_CHANNEL0);
}

