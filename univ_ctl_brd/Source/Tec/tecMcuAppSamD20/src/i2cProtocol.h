/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "app.h"

/************************************************************************/
/* Enum                                                             */
/************************************************************************/
typedef enum {
	Sanyo_9ga0912p4g03,
	Sanyo_9s0912p4f011,
	Sanyo_9ga0912p4j03,
	Nidec_v12e12bmm9 } FanPartNumber;
	
typedef enum {
	Phononic_mw1000_48,
	Phononic_mw500_41,
	Phononic_ma350_36 } PsPartNumber;	

/************************************************************************/
/* Structures                                                           */
/************************************************************************/
struct _varInfo
{
	void * pAddr;
	uint8_t size;
	bool nonvolatileFlag;
};

struct pid_instance
{
	float kp;                  // * (P)roportional Tuning Parameter
	float ki;                  // * (I)ntegral Tuning Parameter
	float kd;                  // * (D)erivative Tuning Parameter
	
	float dispKp;				// display values
	float dispKi;
	float dispKd;

	int controllerDirection;

	float Input;
	float Output;
	float Setpoint;
	float error;
	float dInput;
	
	//unsigned long lastTime;
	float PTerm;
	float ITerm;
	float DTerm;
	float lastInput;

	unsigned long SampleTimeMsec;
	float outMin;
	float outMax;	
	bool inAuto;
	
	float kp_highq;
	float ki_highq;
	float kd_highq;
	float kp_varq;
	float ki_varq;
	float kd_varq;
	float kp_copq;
	float ki_copq;
	float kd_copq;	
};

struct chamber_vars
{
	uint8_t mode_state;
	uint8_t previousModeState;
	float chamberTemp;
	float chamberTempRaw;
	float rejectTemp;
	float coldTemp;
	float deltaT;
	float setpoint;
	float previousSetpoint;
	float VmaxCOP;
	float Vvarq;
	float rejectLimit;
	float rejectLimitCritical;
	float rejectLowLimit;
	float deadband;
	float deadbandLow;
	uint8_t operatingMode;
	bool deactivateAutoControl;
	float chamberOffset;
	float heatLeak;
	float Qc;
	float COP;
	float glycolTemp;
	float glycolOffset;
};

struct ps_vars
{
	bool bulkPwmState;
	bool boostPwmState;
	uint8_t outputStatus;
	uint32_t bulkPwm;
	uint32_t bulkPwmTarget;
	uint32_t boostPwm;
	uint32_t boostPwmTarget;
	float desiredVoltage;
	float previousDesiredVoltage;
	float phpVoltage;
	float outputPower;
	float outputCurrent;
	float previousOutputCurrent;
	uint8_t outputDuty;
	float phpOutputDutyOffset;
	float inputPower;
	float efficiency;
	float bulkVoltage;
	float logicVoltage;
	float standbyVoltage;
	float batteryVoltage;
	PsPartNumber PsType;
	float minVoltage;
	float maxVoltage;
};

/*
struct php_vars
{
	float Qc;
	float COP;
	float deltaT;
	float Tcold;
	float Thot;
	float W;
	float V;
	float I;
};
*/

struct fan_vars
{
	uint8_t fan_id;
	uint32_t speedPwmDutyRegVal;	
	//float Vtop;
	//float Vbottom;
	float minDuty;		// percentage, i.e. 60.2%
	float maxDuty;		// percentage, i.e. 60.2%
	uint32_t rpm;
	uint8_t k;			// constant for calculating rpm; k=(m*(n-1))/p
	bool state;
	uint32_t targetDutyPercent;
	uint8_t emc2305_fan_config;
	uint8_t emc2305_fan_range;
	bool stall;
	uint8_t fan_state;
	uint8_t lim1;
	uint8_t lim1p;
	uint8_t spd1;
	uint8_t lim2;
	uint8_t lim2p;	
	uint8_t spd2;	
	uint8_t lim3;
	uint8_t lim3p;
	uint8_t spd3;		
	uint8_t lim4;
	uint8_t lim4p;
	uint8_t spd4;	
	uint8_t lim5;
	uint8_t lim5p;	
	uint8_t spd5;
	uint8_t spd6;
	uint8_t intDutyPercent;
	uint8_t prevSpdSetting;
	float rc;
	uint16_t maxRpm;
	float fanRpmSetting;	
	float fanSpdUprLim; 
	float fanSpdLwrLim; 
	FanPartNumber fanType;
};

struct heater_vars
{
	bool output_state;
	uint32_t duty;
	uint8_t sm_state;
	uint8_t prev_sm_state;
};

struct user_timer
{
	uint16_t durationMins;
	uint32_t timeoutTimestampInSecs;	// in seconds
	uint32_t startTimestampInSecs;		// in seconds
	bool timerActive;
	bool timerExpired;
	uint32_t presentCountInSecs;		// in seconds
};
