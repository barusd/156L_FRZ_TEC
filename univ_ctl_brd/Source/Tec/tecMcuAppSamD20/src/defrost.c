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
uint16_t defrostCycleCount = 0;
bool defrostActive = false;
bool enableAutoDefrost = true;
uint32_t task4Time = 0;
uint32_t task5Time = 0;
uint32_t task4LastTime = 0;
uint32_t task5LastTime = 0;
uint8_t frz_up_activate_cnt = 0;
uint8_t frz_up_deactivate_cnt = 0;
struct rtc_calendar_alarm_time last_defrost_timestamp;
float defrostCancelTemp;
struct user_timer defrostFreezupTimer;
struct user_timer defrostIntervalTimer;
struct user_timer defrostMinimumIntervalTimer;
struct user_timer defrostMaximumIntervalTimer;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
void initDefrost()
{
	userTimerReset(&defrostIntervalTimer);
	userTimerReset(&defrostFreezupTimer);
	userTimerStart(&defrostMinimumIntervalTimer, defrostFreqHrs*60);
	userTimerStart(&defrostMaximumIntervalTimer, defrostFreqMaxHrs*60);

	//defrostFreqHrs = DEFAULT_DEFROST_FREQ_HRS;
	//defrostDurationMins = DEFAULT_DEFROST_DURATION_MINS;
	defrostCycleCount = 0;
	defrostActive = false;
	enableAutoDefrost = true;
	task4LastTime = presentTimeInSec();
	task5LastTime = presentTimeInSec();
	task4Time = 0;
	task5Time = 0;
	defrostCancelTemp = maxAllowedChamberTemp-1;
}

void setDefrostState(uint8_t state)
{
	if (state == DEFROST_START)
	{
		// start defrost cycle
		defrostActive = true;
		///task5LastTime = presentTimeInSec();
		userTimerReset(&defrostIntervalTimer);
		userTimerStart(&defrostIntervalTimer, defrostDurationMins);
		chamber1.deactivateAutoControl = true;
		ps1.desiredVoltage = 0;
		pidLoop1.inAuto = false;
		//enableFanOverride = true;
		//fan1.targetDutyPercent = 100;
		//fan2.targetDutyPercent = 100;
		defrostCycleCount++;
			
		rtc_calendar_get_time(&time_report.time);
		dbg_info("\n[defrost start %2dh:%2dm:%2ds]",
			time_report.time.hour,
			time_report.time.minute,
			time_report.time.second);	
		
		last_defrost_timestamp = time_report;
	}
	else if (state == DEFROST_STOP)
	{
		// end defrost cycle
		defrostActive = false;
		///task5Time = 0;
		userTimerReset(&defrostIntervalTimer);
		chamber1.deactivateAutoControl = false;
		pidLoop1.inAuto = true;
		//enableFanOverride = false;
			
		rtc_calendar_get_time(&time_report.time);
		dbg_info("\n[defrost stop %2dh:%2dm:%2ds]",
			time_report.time.hour,
			time_report.time.minute,
			time_report.time.second);
		dbg_info(" [cycle %d]\n", defrostCycleCount);		
	}
	else if (state == DEFROST_CANCEL)
	{
		defrostActive = false;
		chamber1.deactivateAutoControl = false;
		pidLoop1.inAuto = true;
		///task4Time = 0;
		///task4LastTime = presentTimeInSec();
		///task5Time = 0;
		///task5LastTime = presentTimeInSec();
		userTimerReset(&defrostIntervalTimer);
		dbg("\n[defrost cancelled]");
	}		
}

void defrostStateMachine()
{
	if (enableAutoDefrost==true) 
	{	
		#ifdef HIGH_PERFORMANCE
		// START ///////////////////////////////////////////////
		if ( ( (defrostMinimumIntervalTimer.timerExpired==true) && (highUsageModeFlag==true) ) ||
			 ( (defrostMinimumIntervalTimer.timerExpired==true) && 
			   ( (defrostFreezupTimer.timerExpired==true) ||
			     ((defrostMaximumIntervalTimer.timerExpired==true) && (highUsageModeFlag==false)) 
			   ) 
			 )
		   )
		{
			setDefrostState(DEFROST_START);
			//userTimerStart(&defrostIntervalTimer, defrostDurationMins);
			userTimerReset(&defrostMinimumIntervalTimer);
			userTimerReset(&defrostMaximumIntervalTimer);
			userTimerReset(&defrostFreezupTimer);	
		}
	
		if (acceptHtxTemperature < DEFROST_ACTIVATE_TEMPERATURE)
		{
			if (defrostFreezupTimer.timerActive==false)
			{
				frz_up_activate_cnt++;
				if (frz_up_activate_cnt > FRZ_UP_ACTIVATE_CNT_LIMIT)
				{
					frz_up_activate_cnt = 0;
					userTimerStart(&defrostFreezupTimer, DEFROST_FREEZEUP_TIMEOUT_IN_MINS);
				}
			}
			else
			{
				frz_up_activate_cnt = 0;
			}
		}
	
		// STOP ////////////////////////////////////////////////
		if (defrostIntervalTimer.timerExpired==true)
		{
			setDefrostState(DEFROST_STOP);
			//userTimerReset(&defrostIntervalTimer);
			userTimerStart(&defrostMinimumIntervalTimer, defrostFreqHrs*60);
			userTimerStart(&defrostMaximumIntervalTimer, defrostFreqMaxHrs*60);
		}

		// CANCEL //////////////////////////////////////////////
		if (acceptHtxTemperature > DEFROST_DEACTIVATE_TEMPERATURE)
		{
			if (defrostFreezupTimer.timerActive==true)
			{
				frz_up_deactivate_cnt++;
				if (frz_up_deactivate_cnt > FRZ_UP_DEACTIVATE_CNT_LIMIT)
				{
					frz_up_deactivate_cnt = 0;
					userTimerReset(&defrostFreezupTimer);
				}
			}
			else
			{
				frz_up_deactivate_cnt = 0;
			}
		}
		
		if ( (controlTemperature > defrostCancelTemp) && (defrostIntervalTimer.timerActive == true) )
		{
			setDefrostState(DEFROST_CANCEL);
			//userTimerReset(&defrostIntervalTimer);
			userTimerStart(&defrostMinimumIntervalTimer, defrostFreqHrs*60);
			userTimerStart(&defrostMaximumIntervalTimer, defrostFreqMaxHrs*60);
		}
				
		// CALCULATE TIME /////////////////////////////////////
		userTimerUpdateStatus(&defrostFreezupTimer);
		userTimerUpdateStatus(&defrostMinimumIntervalTimer);
		userTimerUpdateStatus(&defrostMaximumIntervalTimer);
		userTimerUpdateStatus(&defrostIntervalTimer);
		#endif
	}
}
