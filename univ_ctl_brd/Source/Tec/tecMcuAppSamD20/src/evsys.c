/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "extern.h"

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
#ifdef ENABLE_SLEEP_WAKE
void configure_event_sys(void)
{	
	// configure the event system
	events_init();	
	/*
	// Configure event system user. Event channel 0 
	struct events_user_config event_user_conf;
	events_user_get_config_defaults(&event_user_conf);
	event_user_conf.event_channel_id = EVENT_CHANNEL_0;
	events_user_set_config(EVSYS_ID_USER_TC4_EVU, &event_user_conf);
	// Setup PA22 (TACH0) as input to event system channel 0, asynchronous 
	*/
	
	struct events_chan_config events_ch0_conf;	
	events_chan_get_config_defaults(&events_ch0_conf);
	events_ch0_conf.generator_id = EVSYS_ID_GEN_EIC_EXTINT_6; 
	events_chan_set_config(EVENT_CHANNEL_0, &events_ch0_conf);	
	
	/*
	// Configure event system user. Event channel 1 
	events_user_get_config_defaults(&event_user_conf);
	event_user_conf.event_channel_id = EVENT_CHANNEL_1;
	events_user_set_config(EVSYS_ID_USER_TC5_EVU, &event_user_conf);
	// Setup PA23 (TACH1) as input to event system channel 0, asynchronous 
	struct events_chan_config events_ch1_conf;
	events_chan_get_config_defaults(&events_ch1_conf);
	events_ch1_conf.generator_id = EVSYS_ID_GEN_EIC_EXTINT_7;
	events_chan_set_config(EVENT_CHANNEL_1, &events_ch1_conf);
	*/		
}
#endif