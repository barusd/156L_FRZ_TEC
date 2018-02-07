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
bool flagSetOverride = false;

/************************************************************************/
/* User Functions                                                       */
/************************************************************************/
float set_setdesiredVoltage (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		if (nZone == 0)
		{
			ps1.desiredVoltage = fNewValue;
		}
/*		else
		{
			ps2.desiredVoltage = fNewValue;
		}
*/
		flagSetOverride = true;
	}
	
	return (1);
}

float set_setPointValue (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.setpoint = fNewValue;
			//instProtocol[0].setpoint = fNewValue;
		}
		return chamber1.setpoint;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.setpoint = fNewValue;
			//instProtocol[1].setpointC = fNewValue;
		}
		return chamber2.setpoint;
	}	

}

float set_setRejectLimit (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.rejectLimit = fNewValue;
		}
		return chamber1.rejectLimit;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.rejectLimit = fNewValue;
		}
		return chamber2.rejectLimit;
	}

}

float set_setRejectLimitCritical (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.rejectLimitCritical = fNewValue;
		}
		return chamber1.rejectLimitCritical;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.rejectLimitCritical = fNewValue;
		}
		return chamber2.rejectLimitCritical;
	}

}

float set_setRejectDeadband (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.rejectLowLimit = fNewValue;
		}
		return chamber1.rejectLowLimit;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.rejectLowLimit = fNewValue;
		}
		return chamber2.rejectLowLimit;
	}

}

float set_setDeadband (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.deadband = fNewValue;
		}
		return chamber1.deadband;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.deadband = fNewValue;
		}
		return chamber2.deadband;
	}

}

float set_setDeadbandLow (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.deadbandLow = fNewValue;
		}
		return chamber1.deadbandLow;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.deadbandLow = fNewValue;
		}
		return chamber2.deadbandLow;
	}

}

float set_setHeatLeak (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.heatLeak = fNewValue;
		}
		return chamber1.heatLeak;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.heatLeak = fNewValue;
		}
		return chamber2.heatLeak;
	}

}

/*
double set_setChamberOffset1 (float fNewValue, bool bValid)
{
	if (bValid)
		nTcha1_offset = fNewValue;
	
	return nTcha1_offset;
}

double set_setChamberOffset2 (float fNewValue, bool bValid)
{
	if (bValid)
		nTcha2_offset = fNewValue;
	
	return nTcha2_offset;
}
*/

float set_setChamberOffset (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.chamberOffset = fNewValue;
			eepromDataInfo.pAddr = &chamber1.chamberOffset;
			eepromDataInfo.size = sizeof(chamber1.chamberOffset);
			eepromDataInfo.nonvolatileFlag = true;
			writeToEepromFlag = true;
		}
		return chamber1.chamberOffset;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			chamber2.chamberOffset = fNewValue;
			eepromDataInfo.pAddr = &chamber2.chamberOffset;
			eepromDataInfo.size = sizeof(chamber2.chamberOffset);
			eepromDataInfo.nonvolatileFlag = true;
			writeToEepromFlag = true;
		}
		return chamber2.chamberOffset;
	}
	
}

float set_setFan1 (float fNewValue, bool bValid)
{
	if (bValid)
	{
		fan1.targetDutyPercent = (uint32_t)fNewValue;
	}
	return fNewValue;
}

float set_setFan2 (float fNewValue, bool bValid)
{
	if (bValid)
	{
		fan2.targetDutyPercent = (uint32_t)fNewValue;
	}
	return fNewValue;
}

float set_setFan3 (float fNewValue, bool bValid)
{
	if (bValid)
	{
		fan3.targetDutyPercent = (uint32_t)fNewValue;
	}
	return fan3.rpm;
}

float set_setFan4 (float fNewValue, bool bValid)
{
	if (bValid)
	{
		fan4.targetDutyPercent = (uint32_t)fNewValue;
	}
	return fan4.rpm;
}
/*
float set_setFanTop (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			fan1.Vtop = fNewValue;
		}
		return fan1.Vtop;
	}
	else
	{
		if (bValid)
		{
			fan2.Vtop = fNewValue;
		}
		return fan2.Vtop;
	}
}

float set_setFanBottom (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			fan1.Vbottom =fNewValue;
		}
		return fan1.Vbottom;
	}
	else
	{
		if (bValid)
		{
			fan2.Vbottom = fNewValue;
		}
		return fan2.Vbottom;
	}
}
*/

float set_setFanMin (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			fan1.minDuty = fNewValue;
			//fan1.minDuty = (uint32_t)((fNewValue*DEFAULT_FAN_MAX_DUTY)/100);
		}
		return fan1.minDuty;
		//return ((float)fan1.minDuty/DEFAULT_FAN_MAX_DUTY)*100;
	}
	else
	{
		if (bValid)
		{
			fan2.minDuty = fNewValue;
			//fan2.minDuty = (uint32_t)((fNewValue*DEFAULT_FAN_MAX_DUTY)/100);
		}
		return fan2.minDuty;
		//return ((float)fan2.minDuty/DEFAULT_FAN_MAX_DUTY)*100;
	}
}

float set_setFanMax (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			fan1.maxDuty = fNewValue;
			//fan1.maxDuty = (uint32_t)((fNewValue*DEFAULT_FAN_MAX_DUTY)/100);
		}
		return fan1.maxDuty;
		//return ((float)fan1.maxDuty/DEFAULT_FAN_MAX_DUTY)*100;
	}
	else
	{
		if (bValid)
		{
			fan2.maxDuty = fNewValue;
			//fan2.maxDuty = (uint32_t)((fNewValue*DEFAULT_FAN_MAX_DUTY)/100);
		}
		return fan2.maxDuty;
		//return ((float)fan2.maxDuty/DEFAULT_FAN_MAX_DUTY)*100;
	}
}

float set_setFanRPM1 (float fNewValue, bool bValid)
{
	if (bValid)
	{
		//last_tac_rpm0 = (uint32_t)(fNewValue);
		//fan1.tach = last_tac_rpm0;
	}
	
	return fan1.rpm;
}

float set_setFanRPM2 (float fNewValue, bool bValid)
{
	if (bValid)
	{
		//last_tac_rpm1 = (uint32_t)(fNewValue);
		//fan2.tach = last_tac_rpm0;
	}
	
	return fan2.rpm;
}

float set_setVarqVoltage (float fNewValue, bool bValid, int nZone)
{
/*
	if (bValid)
		dblVarqVoltage = fNewValue;
		
	return dblVarqVoltage;
*/	
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.Vvarq = fNewValue;
		}
		return chamber1.Vvarq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			chamber2.Vvarq = fNewValue;
		}
		return chamber2.Vvarq;
	}
	
}

