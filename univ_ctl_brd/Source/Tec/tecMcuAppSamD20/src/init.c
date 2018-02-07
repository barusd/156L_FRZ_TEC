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
uint8_t runSelfTestOnPOR = true;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
void initMain()
{
	system_interrupt_disable_global();
	set_fuses();
	system_init();
	configure_port_pins();
	cdc_uart_init();
	configure_usart_callbacks();
	
	rtc_calendar_get_time_defaults(&time);
	time.year = 2016;
	time.month = 1;
	time.day = 1;
	time.hour = 0;
	time.minute = 0;
	time.second = 0;
	configure_rtc_calendar();		// configure and enable RTC
	configure_rtc_callbacks();		// configure and enable callback
	rtc_calendar_set_time(&time);	// set current time
	
	delay_init();

	// tc0 cmp1 is php output3
	configure_tc0();
	//tc_enable(&tc0_module);
	configure_tc0_callbacks();
	tc_enable_callback(&tc0_module, TC_CALLBACK_CC_CHANNEL1);
	
	// tc2 cmp1 is boost pwm out
	// tc2 cmp2 is bulk pwm out
	configure_tc2();
	configure_tc2_callbacks();

	// tc4 cap1 is fan1 tachometer capture
	//configure_tc4();
	//configure_tc4_callbacks();
	
	// tc5 cmp0 is php output1
	configure_tc5();
	//tc_enable(&tc5_module);
	configure_tc5_callbacks();
	tc_enable_callback(&tc5_module, TC_CALLBACK_CC_CHANNEL0);
	
	// tc7 cmp1 is heater1 pwm out
	// tc7 cmp2 is heater2 pwm out
	configure_tc7();
	tc_enable(&tc7_instance);
	configure_tc7_callbacks();
	
	#ifdef ENABLE_SLEEP_WAKE
	configure_extint();
	configure_extint_callbacks();
	configure_event_sys();
	#endif
	
	system_interrupt_disable(EIC_IRQn);
	system_interrupt_enable_global();

	// initialize adc
	configure_adc(ADC_REFERENCE_AREFA);
		
	delay_ms(50);
	dbg("\n\n[system reset detected]");
	//dbg("\n[peripheral init...");
	delay_ms(50);

	uiSetpointControl = true;

	pidSetup(&pidLoop1);
	initChamber(&chamber1);
	initChamber(&chamber2);
	initPowerSupply(&ps1);
	
	enableFanOverride = false;
	
	#ifdef HIGH_PERFORMANCE
	fan1.fanType = Sanyo_9ga0912p4g03;
	fan2.fanType = Nidec_v12e12bmm9;
	fan5.fanType = Nidec_v12e12bmm9;
	fan6.fanType = Nidec_v12e12bmm9;
	fan3.fanType = Nidec_v12e12bmm9;
	fan4.fanType = Sanyo_9ga0912p4g03;
	fan7.fanType = Nidec_v12e12bmm9;
	fan8.fanType = Nidec_v12e12bmm9;		
	#endif

	initFan(&fan1);
	initFan(&fan2);
	initFan(&fan3);
	pidFanSetup(&pidFan4, KP_FAN4, KI_FAN1, KD_FAN4);
	initFan(&fan4);
	initFan(&fan5);
	initFan(&fan6);
	initFan(&fan7);
	initFan(&fan8);
	port_pin_set_output_level(PIN_ENBL_PSFAN, HIGH);
	
	initDbgCtrlFunctions();
		
	condHeater.sm_state = STATE_CONDHTR_OFF;
	setHeaterState(&htxAHeater, 0);
	setHeaterState(&htxBHeater, 0);
	setHeaterState(&condHeater, 0);
	setHeaterState(&doorHeater, 0);
		
	// initialize globals to defaults
	firmwareVersion = FIRMWARE_VERSION;
	firmwareRevision = FIRMWARE_REVISION;
	defrostFreqMaxHrs = DEFROST_MAX_INTERVAL_TIME_IN_HRS;
	i2cFunctionReg = 0;
	errorRegister = ERROR_NONE;
	chamberThermistorBeta = BCOEFFICIENT_TCHAMBER;
	rejectThermistorBeta = BCOEFFICIENT_TREJ;
		
	// configure eeprom emulator
	configure_eeprom();
	struct eeprom_emulator_parameters eeprom_param;
	eeprom_emulator_get_parameters(&eeprom_param);
	eepromReadParams();

	//initDefrost();
	//initPhpSelfCheck();
	
	printBootMsg();

	//detectPHP();
	phpDetected = true;

	for(int i=0; i<MAX_RX_BUFFER_LENGTH; i++) {rx_buffer[i] = 0; }

	task1LastTime = presentTimeInSec();
	task2LastTime = presentTimeInSec();
	task3LastTime = presentTimeInSec();
	// task4 used for defrost
	// task5 used for defrost
	task6LastTime = presentTimeInSec();
	task7LastTime = presentTimeInSec();
	// task8 used for php self-check
	// task9 used for php self-check
	// task10 used for php self-check
	task11LastTime = presentTimeInSec();
	task12LastTime = presentTimeInSec();
	
	task1Time = 0;
	task2Time = 0;
	task3Time = 0;
	// task4 used for defrost
	// task5 used for defrost
	task6Time = 0;
	task7Time = 0;
	// task8 used for php self-check
	// task9 used for php self-check
	// task10 used for php self-check
	task11Time = 0;	
	task12Time = 0;
	
	configure_i2c_slave();
	configure_i2c_slave_callbacks();
	configure_i2c_master();
	
	REG_EIC_INTFLAG = 0x8000;			// clear interrupt flag
	system_interrupt_enable(EIC_IRQn);
	
	port_pin_set_output_level_EX(PIN_RDY_N, LOW, enableManualOverride);   // assert RDY pin
	port_pin_set_output_level_EX(PIN_LED_0_PIN, LOW, false);
	
	tc_enable(&tc0_module);
	tc_enable(&tc5_module);

	setOutputSwitch(PIN_ENBL_OUTA_COLD, GPIO_MODE, OFF);
	setOutputSwitch(PIN_ENBL_OUTA_HOT , GPIO_MODE, OFF);
	setOutputSwitch(PIN_ENBL_OUTB_COLD, GPIO_MODE, OFF);
	setOutputSwitch(PIN_ENBL_OUTB_HOT , GPIO_MODE, OFF);

	if (fanEmc2305Init(FAN_CHIP1) != ERROR_NONE)
	{
		dbg("\n...fan controller1 failure; system halted\n");
		//while (1) {};
	}
	
	if (fanEmc2305Init(FAN_CHIP2) != ERROR_NONE)
	{
		dbg("\n...fan controller2 failure; system halted\n");
		//while (1) {};
	}
	
	if (runSelfTestOnPOR > 0)
	{
		selfTest();
	}
	
	#ifdef ENABLE_DEBUG_CMD_INTERFACE
	enableDebugCmds = true;
	enablePsDbgMsgs = false;
	#else
	enableDebugCmds = false;
	enablePsDbgMsgs = true;
	#endif
	
	readSensors();
	controlTemperature = chamber2.chamberTemp;
	
	dbg("\n[system ready]\n");	
}

