/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "extern.h"

/************************************************************************/
/* Module Instances                                                     */
/************************************************************************/
struct adc_module adc_inst;
uint16_t adc_buf[ADC_SAMPLES];

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
volatile bool interrupt_flag = false;
volatile bool adc_init_success = false;

/************************************************************************/
/* Callback Functions                                                   */
/************************************************************************/
void adc_user_callback(const struct adc_module *const module)
{
	interrupt_flag = true;
}

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
void configure_adc(uint8_t adcRef)
{
	enum status_code status = STATUS_ERR_IO;

	// Disable ADC before initialization
	//adc_disable(&adc_inst);
	
	/* Structure for ADC configuration */
	struct adc_config config;
	adc_get_config_defaults(&config);
	config.positive_input = ADC_POSITIVE_INPUT_PIN0;
	config.negative_input = ADC_NEGATIVE_INPUT_GND;
	config.reference      = adcRef;
	config.clock_source   = GCLK_GENERATOR_3;
	config.gain_factor    = ADC_GAIN_FACTOR_1X;
	config.resolution         = ADC_RESOLUTION_CUSTOM;
	config.accumulate_samples = ADC_ACCUMULATE_SAMPLES_16;	
	config.divide_result	  = ADC_DIVIDE_RESULT_16;

	/* Initialize the ADC */
	status = adc_init(&adc_inst, ADC, &config);

	/* Check for successful initialization */
	//test_assert_true(test, status == STATUS_OK,
	//"ADC initialization failed");

	/* Enable the ADC */
	status = adc_enable(&adc_inst);

	/* Check for successful enable */
	//test_assert_true(test, status == STATUS_OK,
	//"ADC enabling failed");

	if (status == STATUS_OK) {
		adc_init_success = true;
	}
}
/**
 * \internal
 * \brief Test for ADC initialization.
 *
 * This test initializes the ADC module and checks whether the
 * initialization is successful or not.
 *
 * If this test fails no other tests will run.
 *
 * \param test Current test case.
 */
/*
static void run_adc_init_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	// Structure for ADC configuration 
	struct adc_config config;
	adc_get_config_defaults(&config);
	config.positive_input = ADC_POSITIVE_INPUT_PIN0;
	config.negative_input = ADC_NEGATIVE_INPUT_GND;
	config.reference      = ADC_REF_VOLTAGE; 
	
	config.clock_source   = GCLK_GENERATOR_3;
	config.gain_factor    = ADC_GAIN_FACTOR_1X;

	// Initialize the ADC 
	status = adc_init(&adc_inst, ADC, &config);

	// Check for successful initialization 
	test_assert_true(test, status == STATUS_OK,
			"ADC initialization failed");

	// Enable the ADC 
	status = adc_enable(&adc_inst);

	// Check for successful enable 
	test_assert_true(test, status == STATUS_OK,
			"ADC enabling failed");

	if (status == STATUS_OK) {
		adc_init_success = true;
	}
}
*/
/**
 * \internal
 * \brief Setup Function: ADC average mode test.
 *
 * This function initializes the ADC in averaging mode.
 *
 * \param test Current test case.
 */
/*
static void setup_adc_average_mode_test(const struct test_case *test)
{
	enum status_code status = STATUS_ERR_IO;

	// Skip test if ADC initialization failed 
	test_assert_true(test, adc_init_success,
			"Skipping test due to failed initialization");

	// Disable ADC before initialization 
	adc_disable(&adc_inst);
	struct adc_config config;
	adc_get_config_defaults(&config);
	config.positive_input     = ADC_POSITIVE_INPUT_PIN0;
	config.negative_input     = ADC_NEGATIVE_INPUT_GND;
	config.reference          = ADC_REF_VOLTAGE;
	config.clock_source       = GCLK_GENERATOR_3;
	config.gain_factor        = ADC_GAIN_FACTOR_1X;
	config.resolution         = ADC_RESOLUTION_16BIT;
	config.accumulate_samples = ADC_ACCUMULATE_SAMPLES_16;

	// Re-initialize & enable ADC 
	status = adc_init(&adc_inst, ADC, &config);
	test_assert_true(test, status == STATUS_OK,
			"ADC initialization failed");
	status = adc_enable(&adc_inst);
	test_assert_true(test, status == STATUS_OK,
			"ADC enabling failed");
}
*/

/**
 * \internal
 * \brief ADC average mode test function
 *
 * This test performs the ADC averaging by starting a conversion.
 * 0.5V is applied as input from DAC.
 * Converted result is verified for expected results.
 *
 * \param test Current test case.
 */
/*
static void run_adc_average_mode_test(const struct test_case *test)
{
	uint16_t adc_result = 0;

	// Set 0.5V DAC output 
	//dac_chan_write(&dac_inst, DAC_CHANNEL_0, DAC_VAL_HALF_VOLT);
	delay_ms(1);

	// Start an ADC conversion 
	adc_start_conversion(&adc_inst);
	while (adc_read(&adc_inst, &adc_result) != STATUS_OK) {
	}
	adc_result = adc_result >> 4;

	// Test result 
	test_assert_true(test,
			(adc_result > (ADC_VAL_DAC_HALF_OUTPUT - ADC_OFFSET)) &&
			(adc_result < (ADC_VAL_DAC_FULL_OUTPUT - ADC_OFFSET)),
			"Error in ADC average mode conversion at 0.5V input");
}
*/