float set_setVmaxCOP (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.VmaxCOP = fNewValue;
		}
		return chamber1.VmaxCOP;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			chamber2.VmaxCOP = fNewValue;
		}
		return chamber2.VmaxCOP;
	}

}

float set_setFanState (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			//fan1.state = (bool)(fNewValue);
		}
		return fan1.state;
	}
	return 0;
/*
	else
	{
		if (bValid)
		{
			//fan2.Vbottom = (bool)(fNewValue);
		}
		return fan2.state;
	}
*/
}

float set_setMaxBulkPower (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
	  maxOutputVoltage = (uint32_t)(fNewValue);
	  //pidSetOutputLimits(&pidLoop1, 0, maxOutputVoltage);
	  //pidSetOutputLimits(&pidLoop2, 0, maxOutputVoltage);
	}
	
	return maxOutputVoltage;
}

float readPhpVoltage (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.phpVoltage;
}

float readPhpCurrent (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.outputCurrent;
}

float readBulkVoltage (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.bulkVoltage;
}

float readLogicVoltage (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.logicVoltage;
}

float readStandbyVoltage (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.standbyVoltage;
}

float readBatteryVoltage (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.batteryVoltage;
}

float readPhpVoltage2 (float fNewValue, bool bValid, int nZone)
{
	return (float)ps2.phpVoltage;
}

float readPhpCurrent2 (float fNewValue, bool bValid, int nZone)
{
	return (float)ps2.outputCurrent;
}

float set_setKP_HIGHQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.kp_highq = fNewValue;
			if (chamber1.mode_state == STATE_HIGH_Q)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_highq, pidLoop1.ki_highq, pidLoop1.kd_highq);
			}
		}
		return pidLoop1.kp_highq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.kp_highq = fNewValue;
			if (chamber2.mode_state == STATE_HIGH_Q)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_highq, pidLoop2.ki_highq, pidLoop2.kd_highq);
			}
		}
		return pidLoop2.kp_highq;
	}

}

float set_setKI_HIGHQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.ki_highq = fNewValue;
			if (chamber1.mode_state == STATE_HIGH_Q)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_highq, pidLoop1.ki_highq, pidLoop1.kd_highq);
			}
		}
		return pidLoop1.ki_highq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.ki_highq = fNewValue;
			if (chamber2.mode_state == STATE_HIGH_Q)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_highq, pidLoop2.ki_highq, pidLoop2.kd_highq);
			}
		}
		return pidLoop2.ki_highq;
	}

}

float set_setKD_HIGHQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.kd_highq = fNewValue;
			if (chamber1.mode_state == STATE_HIGH_Q)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_highq, pidLoop1.ki_highq, pidLoop1.kd_highq);
			}
		}
		return pidLoop1.kd_highq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.kd_highq = fNewValue;
			if (chamber2.mode_state == STATE_HIGH_Q)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_highq, pidLoop2.ki_highq, pidLoop2.kd_highq);
			}
		}
		return pidLoop2.kd_highq;
	}

}

float set_setKP_VARQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.kp_varq = fNewValue;
			if (chamber1.mode_state == STATE_VAR_Q)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_varq, pidLoop1.ki_varq, pidLoop1.kd_varq);
			}
		}
		return pidLoop1.kp_varq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.kp_varq = fNewValue;
			if (chamber2.mode_state == STATE_VAR_Q)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_varq, pidLoop2.ki_varq, pidLoop2.kd_varq);
			}
		}
		return pidLoop2.kp_varq;
	}

}

float set_setKI_VARQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.ki_varq = fNewValue;
			if (chamber1.mode_state == STATE_VAR_Q)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_varq, pidLoop1.ki_varq, pidLoop1.kd_varq);
			}
		}
		return pidLoop1.ki_varq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.ki_varq = fNewValue;
			if (chamber2.mode_state == STATE_VAR_Q)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_varq, pidLoop2.ki_varq, pidLoop2.kd_varq);
			}
		}
		return pidLoop2.ki_varq;
	}

}

float set_setKD_VARQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.kd_varq = fNewValue;
			if (chamber1.mode_state == STATE_VAR_Q)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_varq, pidLoop1.ki_varq, pidLoop1.kd_varq);
			}
		}
		return pidLoop1.kd_varq;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.kd_varq = fNewValue;
			if (chamber2.mode_state == STATE_VAR_Q)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_varq, pidLoop2.ki_varq, pidLoop2.kd_varq);
			}
		}
		return pidLoop2.kd_varq;
	}

}

float set_setKP_COPQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.kp_copq = fNewValue;
			if (chamber1.mode_state == STATE_HIGH_COP)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_copq, pidLoop1.ki_copq, pidLoop1.kd_copq);
			}
		}
		return pidLoop1.kp_copq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.kp_copq = fNewValue;
			if (chamber2.mode_state == STATE_HIGH_COP)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_copq, pidLoop2.ki_copq, pidLoop2.kd_copq);
			}
		}
		return pidLoop2.kp_copq;
	}

}

float set_setKI_COPQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.ki_copq = fNewValue;
			if (chamber1.mode_state == STATE_HIGH_COP)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_copq, pidLoop1.ki_copq, pidLoop1.kd_copq);
			}
		}
		return pidLoop1.ki_copq;
	}
	// return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.ki_copq = fNewValue;
			if (chamber2.mode_state == STATE_HIGH_COP)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_copq, pidLoop2.ki_copq, pidLoop2.kd_copq);
			}
		}
		return pidLoop2.ki_copq;
	}

}

float set_setKD_COPQ (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			pidLoop1.kd_copq = fNewValue;
			if (chamber1.mode_state == STATE_HIGH_COP)
			{
				pidSetTunings(&pidLoop1, pidLoop1.kp_copq, pidLoop1.ki_copq, pidLoop1.kd_copq);
			}
		}
		return pidLoop1.kd_copq;
	}
	//return 0;

	else
	{
		if (bValid)
		{
			pidLoop2.kd_copq = fNewValue;
			if (chamber2.mode_state == STATE_HIGH_COP)
			{
				pidSetTunings(&pidLoop2, pidLoop2.kp_copq, pidLoop2.ki_copq, pidLoop2.kd_copq);
			}
		}
		return pidLoop2.kd_copq;
	}

}

