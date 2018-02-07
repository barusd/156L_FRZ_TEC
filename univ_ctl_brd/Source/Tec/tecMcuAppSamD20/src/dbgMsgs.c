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
const char *modeNames[7] = {"RESET", "IDLE", "HIGH_Q", "VAR_Q", "HIGH_COP", "DEFROST", "MANUAL"};
const char *outputStatusNames[3] = {"OFF", "GPIO", "PWM"};
const char *snsModeNames[] = {"RESET", "NORMAL", "FAIL1", "FAIL2", "FAIL3", "FAIL4", "NORMAL"};
bool enableDbgLoggingMode = false;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
void printDbgMsgs()
{
	// print current time for user
	rtc_calendar_get_time(&time_report.time);
		
	if (!enableDbgLoggingMode) { dbg("\n\n"); }
	dbg_info("%2dy%2dd:%2dh:%2dm:%2ds, time=%lu, ERR=%04x, FW=%02x%2x, pwrMode=%8s, HUM=%d, chaSnsMode=%8s, rejSnsMode=%8s,",
		time_report.time.year,
		time_report.time.day,
		time_report.time.hour,
		time_report.time.minute,
		time_report.time.second,
		presentTimeInSec()-24*60*60,
		errorRegister,
		firmwareVersion,
		firmwareRevision,
		modeNames[chamber1.mode_state],
		highUsageModeFlag,
		snsModeNames[sensor_state_tcha],
		snsModeNames[sensor_state_trej]);
		
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("door=%s, drCnt1Hr=%d, drCntTot=%ld, drCntLife=%ld,",
		doorOpenPresentStatus?"open":"closed",
		doorOpenCount_1Hr,
		doorOpenCount_Tec,
		doorOpenLifetimeCnt);		
			
	if (!enableDbgLoggingMode) { dbg("\n"); }	
	dbg_info("cha1Offset=%4.2f, chaOffset=%4.2f, glyOffset=%4.2f, dbH=%4.2f, dbL=%4.2f,",
		chamber1.chamberOffset,
		chamber2.chamberOffset,
		chamber1.glycolOffset,
		chamber1.deadband,
		chamber1.deadbandLow);
			
	if (!enableDbgLoggingMode) { dbg("\n"); }			
	dbg_info("Tset=%4.2f, Tctl=%4.2f,",
		chamber1.setpoint,
		controlTemperature);
			
	#ifdef HIGH_PERFORMANCE	
	dbg_info(" Tcha1=%4.2f,", chamber1.chamberTemp);
	#endif
			
	dbg_info(" Tcha2=%4.2f, Tgly=%4.2f, Trej1=%4.2f,",	
		chamber2.chamberTemp,
		chamber1.glycolTemp,
		chamber1.rejectTemp);

	#ifdef HIGH_PERFORMANCE
	dbg_info(" Trej2=%4.2f, Tdif=%4.2f,",
		chamber2.rejectTemp,
		chamberTempDifference);		
	#endif
	
	dbg_info(" cond=%4.2f,",
		condensateTemp);

	if (!enableDbgLoggingMode) { dbg("\n"); }		
	dbg_info("VB=%4.2f, VCC=%5.3f, VBAT=%4.2f, Vset=%4.2f,",
		ps1.bulkVoltage,
		ps1.standbyVoltage,
		//ps1.logicVoltage,
		ps1.batteryVoltage,
		ps1.desiredVoltage);		
	dbg_info(" V1=%4.2f, I1=%4.2f, W1=%4.2f,",
		ps1.phpVoltage,
		ps1.outputCurrent,
		ps1.outputPower);
		
	#ifdef HIGH_PERFORMANCE	
	dbg_info(" V2=%4.2f, I2=%4.2f, W2=%4.2f,",
		ps2.phpVoltage,
		ps2.outputCurrent,
		ps2.outputPower);
	#endif

	if (!enableDbgLoggingMode) { dbg("\n"); }		
	dbg_info("CHAMBER-A, Fan1=%d, Set1=%d, Rpm1=%lu, Stall1=%d, f1State=%d, pid4=%4.2f, inh=%lu, inh_to=%lu,",
		fan1.state,
		fan1.intDutyPercent,
		fan1.rpm,
		fan1.stall,
		fan1.fan_state,
		pidFan4.Output,
		inhibitChamberFanTimestamp,
		inhibitChamberFanTimeoutTimestamp);

	dbg_info(" Rpm1Set=%d, LwrLim1=%d,",
		(uint16_t)fan1.fanRpmSetting,
		(uint16_t)fan1.fanSpdLwrLim);	
	
	if (!enableDbgLoggingMode) { dbg("\n"); }		
	dbg_info("REJECT-1A, Fan2=%d, Set2=%d, Rpm2=%lu, Stall2=%d, f2State=%d,",
		fan2.state,
		fan2.intDutyPercent,
		fan2.rpm,
		fan2.stall,
		fan2.fan_state);

	dbg_info(" Rpm2Set=%d, LwrLim2=%d,",
		(uint16_t)fan2.fanRpmSetting,
		(uint16_t)fan2.fanSpdLwrLim);

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("REJECT-2A, Fan5=%d, Set5=%d, Rpm5=%lu, Stall5=%d, f5State=%d,",
		fan5.state,
		fan5.intDutyPercent,
		fan5.rpm,
		fan5.stall,
		fan5.fan_state);
	
	dbg_info(" Rpm5Set=%d, LwrLim5=%d,",
		(uint16_t)fan5.fanRpmSetting,
		(uint16_t)fan5.fanSpdLwrLim);
	
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("REJECT-3A, Fan6=%d, Set6=%d, Rpm6=%lu, Stall6=%d, f6State=%d,",
		fan6.state,
		fan6.intDutyPercent,
		fan6.rpm,
		fan6.stall,
		fan6.fan_state);

	dbg_info(" Rpm6Set=%d, LwrLim6=%d,",
		(uint16_t)fan6.fanRpmSetting,
		(uint16_t)fan6.fanSpdLwrLim);	

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("CHAMBER-B, Fan4=%d, Set4=%d, Rpm4=%lu, Stall4=%d, f4State=%d, pid4=%4.2f, inh=%lu, inh_to=%lu,",
		fan4.state,
		fan4.intDutyPercent,
		fan4.rpm,
		fan4.stall,
		fan4.fan_state,
		pidFan4.Output,
		inhibitChamberFanTimestamp,
		inhibitChamberFanTimeoutTimestamp);

	dbg_info(" Rpm4Set=%d, LwrLim4=%d,",
		(uint16_t)fan4.fanRpmSetting,
		(uint16_t)fan4.fanSpdLwrLim);

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("REJECT-1B, Fan3=%d, Set3=%d, Rpm3=%lu, Stall3=%d, f3State=%d,",
		fan3.state,
		fan3.intDutyPercent,
		fan3.rpm,
		fan3.stall,
		fan3.fan_state);

	dbg_info(" Rpm3Set=%d, LwrLim3=%d,",
		(uint16_t)fan3.fanRpmSetting,
		(uint16_t)fan3.fanSpdLwrLim);
		
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("REJECT-2B, Fan8=%d, Set8=%d, Rpm8=%lu, Stall8=%d, f8State=%d,",
		fan8.state,
		fan8.intDutyPercent,
		fan8.rpm,
		fan8.stall,
		fan8.fan_state);

	dbg_info(" Rpm8Set=%d, LwrLim8=%d,",
		(uint16_t)fan8.fanRpmSetting,
		(uint16_t)fan8.fanSpdLwrLim);
	
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("REJECT-3B, Fan7=%d, Set7=%d, Rpm7=%lu, Stall7=%d, f7State=%d,",
		fan7.state,
		fan7.intDutyPercent,
		fan7.rpm,
		fan7.stall,
		fan7.fan_state);

	dbg_info(" Rpm7Set=%d, LwrLim7=%d,",
		(uint16_t)fan7.fanRpmSetting,
		(uint16_t)fan7.fanSpdLwrLim);		
	
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("OutA_Cold=%d, OutA_Hot=%d, OutB_Cold=%d, OutB_Hot=%d,",
		port_pin_get_output_level(PIN_ENBL_OUTA_COLD),
		port_pin_get_output_level(PIN_ENBL_OUTA_HOT),
		port_pin_get_output_level(PIN_ENBL_OUTB_COLD),
		port_pin_get_output_level(PIN_ENBL_OUTB_HOT) );
	
	/*
	if (!enableDbgLoggingMode) { dbg("\n"); }		
	dbg_info("OutState1=%4s, Val1=%d, php1Offset=%2.2f,",
		outputStatusNames[ps1.outputStatus],
		ps1.outputStatus>GPIO_MODE?ps1.outputDuty:port_pin_get_output_level(PIN_ENBL_OUTPUT1),
		ps1.phpOutputDutyOffset);
		
	#ifdef HIGH_PERFORMANCE	
	dbg_info(" OutState2=%4s, Val2=%d, php2Offset=%2.2f,",
		outputStatusNames[ps2.outputStatus],
		ps2.outputStatus>GPIO_MODE?ps2.outputDuty:port_pin_get_output_level(PIN_ENBL_OUTPUT2),
		ps2.phpOutputDutyOffset);
	#endif
	*/
	if (!enableDbgLoggingMode) { dbg("\n"); }	
	dbg_info("htxAHeater=%d, htxBHeater=%d,",
		htxAHeater.output_state,
		htxBHeater.output_state);

	if (!enableDbgLoggingMode) { dbg("\n"); }		
	dbg_info("condHtrState=%d, condHtrDlySecs=%ld, condHtrDuty=%d, doorHtrState=%d, doorHtrDuty=%d,",
		condHeater.sm_state,
		condHeater.sm_state==STATE_CONDHTR_S3?(presentTimeInSec()-condHeaterStartTime):0,
		condHeater.duty,
		doorHeater.output_state,
		doorHeater.duty);
		
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("Pterm=%4.2f, Iterm=%4.2f, Dterm=%4.2f, Vcop=%3.1f, Kpq=%3.1f, Kiq=%12f, Kpc=%3.1f, Kic=%12f, enPc=%d,",
		pidLoop1.PTerm,
		pidLoop1.ITerm,
		pidLoop1.DTerm,
		chamber1.VmaxCOP,
		pidLoop1.kp_highq,
		pidLoop1.ki_highq,
		pidLoop1.kp_copq,
		pidLoop1.ki_copq,
		enablePwrCap);

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("d_cycles=%d,",
		defrostCycleCount);
				
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("dmi_act=%d, dmi_cnt=%ld, dmi_strt=%ld, dmi_exp=%d,",
		defrostMinimumIntervalTimer.timerActive,
		defrostMinimumIntervalTimer.presentCountInSecs,
		defrostMinimumIntervalTimer.startTimestampInSecs>0?defrostMinimumIntervalTimer.startTimestampInSecs-24*60*60:defrostMinimumIntervalTimer.startTimestampInSecs,
		defrostMinimumIntervalTimer.timerExpired);

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("di_act=%d, di_cnt=%ld, di_strt=%ld, di_exp=%d,",
		defrostIntervalTimer.timerActive,
		defrostIntervalTimer.presentCountInSecs,
		defrostIntervalTimer.startTimestampInSecs>0?defrostIntervalTimer.startTimestampInSecs-24*60*60:defrostIntervalTimer.startTimestampInSecs,
		defrostIntervalTimer.timerExpired);

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("dma_act=%d, dma_cnt=%ld, dma_strt=%ld, dma_exp=%d,",
		defrostMaximumIntervalTimer.timerActive,
		defrostMaximumIntervalTimer.presentCountInSecs,
		defrostMaximumIntervalTimer.startTimestampInSecs>0?defrostMaximumIntervalTimer.startTimestampInSecs-24*60*60:defrostMaximumIntervalTimer.startTimestampInSecs,
		defrostMaximumIntervalTimer.timerExpired);

	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("f_act=%d, f_cnt=%ld, f_strt=%ld, f_exp=%d,",
		defrostFreezupTimer.timerActive,
		defrostFreezupTimer.presentCountInSecs,
		defrostFreezupTimer.startTimestampInSecs>0?defrostFreezupTimer.startTimestampInSecs-24*60*60:defrostFreezupTimer.startTimestampInSecs,
		defrostFreezupTimer.timerExpired);
		
	/*	
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("t4Time=%ld, t4Last=%ld, freq=%d, mins=%d, active=%d, en=%d, Tkill=%4.2f,",
		task4Time,
		task4LastTime-24*60*60,
		defrostFreqHrs,
		defrostDurationMins,
		defrostActive,
		enableAutoDefrost,
		defrostCancelTemp);
	*/	
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("t8Time=%ld, tphpChk=%d, t9Time=%ld, t9Trig=%ld, t10Time=%ld, t10Trig=%ld,",
		task8Time,
		phpChkTimeIntervalInSecs,
		task9Time,
		task9TriggerTime,
		task10Time,
		task10TriggerTime);			
	/*	
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("%2dy%2dd:%2dh:%2dm:%2ds, ",
		php_selfchk_alarm1.year,
		php_selfchk_alarm1.day,
		php_selfchk_alarm1.hour,
		php_selfchk_alarm1.minute,
		php_selfchk_alarm1.second);		
	dbg_info("%2dy%2dd:%2dh:%2dm:%2ds, ",
		php_selfchk_alarm2.year,
		php_selfchk_alarm2.day,
		php_selfchk_alarm2.hour,
		php_selfchk_alarm2.minute,
		php_selfchk_alarm2.second);
	dbg_info("%2dy%2dd:%2dh:%2dm:%2ds, ",
		php_selfchk_alarm3.year,
		php_selfchk_alarm3.day,
		php_selfchk_alarm3.hour,
		php_selfchk_alarm3.minute,
		php_selfchk_alarm3.second);		
	*/
				
	if (mfgTestModeFlag1==false)
	{
		printErrorStatus();
	}
					
	if (!enableDbgLoggingMode) { dbg("\n"); }
	if (chamber1.deactivateAutoControl==true)	{ dbg("|AUTO OVERRIDE|");		}
	if (enableManualOverride==true)				{ dbg("|PID OVERRIDE|");		}
	if (enableTempOverride==true)				{ dbg("|TEMP OVERRIDE|");		}
	if (inhibitOutput==true)					{ dbg("|STARTUP INHIBIT|");		}
	if (enableFanOverride==true)				{ dbg("|FAN OVERRIDE|");		}
	if (uiSetpointControl==false)				{ dbg("|UI SETPOINT OVERRIDE|");}
	if (deactivateAutoHeaterControl==true)		{ dbg("|HEATER OVERRIDE|");		}
	if (enableSensorEDFI==false)				{ dbg("|SENSOR EDFI DISABLED|");}						
		
	if (enableDbgLoggingMode) { dbg("\n"); } //{ dbg("\r"); }
}

