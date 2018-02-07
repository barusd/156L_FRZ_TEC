/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "extern.h"

/************************************************************************/
/* Module Instances                                                     */
/************************************************************************/
struct usart_module cdc_uart_module;

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];
volatile uint8_t command[MAX_RX_BUFFER_LENGTH];
bool cmdFlag = false;

/************************************************************************/
/* Callback Functions                                                   */
/************************************************************************/
void usart_read_callback(const struct usart_module *const usart_module)
{
	uint8_t i = 0;
	cmdFlag = false;
	uint8_t nLength = 0;
	uint8_t nStrEnd = 0;
	const char* strSearch = NULL;
	
	nLength = strlen((const char*)rx_buffer);
	strSearch = strchr((const char*)rx_buffer, '\n');
	if (strSearch != NULL)
	{
		nStrEnd = strSearch - (const char*) rx_buffer;
	}
	command[0] = 0;
	// valid command
	if ((nLength-1) == nStrEnd ) {
		//dbg("\n[command received]");
		command[0] = nLength;
		for(i=0; i<nLength; i++) {
			command[i+1] = rx_buffer[i];
		}
		cmdFlag = true;
		//dbg_info("\n%s", command);
		for(i=0; i<MAX_RX_BUFFER_LENGTH; i++) {
			rx_buffer[i] = 0;
		}
		
	// invalid command
	} else {
		dbg_info("\n[invalid command %d, %d, %d]", rx_buffer[0], rx_buffer[1], rx_buffer[2]);
		for(i=0; i<MAX_RX_BUFFER_LENGTH; i++) {
			rx_buffer[i] = 0;
		}
	}
	
}

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/

/**
 * \brief Initialize the USART for unit test
 *
 * Initializes the SERCOM USART (SERCOM4) used for sending the
 * unit test status to the computer via the EDBG CDC gateway.
 */
void cdc_uart_init(void)
{
	struct usart_config usart_conf;

	/* Configure USART for unit test output */
	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = CONF_STDIO_MUX_SETTING;
	usart_conf.pinmux_pad0 = CONF_STDIO_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = CONF_STDIO_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = CONF_STDIO_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = CONF_STDIO_PINMUX_PAD3;
	usart_conf.baudrate    = 115200;

	stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART, &usart_conf);
	usart_enable(&cdc_uart_module);
	
	usart_enable_transceiver(&cdc_uart_module, USART_TRANSCEIVER_TX);
	usart_enable_transceiver(&cdc_uart_module, USART_TRANSCEIVER_RX);
}

void configure_usart_callbacks(void)
{
	//usart_register_callback(&cdc_uart_module,
	//		usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&cdc_uart_module,
			usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
	//usart_enable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_RECEIVED);
}