float set_setChamberTemp (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.chamberTemp = fNewValue;
			chamber1.chamberTempRaw = fNewValue;
		}
		return chamber1.chamberTemp;
	}
	else
	{
		if (bValid)
		{
			chamber2.chamberTemp = fNewValue;
			chamber2.chamberTempRaw = fNewValue;
		}	
		return chamber2.chamberTemp;	
	}
}

float set_setRejectTemp (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.rejectTemp = fNewValue;
		}
		return chamber1.rejectTemp;
	}
	else
	{
		if (bValid)
		{
			chamber2.rejectTemp = fNewValue;
		}
		return chamber2.rejectTemp;
	}
}

float set_setGlycolOffset (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		chamber1.glycolOffset = fNewValue;
		eepromDataInfo.pAddr = &chamber1.glycolOffset;
		eepromDataInfo.size = sizeof(chamber1.glycolOffset);
		eepromDataInfo.nonvolatileFlag = true;
		writeToEepromFlag = true;
	}
	return chamber1.glycolOffset;
}

float set_setModeState (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			chamber1.mode_state = fNewValue;
		}
		return chamber1.mode_state;
	}
	else
	{
		if (bValid)
		{
			chamber2.mode_state = fNewValue;
		}
		return chamber2.mode_state;
	}
}

float set_setEnablePcp (float fNewValue, bool bValid, int nZone)
{
	if (nZone == 0)
	{
		if (bValid)
		{
			enableSideAPcp = (bool)fNewValue;
		}
		return (float)enableSideAPcp;
	}
	else
	{
		if (bValid)
		{
			enableSideBPcp = (bool)fNewValue;
		}
		return (float)enableSideBPcp;
	}
}

float set_setDesiredVoltage (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		ps1.desiredVoltage = fNewValue;
	}
	
	return ps1.desiredVoltage;
}

float set_setGlycolTemp (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		chamber1.glycolTemp = fNewValue;
	}
	
	return chamber1.glycolTemp;
}

float set_maxSnsErrorCount (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		maxSnsErrorCount = fNewValue;
	}
	
	return maxSnsErrorCount;	
}

float set_maxFanErrorCount (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		maxFanErrorCount = fNewValue;
	}
	
	return maxFanErrorCount;
}

float set_maxPhpErrorCount (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		maxPhpErrorCount = fNewValue;
	}
	
	return maxPhpErrorCount;
}

float set_maxAllowedChamberTemp (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		maxAllowedChamberTemp = fNewValue;
		eepromDataInfo.pAddr = &maxAllowedChamberTemp;
		eepromDataInfo.size = sizeof(maxAllowedChamberTemp);
		eepromDataInfo.nonvolatileFlag = true;
		writeToEepromFlag = true;
	}
	
	return maxAllowedChamberTemp;
}

float set_minAllowedChamberTemp (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		minAllowedChamberTemp = fNewValue;
		eepromDataInfo.pAddr = &minAllowedChamberTemp;
		eepromDataInfo.size = sizeof(minAllowedChamberTemp);
		eepromDataInfo.nonvolatileFlag = true;
		writeToEepromFlag = true;
	}
	
	return minAllowedChamberTemp;
}

float fNoAction (float fNewValue, bool bValid, int nZone)
{
	return 0;
}

float readChamberThermistorBeta (float fNewValue, bool bValid, int nZone)
{
	return (float)chamberThermistorBeta;
}

float readRejectThermistorBeta (float fNewValue, bool bValid, int nZone)
{
	return (float)rejectThermistorBeta;
}

float readPhp1OutputDutyOffset (float fNewValue, bool bValid, int nZone)
{
	return (float)ps1.phpOutputDutyOffset;
}

float readPhp2OutputDutyOffset (float fNewValue, bool bValid, int nZone)
{
	return (float)ps2.phpOutputDutyOffset;
}

float set_doorOpenCount1HrLimit (float fNewValue, bool bValid, int nZone)
{
	if (bValid)
	{
		doorOpenCount1HrLimit = (uint8_t)fNewValue;
		eepromDataInfo.pAddr = &doorOpenCount1HrLimit;
		eepromDataInfo.size = sizeof(doorOpenCount1HrLimit);
		eepromDataInfo.nonvolatileFlag = true;
		writeToEepromFlag = true;
	}
	
	return (float)doorOpenCount1HrLimit;
}