void initChamber(struct chamber_vars *ch_inst)
{
	ch_inst->mode_state = 0;
	ch_inst->previousModeState = 0;
	ch_inst->chamberTemp = 0;
	ch_inst->rejectTemp = 0;
	ch_inst->deltaT = 0;
	ch_inst->setpoint = SETPOINT_DEFAULT;
	ch_inst->previousSetpoint = SETPOINT_DEFAULT;
	ch_inst->VmaxCOP = VMAXCOP_DEFAULT;
	ch_inst->Vvarq = VARQ_VOLTAGE_DEFAULT;
	ch_inst->rejectLimit = REJECT_LIMIT_DEFAULT;
	ch_inst->rejectLimitCritical = REJECT_LIMIT_CRITICAL_DEFAULT;
	ch_inst->rejectLowLimit = REJECT_LIMIT_DEFAULT-REJECT_DEADBAND_DEFAULT;
	ch_inst->deadband = DEADBAND_DEFAULT;
	ch_inst->deadbandLow = DEADBANDLOW_DEFAULT;
	ch_inst->operatingMode = PID_MODE;
	ch_inst->deactivateAutoControl = false;
	ch_inst->chamberOffset = 0;
	ch_inst->heatLeak = HEATLEAK_DEFAULT;
	ch_inst->COP = 0;
	ch_inst->Qc = 0;
	ch_inst->glycolTemp = 0;
	ch_inst->glycolOffset = 0;
		
	return;
}

