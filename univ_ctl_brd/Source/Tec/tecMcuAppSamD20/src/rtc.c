/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "i2cProtocol.h"
#include "extern.h"

/************************************************************************/
/* Module Instances                                                     */
/************************************************************************/
struct rtc_calendar_alarm_time time_report;
struct rtc_calendar_time time;
struct rtc_calendar_time currentTime;

/************************************************************************/
/* Callback Functions                                                   */
/************************************************************************/
void rtc_alarm_match_callback(void)
{
	// compute pid variables on periodic interval
	pidCompute(&pidLoop1);

	/*
	if (max(chamber1.rejectTemp, chamber2.rejectTemp) > STEADY_STATE_REJECT_TEMP)
	{
		pidCompute(&pidFan1);
		pidCompute(&pidFan2);
	}
	else
	{
		pidFan1.PTerm = 0;		pidFan2.PTerm = 0;
		pidFan1.ITerm = 0;		pidFan2.ITerm = 0;
		pidFan1.error = 0;		pidFan2.error = 0;
		pidFan1.Output = 0;		pidFan2.Output = 0;
	}
	*/	
	if (chamberTempDifference > chamberTempUniformityTarget)
	{
		pidCompute(&pidFan4);
	}
	else
	{
		pidFan4.PTerm = 0;
		pidFan4.ITerm = 0;
		pidFan4.error = 0;
		pidFan4.Output = 0;
	}
	
	/* Set new alarm */
	struct rtc_calendar_alarm_time alarm;
	rtc_calendar_get_time(&alarm.time);	
	alarm.mask = RTC_CALENDAR_ALARM_MASK_SEC;
	alarm.time.second += RTC_PERIOD_IN_MSEC / 1000;
	alarm.time.second = alarm.time.second % 60;
	rtc_calendar_set_alarm(&alarm, RTC_CALENDAR_ALARM_0);
}

/************************************************************************/
/* Configuration                                                        */
/************************************************************************/
void configure_rtc_calendar(void)
{
	/* Initialize RTC in calendar mode. */
	struct rtc_calendar_config config_rtc_calendar;
	rtc_calendar_get_config_defaults(&config_rtc_calendar);
	config_rtc_calendar.clock_24h = true;
	
	// build alarm0 settings
	struct rtc_calendar_time alarm;
	rtc_calendar_get_time_defaults(&alarm);
	alarm.year = 2016;
	alarm.month = 1;
	alarm.day = 1;
	alarm.hour = 0;
	alarm.minute = 0;
	alarm.second = 4;
	
	// setup alarm
	config_rtc_calendar.alarm[0].time = alarm;
	config_rtc_calendar.alarm[0].mask = RTC_CALENDAR_ALARM_MASK_YEAR;
	
	rtc_calendar_init(&config_rtc_calendar);
	rtc_calendar_enable();
}

void configure_rtc_callbacks(void)
{
	rtc_calendar_register_callback(rtc_alarm_match_callback, RTC_CALENDAR_CALLBACK_ALARM_0);
	rtc_calendar_enable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);
}