void usartCommandHandler()
{
	bool bResult = false;
	char bCmdGroup[8];
	char bCommand[8];
	char bParam[16];
	char bFraction[8];
	uint8_t nLength = 0;
	char* strTokens = NULL;
	uint8_t nIndexParser = 0;
	uint8_t nParseLength = 0;
	bool result = false;
    int  nSign = 1;
	float dblGetValue= 0.0f;
	
	//Copy incoming buffer
	nLength = command[0];
	strTokens = strtok ((char*)&command[1], " ,.\n");
	while (strTokens != NULL)
	{
		nParseLength += strlen(strTokens);
		if (nIndexParser == 0)
		{
		  memcpy (bCmdGroup, strTokens, strlen(strTokens));
		  bCmdGroup[strlen(strTokens)] = 0x00;
		}
		else if (nIndexParser == 1)
		{
		  memcpy (bCommand, strTokens, strlen(strTokens));
		  bCommand[strlen(strTokens)] = 0x00;
		}
		else if (nIndexParser == 2)
		{
          if (strTokens[0] == '+')
          {
            nSign = 1;
            strTokens++;
          }
          else if (strTokens[0] == '-')
          {
            nSign = -1;
            strTokens++;
          }
		  memcpy (bParam, strTokens, strlen(strTokens));
		  bParam[strlen(strTokens)] = 0x00;
		}
		else if (nIndexParser == 3)
		{
			memcpy (bFraction, strTokens, strlen(strTokens));
			bFraction[strlen(strTokens)] = 0x00;
		}
		
		if ((nParseLength+1) < nLength)
		{
		  strTokens = strtok (NULL, " ,.\n");  
		}
		else
		  strTokens = NULL;
		  
		nIndexParser += 1;
		nParseLength += 1; //adjust for .
	}
	/*
	dbg_info("\nbCmdGrp %s", (int)strtol((const char*)bCmdGroup, NULL, 10));
	dbg_info("\nbCmd    %s", bCommand);
	dbg_info("\nbParam  %s", bParam);
	dbg_info("\nbFrac   %s", bFraction);
	*/
	switch ((int)strtol((const char*)bCmdGroup, NULL, 10))
	{
		case 0:
		{
		  switch (bCommand[0])
		  {
//------------------------------------------------------------------
// 0.0.x	enable/disable manual override
//------------------------------------------------------------------			  
			  case '0':
				  result = ((bParam[0] - '0') != 0);
				  enableManualOverride = result;
				  if (enableManualOverride)
				  {
					  chamber1.deactivateAutoControl = true;
					  chamber2.deactivateAutoControl = true;
					  dbg("\n[manual override enabled]\n");
				  }
				  else
				  {
					  chamber1.deactivateAutoControl = false;
					  chamber2.deactivateAutoControl = false;
					  dbg("\n[manual override disabled]");
				  }
				  break;
//------------------------------------------------------------------
// 0.1.x	enable/disable temp sensor override
//------------------------------------------------------------------				  
			  case '1':
				  result = ((bParam[0] - '0') != 0);
				  enableTempOverride = result;
				  enableSensorEDFI = false;
				  if (result)
				  {
					  dbg("\n[sensor override enabled]\n");
				  }
				  else
				  {
					  dbg("\n[sensor override disabled]\n");
				  }
				  break;			  
			  case '2':
			  {
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10);
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10);
				int ngetValue = 0;
				int nZone = 0;
				
				switch (nParamValue)
				{
//------------------------------------------------------------------
// 0.2.0.x	print pin state
//------------------------------------------------------------------					
					case 0:
						ngetValue = port_pin_get_output_level(D20Pins[nFractionValue]);
						dbg_info("\n[pin value is %d]\n", ngetValue);
					break;
//------------------------------------------------------------------
// 0.2.1.x	print chamber parameter
//------------------------------------------------------------------					
					case 1:
						if (nFractionValue < ((int)(sizeof(D20Chamber) / sizeof(int)) * 2 ) )
						{
							if (nFractionValue > 11)
							{
								nZone = 1;
								nFractionValue -= 12;
							}
							
							dblGetValue = fChamber[nFractionValue](0.0f, false, nZone);
							dbg_info("\n[chamber value is %f]\n", dblGetValue);
						}
					break;
//------------------------------------------------------------------
// 0.2.2.x	print power supply parameter
//------------------------------------------------------------------					
					case 2:
						if (nFractionValue < ((int)(sizeof(D20POWERSUPPLY) / sizeof(int)) * 2 ) )
						{
							if (nFractionValue > 12)
							{
								nZone = 1;
								nFractionValue -= 13;
							}
							
						dblGetValue = fPower[nFractionValue](0.0f, false, nZone);
						dbg_info("\n[ps value is %f]\n", dblGetValue);
					}
					break;
//------------------------------------------------------------------
// 0.2.3.x	print pid parameter
//------------------------------------------------------------------					
					case 3:
						if (nFractionValue < ((int)(sizeof(D20PID) / sizeof(int))) )
						{
							/*if (nFractionValue > 8)
							{
								nZone = 1;
								nFractionValue -= 9;
							}
							*/
							dblGetValue = fPid[nFractionValue](0.0f, false, nZone);
							dbg_info("\n[PID value is %f]\n", dblGetValue);
						}
						break;
//------------------------------------------------------------------
// 0.2.4.x	print fan parameter
//------------------------------------------------------------------						
					case 4:
						if (nFractionValue < (int)(sizeof(D20FAN) / sizeof(int)))
						{
							if (nFractionValue > 6)
							{
								nZone = 1;
							}
							dblGetValue = fFan[nFractionValue](0.0f, false, nZone);
							dbg_info("\n[Fan value is %f]\n", dblGetValue);
						}
						break;
//------------------------------------------------------------------
// 0.2.5.x	print control parameter	
//------------------------------------------------------------------
					case 5:
						if (nFractionValue < ((int)(sizeof(D20CONTROL) / sizeof(int))) )
						{
							/*if (nFractionValue > 2)
							{
								nZone = 1;
								nFractionValue -= 3;
							}
							*/
						dblGetValue = fControl[nFractionValue](0.0f, false, nZone);
						dbg_info("\n[value is %f]\n", dblGetValue);
						}
						break;
//------------------------------------------------------------------
// 0.2.6
//------------------------------------------------------------------

//------------------------------------------------------------------
// 0.2.7
//------------------------------------------------------------------

//------------------------------------------------------------------
// 0.2.8	
//------------------------------------------------------------------

//------------------------------------------------------------------
// 0.2.9	print boot msg
//------------------------------------------------------------------
					case 9:
						dbg("\n\n");
						printBootMsg();
					break;		
				}
			    break;
			  }
//------------------------------------------------------------------
// 0.3.x	enablePsDbgMsgs
//------------------------------------------------------------------			  
			  case '3':
				result = ((bParam[0] - '0') != 0);
				//enableManualOverride = result;
				if (result)
				{
					enablePsDbgMsgs = true;
					dbg("\n[debug msgs enabled]\n");
				}
				else
				{
					enablePsDbgMsgs = false;
					dbg("\n[debug msgs disabled]\n");
				}
			  break;
//------------------------------------------------------------------
// 0.4.x	selfTest
//------------------------------------------------------------------			  
			  case '4':
			    result = ((bParam[0] - '0') != 0);
				selfTest();
			  break;
//------------------------------------------------------------------
// 0.5.x	deactivateAutoControl
//------------------------------------------------------------------			  
			  case '5':
			     //dbg("\PID loop control\n");
			     result = ((bParam[0] - '0') != 0);
			     enableManualOverride = result;
			     
			     if (enableManualOverride)
			     {
				     //pidLoop1.inAuto = false;
				     //pidLoop2.inAuto = false;
					 chamber1.deactivateAutoControl = true;
				     chamber2.deactivateAutoControl = true;
				     dbg("\n[PID override enabled]\n");
			     }
			     else
			     {
				     //pidLoop1.inAuto = true;
				     //pidLoop2.inAuto = true;
					 chamber1.deactivateAutoControl = false;
					 chamber2.deactivateAutoControl = false;
				     dbg("\n[PID override disabled]\n");
			     }
			  break;
//------------------------------------------------------------------
// 0.6.x	ps output voltage
//------------------------------------------------------------------			  
			  case '6':
			  {
				 float dblValue = 0.0f;
				 uint8_t nFracSize = 0;
			  
				 int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				 int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
				 
				 nFracSize = strlen(bFraction);
				 dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
				 
				 if (pidLoop1.inAuto == false)
				 {
					 pidLoop1.Output = dblValue;
					 set_setdesiredVoltage(dblValue, true, 0);
					 dbg_info("\n[ps output voltage is %f]\n", dblValue);
				 }
				 /*
				 if (pidLoop2.inAuto == false)
				 {
					 pidLoop2.Output = dblValue;
					 set_setdesiredVoltage(dblValue, true, 1);
					 dbg("\nSet PS 2 Output Voltage\n");
				 }
				 */
			  break;
			  }
//------------------------------------------------------------------
// 0.7.x	enable/disable htxAHeater
//------------------------------------------------------------------			  
			  case '7':
			  {
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10);
				  
				if (nParamValue >= 1) {
					setHeaterState(&htxAHeater, 100);
				} else if (nParamValue <= 0) {
					setHeaterState(&htxAHeater, 0);
				}
				break;
			  }
//------------------------------------------------------------------
// 0.8.x	enable/disable htxBHeater
//------------------------------------------------------------------			  
			  case '8':
			  {
			    int nParamValue = (int)strtol((const char*)bParam, NULL, 10);
				
				if (nParamValue >= 1) {
					setHeaterState(&htxBHeater, 100);
				} else if (nParamValue <= 0) {
					setHeaterState(&htxBHeater, 0);
				}
				break;
			  }
//------------------------------------------------------------------
// 0.9		print firmware version
//------------------------------------------------------------------			  
			  case '9':
			  { 
				  dbg_info("\n[firmwareVersion is %.2x.%.2x]\n", firmwareVersion, firmwareRevision);
				  break;
			  }		  
			  default:
			    dbg("\n[no action]\n");
			  break;
		  }
		  break;	
		}
