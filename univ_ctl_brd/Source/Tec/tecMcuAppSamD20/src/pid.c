/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"
#include "dbgCmds.h"
#include "i2cProtocol.h"
#include "extern.h"

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
bool pidSetup(struct pid_instance *pid_inst)
{
	// define pid functions
	fPid[0] = set_setKP_HIGHQ;
	fPid[1] = set_setKI_HIGHQ;
	fPid[2] = set_setKD_HIGHQ;
	fPid[3] = set_setKP_VARQ;
	fPid[4] = set_setKI_VARQ;
	fPid[5] = set_setKD_VARQ;
	fPid[6] = set_setKP_COPQ;
	fPid[7] = set_setKI_COPQ;
	fPid[8] = set_setKD_COPQ;

	pid_inst->inAuto = true;
	pid_inst->SampleTimeMsec = RTC_PERIOD_IN_MSEC;
	pidSetControllerDirection(pid_inst, REVERSE);
	pidSetOutputLimits(pid_inst, 0, maxOutputVoltage);
	
	pid_inst->kp_highq = KP_HIGHQ;
	pid_inst->ki_highq = KI_HIGHQ;
	pid_inst->kd_highq = KD_HIGHQ;
	pid_inst->kp_varq = KP_VARQ;
	pid_inst->ki_varq = KI_VARQ;
	pid_inst->kd_varq = KD_VARQ;
	pid_inst->kp_copq = KP_COPQ;
	pid_inst->ki_copq = KI_COPQ;
	pid_inst->kd_copq = KD_COPQ;
	
	pidSetTunings(pid_inst, pid_inst->kp_highq, pid_inst->ki_highq, pid_inst->kd_highq);
	
	pwrCapTimeoutInMins = DEFAULT_PWR_CAP_TIMEOUT_MINS;
	
	return true;
}

bool pidFanSetup(struct pid_instance *pid_inst, float kp, float ki, float kd)
{
	pid_inst->inAuto = true;
	pid_inst->SampleTimeMsec = RTC_PERIOD_IN_MSEC;
	pidSetControllerDirection(pid_inst, REVERSE);
	pidSetOutputLimits(pid_inst, 0, DEFAULT_FAN_MAX_DUTY);
	
	pid_inst->kp = kp;
	pid_inst->ki = ki;
	pid_inst->kd = kd;
	
	pidSetTunings(pid_inst, pid_inst->kp, pid_inst->ki, pid_inst->kd);
	
	return true;
}

bool pidCompute(struct pid_instance *pid_inst)
{
	if(!pid_inst->inAuto) return false;
	
	pid_inst->error = pid_inst->Setpoint - pid_inst->Input;

	pid_inst->PTerm = pid_inst->kp * pid_inst->error;
	
	if ( (pid_inst->PTerm < pid_inst->outMax) && (pid_inst->PTerm > pid_inst->outMin) )
	{
		pid_inst->ITerm += (pid_inst->ki * pid_inst->error);
	} 
	else 
	{
		if (chamber1.setpoint > chamber1.previousSetpoint) 
		{
			pid_inst->ITerm = 0;
			chamber1.previousSetpoint = chamber1.setpoint;
		}
		//else if (chamber1.setpoint < chamber1.previousSetpoint) 
		//{
		//	ITerm = 0;
		//	chamber1.previousSetpoint = chamber1.setpoint;
		//}
	}
	
	if(pid_inst->ITerm > pid_inst->outMax) pid_inst->ITerm = pid_inst->outMax;
	else if(pid_inst->ITerm < pid_inst->outMin) pid_inst->ITerm = pid_inst->outMin;

	pid_inst->dInput = (pid_inst->Input - pid_inst->lastInput);
	pid_inst->DTerm = pid_inst->kd * pid_inst->dInput * (-1);
	
	pid_inst->Output = pid_inst->PTerm + pid_inst->ITerm + pid_inst->DTerm;
	
	if(pid_inst->Output > pid_inst->outMax) pid_inst->Output = pid_inst->outMax;
	else if(pid_inst->Output < pid_inst->outMin) pid_inst->Output = pid_inst->outMin;
	
	pid_inst->lastInput = pid_inst->Input;
	
	return true;
}

void pidSetOutputLimits(struct pid_instance *pid_inst, float Min, float Max)
{
	// prevent interruption of changes
	rtc_calendar_disable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);
	
	if(Min >= Max) return;
	pid_inst->outMin = Min;
	pid_inst->outMax = Max;
	
	if(pid_inst->inAuto)
	{
		if(pid_inst->Output > pid_inst->outMax) pid_inst->Output = pid_inst->outMax;
		else if(pid_inst->Output < pid_inst->outMin) pid_inst->Output = pid_inst->outMin;
		
		if(pid_inst->ITerm > pid_inst->outMax) pid_inst->ITerm = pid_inst->outMax;
		else if(pid_inst->ITerm < pid_inst->outMin) pid_inst->ITerm = pid_inst->outMin;
	}
	
	rtc_calendar_enable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);
}

void pidSetTunings(struct pid_instance *pid_inst, float Kp, float Ki, float Kd)
{
	// prevent interruption of changes
	rtc_calendar_disable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);

	if (Kp<0 || Ki<0 || Kd<0) return;
	
	pid_inst->dispKp = Kp;
	pid_inst->dispKi = Ki;
	pid_inst->dispKd = Kd;
	
	// calculate ideal parallel form constants
	pid_inst->kp = Kp;
	pid_inst->ki = Ki * pid_inst->SampleTimeMsec;	// Ki = Kp/Ti in standard form
	pid_inst->kd = Kd / pid_inst->SampleTimeMsec;	// Kd = Kp*Td in standard form
	
	if(pid_inst->controllerDirection == REVERSE) {
		pid_inst->kp = (0 - pid_inst->kp);
		pid_inst->ki = (0 - pid_inst->ki);
		pid_inst->kd = (0 - pid_inst->kd);
	}

	rtc_calendar_enable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);
}

void pidSetControllerDirection(struct pid_instance *pid_inst, int Direction)
{
	// prevent interruption of changes
	rtc_calendar_disable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);
	
	if(pid_inst->inAuto && Direction != pid_inst->controllerDirection)
	{
		pid_inst->kp = (0 - pid_inst->kp);
		pid_inst->ki = (0 - pid_inst->ki);
		pid_inst->kd = (0 - pid_inst->kd);
	}
	pid_inst->controllerDirection = Direction;

	rtc_calendar_enable_callback(RTC_CALENDAR_CALLBACK_ALARM_0);
}