void initPowerSupply(struct ps_vars *ps_inst)
{
	// initialize variables
	ps_inst->phpVoltage = 0;
	ps_inst->desiredVoltage = 0;
	ps_inst->previousDesiredVoltage = 0;
	ps_inst->boostPwm = BOOST_OFF_DUTY;
	ps_inst->boostPwmTarget = BOOST_OFF_DUTY;
	ps_inst->outputStatus = OFF;

	if (ps_inst == &ps1)
	{
		ps_inst->minVoltage = PCP_COLD_STAGE_MIN_OUTPUT_VOLTAGE;
		ps_inst->maxVoltage = PCP_COLD_STAGE_MAX_OUTPUT_VOLTAGE;
	}
	else if (ps_inst == &ps2)
	{
		ps_inst->minVoltage = PCP_HOT_STAGE_MIN_OUTPUT_VOLTAGE;
		ps_inst->maxVoltage = PCP_HOT_STAGE_MAX_OUTPUT_VOLTAGE;
	}

	setBoostVoltage(ps_inst, BOOST_OFF_DUTY);
	ps_inst->boostPwmState = OFF;
	
	#ifdef HIGH_PERFORMANCE
	bulk_voltage_max = BULK_VOLTAGE_48V_MAX;
	bulk_voltage_nom = BULK_VOLTAGE_48V_NOM;
	bulk_voltage_min = BULK_VOLTAGE_48V_MIN;
	boost_idx_offset = 0;	// hp boost table starts at 48V
							// if ctl brd hw is permanently change to 36V scaling,
							// then this should be changed to match below
	#endif
		
	return;
}

void initFan(struct fan_vars *fan_inst)
{
	if (fan_inst->fanType==Sanyo_9ga0912p4g03)
	{
		fan_inst->maxRpm = fan_sanyo_9ga0912p4g03_rpm_lookup_table[FAN_RPM_LOOKUP_TABLE_SIZE-1];
	}
	else if (fan_inst->fanType==Sanyo_9s0912p4f011)
	{
		fan_inst->maxRpm = fan_sanyo_9s0912p4f011_rpm_lookup_table[FAN_RPM_LOOKUP_TABLE_SIZE-1];
	}
	else if (fan_inst->fanType==Sanyo_9ga0912p4j03)
	{
		fan_inst->maxRpm = fan_sanyo_9ga0912p4j03_rpm_lookup_table[FAN_RPM_LOOKUP_TABLE_SIZE-1];
	}
	else if (fan_inst->fanType==Nidec_v12e12bmm9)
	{
		fan_inst->maxRpm = fan_nidec_v12e12bmm9_rpm_lookup_table[FAN_RPM_LOOKUP_TABLE_SIZE-1];
	}
	fan_inst->speedPwmDutyRegVal = MAX_FAN_PWM_DUTY_REG_VAL;
	//fan_inst->Vtop = DEFAULT_FAN_VTOP;
	//fan_inst->Vbottom = DEFAULT_FAN_VTOP;
	fan_inst->minDuty = DEFAULT_FAN_MIN_DUTY;
	fan_inst->maxDuty = DEFAULT_FAN_MAX_DUTY;
	fan_inst->rpm = 0;
	fan_inst->state = OFF;
	fan_inst->targetDutyPercent = 100.0;
	fan_inst->rc = FAN_RESPONSE_TIMECONSTANT;
	fan_inst->fanRpmSetting = 0;
	fan_inst->fanSpdUprLim = DEFAULT_FAN_MAX_RPM;
	fan_inst->fanSpdLwrLim = 0;	
	fan_inst->maxRpm = DEFAULT_FAN_MAX_RPM;

	if		(fan_inst == &fan1) { fan_inst->fan_id = 1; }
	else if (fan_inst == &fan2) { fan_inst->fan_id = 2; }
	else if (fan_inst == &fan3) { fan_inst->fan_id = 3; }
	else if (fan_inst == &fan4) { fan_inst->fan_id = 4; }
	else if (fan_inst == &fan5) { fan_inst->fan_id = 5; }
	else if (fan_inst == &fan6) { fan_inst->fan_id = 6; }
	else if (fan_inst == &fan7) { fan_inst->fan_id = 7; }
	else if (fan_inst == &fan8) { fan_inst->fan_id = 8; }						

	fan_inst->spd1  =  fan_limit_table[fan_inst->fan_id-1][0];
	fan_inst->lim1p =  fan_limit_table[fan_inst->fan_id-1][1];
	fan_inst->lim1  =  fan_limit_table[fan_inst->fan_id-1][2];
	fan_inst->spd2  =  fan_limit_table[fan_inst->fan_id-1][3];
	fan_inst->lim2p =  fan_limit_table[fan_inst->fan_id-1][4];
	fan_inst->lim2  =  fan_limit_table[fan_inst->fan_id-1][5];
	fan_inst->spd3  =  fan_limit_table[fan_inst->fan_id-1][6];
	fan_inst->lim3p =  fan_limit_table[fan_inst->fan_id-1][7];
	fan_inst->lim3  =  fan_limit_table[fan_inst->fan_id-1][8];
	fan_inst->spd4  =  fan_limit_table[fan_inst->fan_id-1][9];
	fan_inst->lim4p =  fan_limit_table[fan_inst->fan_id-1][10];
	fan_inst->lim4  =  fan_limit_table[fan_inst->fan_id-1][11];
	fan_inst->spd5  =  fan_limit_table[fan_inst->fan_id-1][12];
	fan_inst->lim5p =  fan_limit_table[fan_inst->fan_id-1][13];
	fan_inst->lim5  =  fan_limit_table[fan_inst->fan_id-1][14];
	fan_inst->spd6  =  fan_limit_table[fan_inst->fan_id-1][15];
	fan_inst->k		=  fan_limit_table[fan_inst->fan_id-1][16];
		
	fan_inst->fan_state = FAN_STATE_S6;
}