void printErrorStatus()
{
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("maxSnsErrCnt=%d, cha1ErrCnt=%d, cha2ErrCnt=%d, rejErrCnt=%d, rejCritErrCnt=%d,",
		maxSnsErrorCount,
		cha1SnsErrCnt,
		cha2SnsErrCnt,
		rejSnsErrCnt,
		rejCritErrCnt);
		
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("maxFanErrCnt=%d, fan1LowSpdErrCnt=%d, fan1ErrCnt=%d, fan2LowSpdErrCnt=%d, fan2ErrCnt=%d, fan4LowSpdErrCnt=%d, fan4ErrCnt=%d,",
		maxFanErrorCount,
		fan1LowSpdErrCnt,
		fan1ErrorCount,
		fan2LowSpdErrCnt,
		fan2ErrorCount,
		fan4LowSpdErrCnt,
		fan4ErrorCount);
			
	if (!enableDbgLoggingMode) { dbg("\n"); }
	dbg_info("maxPhpErrCnt=%d, php1TotErrCnt=%d,",
		maxPhpErrorCount,
		php1TotalErrorCount);
	
	#ifdef HIGH_PERFORMANCE	
	dbg_info(" php2TotErrCnt=%d,",
		php2TotalErrorCount);			
	#endif
}

void printBootMsg()
{
	#ifdef HIGH_PERFORMANCE
	dbg("\nproduct is HIGH PERFORMANCE");
	#else
	dbg("\nproduct is GENERAL PURPOSE");
	#endif
	dbg_info("\nps is %s", psNameString[ps1.PsType]);
	dbg_info("\neepromStatus is %lx", eepromStatus);
	dbg_info("\nmfgTestModeFlag1 is %x", mfgTestModeFlag1);
	dbg_info("\nmfgTestModeFlag2 is %x", mfgTestModeFlag2);
	dbg_info("\nrunSelfTestOnPor is %x", runSelfTestOnPOR);
	dbg_info("\nchamber1 offset %4.2f", chamber1.chamberOffset);
	dbg_info("\nglycol1 offset %4.2f", chamber1.glycolOffset);
	dbg_info("\nchamber2 offset %4.2f", chamber2.chamberOffset);
	dbg_info("\nchamberThermistorBeta %4d", chamberThermistorBeta);
	dbg_info("\nrejectThermistorBeta %4d", rejectThermistorBeta);
	dbg_info("\nps1.phpOutputDutyOffset %4.2f", ps1.phpOutputDutyOffset);
	dbg_info("\nps2.phpOutputDutyOffset %4.2f", ps2.phpOutputDutyOffset);
	#ifdef HIGH_PERFORMANCE
	dbg_info("\ndefrost freq hrs %d", defrostFreqHrs);
	#else
	dbg_info("\nhigh usage mode 1hr door cnt limit %d", doorOpenCount1HrLimit);	
	dbg_info("\ndefrost min interval hrs %d", defrostFreqHrs);
	dbg_info("\ndefrost max interval hrs %d", defrostFreqMaxHrs);
	#endif
	dbg_info("\ndefrost duration mins %d", defrostDurationMins);
	dbg_info("\nmaxAllowedChamberTemp %4.2f", maxAllowedChamberTemp);
	dbg_info("\nminAllowedChamberTemp %4.2f", minAllowedChamberTemp);		
	dbg_info("\nmodel number %s", strModel);
	dbg_info("\nserial number %s", strSerial);
	dbg_info("\nui fw ver %s", strUiFwVer);
	dbg_info("\nmac addr %02X:%02X:%02X:%02X:%02X:%02X",
	MAC_address[0], MAC_address[1], MAC_address[2],
	MAC_address[3], MAC_address[4], MAC_address[5] );
	dbg_info("\ndoorOpenLifetimeCnt %ld", doorOpenLifetimeCnt);
	dbg_info("\nfirmware build %.2x.%.2x", firmwareVersion, firmwareRevision);
		
}
