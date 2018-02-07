/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "i2cProtocol.h"
#include "extern.h"

/************************************************************************/
/* Variables                                                            */
/************************************************************************/


/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
void userTimerStart(struct user_timer *user_timer_inst, uint16_t mins)
{
	user_timer_inst->durationMins = mins;
	user_timer_inst->timeoutTimestampInSecs = presentTimeInSec() + user_timer_inst->durationMins*60;
	user_timer_inst->startTimestampInSecs = presentTimeInSec();

	user_timer_inst->timerActive = true;
	user_timer_inst->timerExpired = false;
	user_timer_inst->presentCountInSecs = 0;
}

void userTimerUpdateStatus(struct user_timer *user_timer_inst)
{
	if (user_timer_inst->timerActive == true)
	{
		if (user_timer_inst->timerExpired == false)
		{
			user_timer_inst->presentCountInSecs = presentTimeInSec() - user_timer_inst->startTimestampInSecs;
		}
		if (presentTimeInSec() >= user_timer_inst->timeoutTimestampInSecs)
		{
			user_timer_inst->timerExpired = true;
		}
	}
}

void userTimerReset(struct user_timer *user_timer_inst)
{
	user_timer_inst->timerActive = false;
	user_timer_inst->timerExpired = false;
	user_timer_inst->presentCountInSecs = 0;
}