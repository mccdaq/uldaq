/*
 * TmrDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */


#include <limits.h>
#include <math.h>
#include <algorithm>
#include <bitset>

#include "TmrDevice.h"
#include "UlException.h"

namespace ul
{

TmrDevice::TmrDevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlTmrDevice()
{

}

TmrDevice::~TmrDevice()
{

}


void TmrDevice::tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void TmrDevice::tmrPulseOutStop(int timerNum)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void TmrDevice::tmrPulseOutStatus(int timerNum, TmrStatus* status)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void TmrDevice::setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	check_TmrSetTrigger_Args(type, trigChan, level, variance, retriggerCount);

	mTrigCfg.type = type;
	mTrigCfg.trigChan = trigChan;
	mTrigCfg.level = round(level);
	mTrigCfg.variance = round(variance);
	mTrigCfg.retrigCount = retriggerCount;
}

void TmrDevice::check_TmrPulseOutStart_Args(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options)
{
	if(timerNum < 0 || timerNum >= mTmrInfo.getNumTimers())
		throw UlException(ERR_BAD_TMR);

	if (frequency == NULL || (*frequency > mTmrInfo.getMaxFrequency()) || (*frequency < mTmrInfo.getMinFrequency()))
		throw UlException(ERR_BAD_FREQUENCY);

	if(dutyCycle  == NULL || (*dutyCycle >= 1 || *dutyCycle <= 0))
		throw UlException(ERR_BAD_DUTY_CYCLE);

	if(initialDelay == NULL)
		throw UlException(ERR_BAD_INITIAL_DELAY);

	double clockFreq = mDaqDevice.getClockFreq();

	unsigned long long  delayPulseCount = *initialDelay * clockFreq;

	if(delayPulseCount > UINT_MAX)
		throw UlException(ERR_BAD_INITIAL_DELAY);

	if(~mTmrInfo.getScanOptions() & options)
		throw UlException(ERR_BAD_OPTION);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void TmrDevice::check_TmrPulseOutStop_Args(int timerNum)
{
	if(timerNum < 0 || timerNum >= mTmrInfo.getNumTimers())
		throw UlException(ERR_BAD_TMR);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void TmrDevice::check_TmrOutStatus_Args(int timerNum)
{
	if(timerNum < 0 || timerNum >= mTmrInfo.getNumTimers())
		throw UlException(ERR_BAD_TMR);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void TmrDevice::check_TmrSetTrigger_Args(TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount)
{
	if(mTmrInfo.supportsTrigger())
	{
		if(!(mTmrInfo.getTriggerTypes() & trigType))
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mTmrInfo.getScanOptions() & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}


} /* namespace ul */