//------------------------------------------------------------------
// 1.x.x.x	write control parameter
//------------------------------------------------------------------		
		case 1:
		{
			float dblValue = 0.0f;
			uint8_t nFracSize = 0;
			
			int nSetValue = (int)strtol((const char*)bCommand, NULL, 10);
			int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
			int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
			int nZone = 0;
			
			nFracSize = strlen(bFraction);
			dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
			if (nSetValue < (((int)(sizeof(D20CONTROL) / sizeof(int)))))
			{
				/*
				if (nSetValue > 11)
				{
					nZone = 1;
					nSetValue -= 12;
				}
				*/
				fControl[nSetValue](dblValue, true, nZone);
				dbg("\n[parameter set]\n");
			}
			break;
		}
//------------------------------------------------------------------
// 2.x.x.x	write chamber parameter
//------------------------------------------------------------------		
		case 2:
		{ 
			float dblValue = 0.0f;
			uint8_t nFracSize = 0;
			
			int nSetValue = (int)strtol((const char*)bCommand, NULL, 10);
			int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
			int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
			int nZone = 0;
			
			nFracSize = strlen(bFraction);
			dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
			if (nSetValue < (((int)(sizeof(D20Chamber) / sizeof(int)) * 2 )))
			{
				if (nSetValue > 11)
				{
					nZone = 1;
					nSetValue -= 12;
				}
			    fChamber[nSetValue](dblValue, true, nZone);
				dbg("\n[chamber parameter set]\n");
			}
            break;
		}
//------------------------------------------------------------------
// 3.x.x.x	write power supply parameter
//------------------------------------------------------------------		
		case 3:
		{
			float dblValue = 0.0f;
			uint8_t nFracSize = 0;
			
			int nSetValue = (int)strtol((const char*)bCommand, NULL, 10);
			int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
			int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
			int nZone = 0;
			
			nFracSize = strlen(bFraction);
			dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
			if (nSetValue < (((int)(sizeof(D20POWERSUPPLY) / sizeof(int)) * 2 )))
			{
				if (nSetValue > 12)
				{
					nZone = 1;
					nSetValue -= 13;
				}
				
				fPower[nSetValue](dblValue, true, nZone);
				dbg("\n[ps paramter set]\n");
			}
            break;
		}
//------------------------------------------------------------------
// 4.x.x.x	write pid parameter
//------------------------------------------------------------------		
        case 4:
        {
	        float dblValue = 0.0f;
	        uint8_t nFracSize = 0;
	        
	        int nSetValue = (int)strtol((const char*)bCommand, NULL, 10);
	        int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
	        int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
			int nZone = 0;
	        
	        nFracSize = strlen(bFraction);
	        dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));			
			if (nSetValue < ((int)(sizeof(D20PID) / sizeof(int))) )
			{
				/*
				if (nSetValue > 8)
				{
					nZone = 1;
					nSetValue -= 9;
				}
				*/
		        fPid[nSetValue](dblValue, true, nZone);
		        dbg("\n[pid parameter set]\n");
	        }
            break;
        }