void initDbgCtrlFunctions()
{
	// define chamber functions
	fChamber[0] = set_setPointValue;
	fChamber[1] = set_setRejectLimit;
	fChamber[2] = set_setRejectLimitCritical;
	fChamber[3] = set_setRejectDeadband;
	fChamber[4] = set_setDeadband;
	fChamber[5] = set_setDeadbandLow;
	fChamber[6] = set_setGlycolTemp;
	fChamber[7] = set_setChamberTemp;
	fChamber[8] = set_setRejectTemp;
	fChamber[9] = set_setGlycolOffset;
	fChamber[10] = set_setChamberOffset;
	fChamber[11] = set_setModeState;
	
	// define power supply functions
	fPower[0] = set_setVarqVoltage;
	fPower[1] = set_setVmaxCOP;
	fPower[2] = set_setMaxBulkPower;
	fPower[3] = readPhpVoltage;
	fPower[4] = readPhpCurrent;
	fPower[5] = readBulkVoltage;
	fPower[6] = set_setDesiredVoltage;
	fPower[7] = readLogicVoltage;
	fPower[8] = readStandbyVoltage;
	fPower[9] = readBatteryVoltage;
	fPower[10] = readPhpVoltage2;
	fPower[11] = readPhpCurrent2;
	fPower[12] = set_setEnablePcp;
	
	// define fan functions
	fFan[0] = set_setFan1;
	fFan[1] = fNoAction;
	fFan[2] = fNoAction;
	fFan[3] = set_setFanMin;
	fFan[4] = set_setFanMax;
	fFan[5] = set_setFanRPM1;
	fFan[6] = set_setFanState;
	fFan[7] = set_setFan2;
	fFan[8] = fNoAction;
	fFan[9] = fNoAction;
	fFan[10] = set_setFanMin;
	fFan[11] = set_setFanMax;
	fFan[12] = set_setFanRPM2;
	fFan[13] = set_setFanState;
	fFan[14] = set_setFan3;
	fFan[15] = set_setFan4;

	// define control functions
	fControl[0] = fNoAction;
	fControl[1] = fNoAction;
	fControl[2] = fNoAction;
	fControl[3] = readChamberThermistorBeta;
	fControl[4] = readRejectThermistorBeta;
	fControl[5] = readPhp1OutputDutyOffset;
	fControl[6] = readPhp2OutputDutyOffset;
	fControl[7] = set_maxSnsErrorCount;
	fControl[8] = set_maxFanErrorCount;
	fControl[9] = set_maxPhpErrorCount;
	fControl[10] = set_maxAllowedChamberTemp;
	fControl[11] = set_minAllowedChamberTemp;
	fControl[12] = set_doorOpenCount1HrLimit;
	
	return;	
}