//------------------------------------------------------------------
// 5.x.x.x	write fan parameter
//------------------------------------------------------------------		
		case 5:
		{
			float dblValue = 0.0f;
			uint8_t nFracSize = 0;
			
			int nSetValue = (int)strtol((const char*)bCommand, NULL, 10);
			int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
			int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
			int nZone = 0;
			
			nFracSize = strlen(bFraction);
			dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));

			if (nSetValue < (int)(sizeof(D20FAN) / sizeof(int)))
			{
				/*
				if (nSetValue > 6)
				{
					nZone = 1;
				}
				*/
				fFan[nSetValue](dblValue, true, nZone);
				dbg("\n[fan parameter set]\n");
			}
			break;
		}		
		case 6:
		{
			switch (bCommand[0])
			{
//------------------------------------------------------------------
// 6.0.x	enable/disable pid control
//------------------------------------------------------------------				
				case '0':
				//dbg("\PID loop control\n");
				result = ((bParam[0] - '0') != 0);
				//enableManualOverride = result;
				
				if (result)
				{
					//pidLoop1.inAuto = false;
					chamber1.deactivateAutoControl = true;
					chamber2.deactivateAutoControl = true;
					enableSensorEDFI = false;
					dbg("\n[pid override enabled]\n");
				}
				else
				{
					//pidLoop1.inAuto = true;
					chamber1.deactivateAutoControl = false;
					chamber2.deactivateAutoControl = false;
					enableSensorEDFI = true;
					dbg("\n[pid override disabled]\n");
				}
				break;
//------------------------------------------------------------------
// 6.1.x	enable/disable heater override
//------------------------------------------------------------------				
				case '1':
				result = ((bParam[0] - '0') != 0);
				if (result)
				{
					deactivateAutoHeaterControl = true;
					dbg("\n[heater override enabled]\n");
				}
				else
				{
					deactivateAutoHeaterControl = false;
					dbg("\n[heater override disabled]\n");
				}
				break;
//------------------------------------------------------------------
// 6.2.x.x	write output voltage
//------------------------------------------------------------------				
				case '2':
				{
				float dblValue = 0.0f;
				uint8_t nFracSize = 0;
				
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
				
				nFracSize = strlen(bFraction);
				dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
				
				if (chamber1.deactivateAutoControl==true)//(pidLoop1.inAuto == false)
				{
					pidLoop1.Output = dblValue;
					set_setdesiredVoltage(dblValue, true, 0);
					dbg_info("\n[ps output voltage set to %f]\n", dblValue);
				}
				break;
				}
//------------------------------------------------------------------
// 6.3.x.x	
//------------------------------------------------------------------				
				/*
				case '3':
				{
				double dblValue = 0.0f;
				uint8_t nFracSize = 0;
				
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
				
				nFracSize = strlen(bFraction);
				dblValue = (double)nParamValue + (double)((double)nFractionValue/(pow(10,nFracSize)));
				
				if (chamber2.deactivateAutoControl==true)
				{
					pidLoop1.Output = dblValue;
					set_setdesiredVoltage(dblValue, true, 0);
					dbg_info("\n[ps output voltage set to %f]\n", dblValue);
				}				
				break;
				*/
//------------------------------------------------------------------
// 6.4.x	write chamber thermistor beta
//------------------------------------------------------------------				
				case '4':
				{
				float dblValue = 0.0f;
				uint8_t nFracSize = 0;
				
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
				
				nFracSize = strlen(bFraction);
				dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
				chamberThermistorBeta = (uint16_t)dblValue;	
				dbg_info("\n[chamberThermistorBeta set to %d]\n", chamberThermistorBeta);
				writeToEepromFlag = true;
				eepromDataInfo.pAddr = &chamberThermistorBeta;
				eepromDataInfo.size = sizeof(chamberThermistorBeta);
				eepromDataInfo.nonvolatileFlag = true;					
				break;			
				}
//------------------------------------------------------------------
// 6.5.x	write reject thermistor beta
//------------------------------------------------------------------				
				case '5':
				{
				float dblValue = 0.0f;
				uint8_t nFracSize = 0;
					
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
					
				nFracSize = strlen(bFraction);
				dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
				rejectThermistorBeta = (uint16_t)dblValue;
				dbg_info("\n[rejectThermistorBeta set to %d]\n", rejectThermistorBeta);
				writeToEepromFlag = true;
				eepromDataInfo.pAddr = &rejectThermistorBeta;
				eepromDataInfo.size = sizeof(rejectThermistorBeta);
				eepromDataInfo.nonvolatileFlag = true;
				break;
				}
//------------------------------------------------------------------
// 6.6.x	set condensate tray heater pwm duty
//------------------------------------------------------------------				
				case '6':
				{
				//result = ((bParam[0] - '0') != 0);
				float dblValue = 0.0f;
				uint8_t nFracSize = 0;
				
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
				
				nFracSize = strlen(bFraction);
				dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));

				if (deactivateAutoHeaterControl==true)
				{
					setHeaterState(&condHeater, (uint8_t)dblValue);
					dbg_info("\n[cond heater enabled at %d%%]\n", (uint8_t)dblValue);
				}
				else
				{
					dbg("\nno action\n");
				}					
				break;	
				}
//------------------------------------------------------------------
// 6.7.x	set door frame heater pwm duty
//------------------------------------------------------------------				
				case '7':
				{
				//result = ((bParam[0] - '0') != 0);
				float dblValue = 0.0f;
				uint8_t nFracSize = 0;
				
				int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
				int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
				
				nFracSize = strlen(bFraction);
				dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));

				if (deactivateAutoHeaterControl==true)
				{
					setHeaterState(&doorHeater, (uint8_t)dblValue);
					dbg_info("\n[door heater enabled at %d%%]\n", (uint8_t)dblValue);
				}
				else
				{
					dbg("\nno action\n");
				}
				break;
				}		
//------------------------------------------------------------------
// 6.8.x	enable/disable ui setpoint control
//------------------------------------------------------------------
				case '8':
				{
				result = ((bParam[0] - '0') != 0);
				if (result)
				{
					uiSetpointControl = true;
					dbg("\n[uiSetpointControl enabled]\n");
				}
				else
				{
					uiSetpointControl = false;
					dbg("\n[uiSetpointControl disabled]\n");
				}
				break;
				}
//------------------------------------------------------------------
// 6.9.x	enable/disable debug logging mode formatting
//------------------------------------------------------------------
				case '9':
				{
					result = ((bParam[0] - '0') != 0);
					if (result)
					{
						enableDbgLoggingMode = true;
						dbg("\n[DbgLoggingMode enabled]\n");
					}
					else
					{
						enableDbgLoggingMode = false;
						dbg("\n[DbgLoggingMode disabled]\n");
					}
					break;
				}
										
				default:
				dbg("\nno action\n");
				break;
				
			}
			break;
		}		
		case 7:
		{
			switch (bCommand[0])
			{
//------------------------------------------------------------------
// 7.0.x	software reset
//------------------------------------------------------------------				
				case '0':
				result = ((bParam[0] - '0') != 0);
				
				if (result)
				{
					dbg("\n[soft reset now!]\n");
					system_reset();
				}
				else
				{
					dbg("\n[no action]\n");
				}
				break;
//------------------------------------------------------------------
// 7.1.x	enable/disable temp sensor override
//------------------------------------------------------------------				
				case '1':
				result = ((bParam[0] - '0') != 0);
				if (result)
				{
					enableTempOverride = true;
					dbg("\n[sensor override enabled]\n");
				}
				else
				{
					enableTempOverride = false;
					dbg("\n[sensor override disabled]\n");
				}
				break;
//------------------------------------------------------------------
// 7.2.x	enable/disable fan override
//------------------------------------------------------------------				
				case '2':
				result = ((bParam[0] - '0') != 0);
				enableFanOverride = result;
				if (enableFanOverride)
				{
					enableSensorEDFI = false;
					dbg("\n[fan override enabled]\n");
				}
				else
				{
					enableSensorEDFI = true;
					dbg("\n[fan override disabled]\n");
				}
				break;
//------------------------------------------------------------------
// 7.3		 
//------------------------------------------------------------------				
				case '3':
				//dbg_info("\n[humidity is %4.2f]\n", humidityPercentage);
				break;
//------------------------------------------------------------------
// 7.4		print condensate temp
//------------------------------------------------------------------				
				case '4':
				dbg_info("\n[condensate temp is %4.2f]\n", condensateTemp);
				break;	
//------------------------------------------------------------------
// 7.5		print ui firmware version
//------------------------------------------------------------------				
				case '5':
				dbg_info("\n[ui firmware version is %X]\n", UIfwVersion);
				break;	
//------------------------------------------------------------------
// 7.6		print ui version string (long version)
//------------------------------------------------------------------
				case '6':
				dbg_info("\n[ui firmware version is %s]\n", strUiFwVer);
				break;				
//------------------------------------------------------------------
// 7.7.x	write ps1.phpOutputDutyOffset
//------------------------------------------------------------------
				case '7':
				{
					float dblValue = 0.0f;
					uint8_t nFracSize = 0;
					
					int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
					int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
					
					nFracSize = strlen(bFraction);
					dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
					ps1.phpOutputDutyOffset = dblValue;
					dbg_info("\n[ps1.phpOutputDutyOffset set to %4.2f]\n", ps1.phpOutputDutyOffset);
					writeToEepromFlag = true;
					eepromDataInfo.pAddr = &ps1.phpOutputDutyOffset;
					eepromDataInfo.size = sizeof(ps1.phpOutputDutyOffset);
					eepromDataInfo.nonvolatileFlag = true;
					break;
				}
//------------------------------------------------------------------
// 7.8.x	write ps2.phpOutputDutyOffset
//------------------------------------------------------------------
				case '8':
				{
					float dblValue = 0.0f;
					uint8_t nFracSize = 0;
					
					int nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
					int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
					
					nFracSize = strlen(bFraction);
					dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
					ps2.phpOutputDutyOffset = dblValue;
					dbg_info("\n[ps2.phpOutputDutyOffset set to %4.2f]\n", ps2.phpOutputDutyOffset);
					writeToEepromFlag = true;
					eepromDataInfo.pAddr = &ps2.phpOutputDutyOffset;
					eepromDataInfo.size = sizeof(ps2.phpOutputDutyOffset);
					eepromDataInfo.nonvolatileFlag = true;
					break;
				}
//------------------------------------------------------------------
// 7.9		print a single debug message string
//------------------------------------------------------------------
				case '9':
				{
					bool prev_val = enableDbgLoggingMode;
					enableDbgLoggingMode = true;
					printDbgMsgs();
					enableDbgLoggingMode = prev_val;	
					break;
				}
					
				default:
				dbg("\n[no action]\n");
				break;
			}
			break;
		}
		case 8:
		{	
			uint8_t bTemp[3];
			int nParamValue = (int)strtol((const char*)bParam, NULL, 10);
								
			switch (bCommand[0])
			{	
//------------------------------------------------------------------
// 8.0.x	write runSelfTestOnPor flag
//------------------------------------------------------------------
				case '0':
				{
					float dblValue = 0.0f;
					uint8_t nFracSize = 0;
					
					nParamValue = (int)strtol((const char*)bParam, NULL, 10) * nSign;
					int nFractionValue = (int)strtol((const char*)bFraction, NULL, 10) * nSign;
					
					nFracSize = strlen(bFraction);
					dblValue = (float)nParamValue + (float)((float)nFractionValue/(pow(10,nFracSize)));
					runSelfTestOnPOR = (uint8_t)dblValue;
					dbg_info("\n[runSelfTestOnPor set to %d]\n", runSelfTestOnPOR);
					writeToEepromFlag = true;
					eepromDataInfo.pAddr = &runSelfTestOnPOR;
					eepromDataInfo.size = sizeof(runSelfTestOnPOR);
					eepromDataInfo.nonvolatileFlag = true;
					break;
				}					
//------------------------------------------------------------------
// 8.1.x	write MAC address
//------------------------------------------------------------------				
				case '1':
					// mac addr
					for (uint8_t n=0; n<6; n++)
					{
						if ( (bParam[2*n]>='0' && bParam[2*n]<='9') ||
							 (bParam[2*n]>='A' && bParam[2*n]<='F') )
						{
							bTemp[0] = bParam[2*n];
						}
						else
						{
							dbg("\nsyntax error\n");
							return;
						}
						if ( (bParam[2*n+1]>='0' && bParam[2*n+1]<='9') ||
							 (bParam[2*n+1]>='A' && bParam[2*n+1]<='F') )
						{
							bTemp[1] = bParam[2*n+1];
						}
						else
						{
							dbg("\nsyntax error\n");
							return;
						}						
						bTemp[2] = '\0';
						MAC_address[n] = (int)strtol((const char*)bTemp, NULL, 16);						
					}			
					eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[12] = MAC_address[0];
					eepromPageData[13] = MAC_address[1];
					eepromPageData[14] = MAC_address[2];
					eepromPageData[15] = MAC_address[3];
					eepromPageData[16] = MAC_address[4];
					eepromPageData[17] = MAC_address[5];
					eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					dbg("\n[mac addr is programmed]\n");
					eepromStatus |= EEPROM_MACADDR_WRITTEN;
					eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
					eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
					eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
					eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					break;	
//------------------------------------------------------------------
// 8.2.x	write serial number
//------------------------------------------------------------------							
				case '2':
					// serial number
					eeprom_emulator_read_page(EEPROM_PAGE2, eepromPageData);
					eepromPageData[0] = bParam[0];
					eepromPageData[1] = bParam[1];
					eepromPageData[2] = bParam[2];
					eepromPageData[3] = bParam[3];
					eepromPageData[4] = bParam[4];
					eepromPageData[5] = bParam[5];
					eepromPageData[6] = bParam[6];
					eepromPageData[7] = bParam[7];
					eepromPageData[8] = bParam[8];
					eepromPageData[9] = bParam[9];
					eepromPageData[10] = bParam[10];
					eepromPageData[11] = bParam[11];
					eepromPageData[12] = bParam[12];
					eepromPageData[13] = bParam[13];
					eepromPageData[14] = bParam[14];
					eepromPageData[15] = '\0';
					memcpy(strSerial, eepromPageData, SERIAL_SIZE);
					eeprom_emulator_write_page(EEPROM_PAGE2, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					dbg("\n[serial number is programmed]\n");
					eepromStatus |= EEPROM_SERIAL_WRITTEN;
					eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
					eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
					eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
					eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					eeprom_emulator_commit_page_buffer();					
					break;
//------------------------------------------------------------------
// 8.3.x	write model number
//------------------------------------------------------------------					
				case '3':
					// model number
					eeprom_emulator_read_page(EEPROM_PAGE1, eepromPageData);
					eepromPageData[0] = bParam[0];
					eepromPageData[1] = bParam[1];
					eepromPageData[2] = bParam[2];
					eepromPageData[3] = bParam[3];
					eepromPageData[4] = bParam[4];
					eepromPageData[5] = bParam[5];
					eepromPageData[6] = bParam[6];
					eepromPageData[7] = bParam[7];
					eepromPageData[8] = bParam[8];
					eepromPageData[9] = bParam[9];
					eepromPageData[10] = bParam[10];
					eepromPageData[11] = bParam[11];
					eepromPageData[12] = '\0';
					memcpy(strModel, eepromPageData, MODEL_SIZE);					
					eeprom_emulator_write_page(EEPROM_PAGE1, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					dbg("\n[model number is programmed]\n");
					eepromStatus |= EEPROM_MODEL_WRITTEN;
					eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
					eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
					eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
					eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					break;
//------------------------------------------------------------------
// 8.4		print MAC address
//------------------------------------------------------------------					
				case '4':
					dbg_info("\n[mac addr is %x:%x:%x:%x:%x:%x]\n",
						MAC_address[0], MAC_address[1], MAC_address[2],
						MAC_address[3], MAC_address[4], MAC_address[5] );	
					break;	
//------------------------------------------------------------------
// 8.5		print serial number
//------------------------------------------------------------------						
				case '5':
					dbg_info("\n[serial number is %s]\n", strSerial);	
					break;	
//------------------------------------------------------------------
// 8.6		print model number
//------------------------------------------------------------------										
				case '6':
					dbg_info("\n[model number is %s]\n", strModel);
					break;		
//------------------------------------------------------------------
// 8.7		initiate defrost cycle
//------------------------------------------------------------------					
				case '7':
					setDefrostState(DEFROST_START);
					break;	
//------------------------------------------------------------------
// 8.8.x		write mfgTestModeFlag2
//------------------------------------------------------------------					
				case '8':
					mfgTestModeFlag2 = nParamValue & 0x01;
					eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[23]  = mfgTestModeFlag2;
					//eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					//eeprom_emulator_commit_page_buffer();
					eepromStatus |= EEPROM_MFGTESTFLAG2_WRITTEN;
					//eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
					eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
					eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
					eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					dbg("\n[mfgTestModeFlag2 is programmed]\n");
					break;
//------------------------------------------------------------------
// 8.9		print mfgTestModeFlag2
//------------------------------------------------------------------					
				case '9':
					dbg_info("\n[mfgTestModeFlag2 is %x]\n", mfgTestModeFlag2);
					break;																													
				default:
					dbg("\n[no action]\n");
					break;				
			}
			break;
		}	
		case 9:
		{
			int nParamValue = (int)strtol((const char*)bParam, NULL, 10);
			
			switch (bCommand[0])
			{
//------------------------------------------------------------------
// 9.0.x	write defrost freq max hours
//------------------------------------------------------------------				
				case '0':
					defrostFreqMaxHrs = nParamValue;
					dbg_info("\n[defrostFreqMaxHrs set to %i]\n", defrostFreqMaxHrs);
					writeToEepromFlag = true;
					eepromDataInfo.pAddr = &defrostFreqMaxHrs;
					eepromDataInfo.size = sizeof(defrostFreqMaxHrs);
					eepromDataInfo.nonvolatileFlag = true;					
					break;
//------------------------------------------------------------------
// 9.1.x	print defrost freq max hours
//------------------------------------------------------------------					
				case '1':
					dbg_info("\n[defrostFreqMaxHrs is %i]\n", defrostFreqMaxHrs);
					break;	
//------------------------------------------------------------------
// 9.2.x	write defrost frequency hrs
//------------------------------------------------------------------					
				case '2':
					defrostFreqHrs = nParamValue;
					dbg_info("\n[defrostFreqHrs set to %i]\n", defrostFreqHrs);
					writeToEepromFlag = true;
					eepromDataInfo.pAddr = &defrostFreqHrs;
					eepromDataInfo.size = sizeof(defrostFreqHrs);
					eepromDataInfo.nonvolatileFlag = true;					
					break;
//------------------------------------------------------------------
// 9.3.x	print defrost frequency hrs
//------------------------------------------------------------------					
				case '3':
					dbg_info("\n[defrostFreqHrs is %i]\n", defrostFreqHrs);
					break;
//------------------------------------------------------------------
// 9.4.x	write defrost duration minutes
//------------------------------------------------------------------					
				case '4':
					defrostDurationMins = nParamValue;
					dbg_info("\n[defrostDurationMins set to %i]\n", defrostDurationMins);
					writeToEepromFlag = true;
					eepromDataInfo.pAddr = &defrostDurationMins;
					eepromDataInfo.size = sizeof(defrostDurationMins);
					eepromDataInfo.nonvolatileFlag = true;					
					break;
//------------------------------------------------------------------
// 9.5		print defrost duration minutes
//------------------------------------------------------------------					
				case '5':
					dbg_info("\n[defrostDurationMins is %i]\n", defrostDurationMins);
					break;	
//------------------------------------------------------------------
// 9.6		clear error register
//------------------------------------------------------------------						
				case '6':
					errorRegister = ERROR_NONE;
					dbg("\n[errorRegister cleared]\n");
					break;
//------------------------------------------------------------------
// 9.7		print eeprom status
//------------------------------------------------------------------					
				case '7':
					dbg_info("\n[eepromStatus is %lx]\n", eepromStatus);
					break;
//------------------------------------------------------------------
// 9.8.x	write mfgTestModeFlag1
//------------------------------------------------------------------						
				case '8':
					mfgTestModeFlag1 = nParamValue & 0x01;
					eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[22]  = mfgTestModeFlag1;
					//eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					//eeprom_emulator_commit_page_buffer();
					eepromStatus |= EEPROM_MFGTESTFLAG1_WRITTEN;
					//eeprom_emulator_read_page(EEPROM_PAGE0, eepromPageData);
					eepromPageData[19]  = (~eepromStatus) & 0x0000ff;
					eepromPageData[20]  = ((~eepromStatus) & 0x00ff00) >> 8;
					eepromPageData[21]  = ((~eepromStatus) & 0xff0000) >> 16;
					eeprom_emulator_write_page(EEPROM_PAGE0, eepromPageData);
					eeprom_emulator_commit_page_buffer();
					dbg("\n[mfgTestModeFlag1 is programmed]\n");
					break;
//------------------------------------------------------------------
// 9.9		print mfgTestModeFlag1
//------------------------------------------------------------------									
				case '9':
					dbg_info("\n[mfgTestModeFlag1 is %x]\n", mfgTestModeFlag1);
					break;																							
				default:
					dbg("\n[no action]\n");
				break;
			}
			break;
		}			
	}
	
	return;
}