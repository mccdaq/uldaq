/*
 * TmrUsb1208hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "TmrUsb1208hs.h"

namespace ul
{

TmrUsb1208hs::TmrUsb1208hs(const UsbDaqDevice& daqDevice, int numTimers) : TmrUsbBase(daqDevice)
{
	mTmrInfo.setMinFrequency(daqDev().getClockFreq() / 0x00ffffffff);
	mTmrInfo.setMaxFrequency(daqDev().getClockFreq() / 2);
	mTmrInfo.setNumTimers(numTimers);
	mTmrInfo.setTimerType(TMR_ADVANCED);

	for(int tmr = 0; tmr < numTimers; tmr++)
		mIdleState.push_back(0);

}

TmrUsb1208hs::~TmrUsb1208hs()
{

}

void TmrUsb1208hs::initialize()
{
	try
	{
		unsigned char tmrCtrlVa = 0;

		for(int timerNum = 0; timerNum < mTmrInfo.getNumTimers(); timerNum++)
		{
			daqDev().queryCmd(CMD_TMR_CTRL, 0, timerNum, &tmrCtrlVa, sizeof(tmrCtrlVa));

			mIdleState[timerNum] =  (tmrCtrlVa >> 2) & 0x01;
		}
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void TmrUsb1208hs::tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options)
{
	check_TmrPulseOutStart_Args(timerNum, frequency, dutyCycle, pulseCount, initialDelay, idleState, options);

	double clockFreq = daqDev().getClockFreq();
	unsigned int  scanDivisor = (unsigned int) ((clockFreq / *frequency) + 0.5);
	*frequency = clockFreq / scanDivisor;
	scanDivisor--;

	double scanPeriod = 1 / *frequency;

	unsigned int dutyCycleDivisor = (unsigned int) (*dutyCycle * (clockFreq / *frequency) + 0.5);

	if(dutyCycleDivisor == (scanDivisor + 1))
		dutyCycleDivisor--;
	else if(dutyCycleDivisor == 0)
		dutyCycleDivisor++;

	double highPeriod =  dutyCycleDivisor / clockFreq;
	dutyCycleDivisor--;

	*dutyCycle = highPeriod / scanPeriod;

	unsigned long long  delayPulseCount = *initialDelay * clockFreq;

	*initialDelay = delayPulseCount / clockFreq;

	unsigned char control = (mIdleState[timerNum] & 0x00000001) << 2;

	if(pulseCount > 0 || *initialDelay > 0)
		daqDev().sendCmd(CMD_TMR_CTRL, control, timerNum, NULL, 0);

#pragma pack(1)
	struct
	{
		unsigned int period;
		unsigned int pulseWidth;
		unsigned int count;
		unsigned int delay;
	} params;
#pragma pack()


	params.period = Endian::cpu_to_le_ui32(scanDivisor);
	params.pulseWidth = Endian::cpu_to_le_ui32(dutyCycleDivisor);
	params.count = Endian::cpu_to_le_ui32(pulseCount);
	params.delay = Endian::cpu_to_le_ui32(delayPulseCount);

	daqDev().sendCmd(CMD_TMR_PARAMS, 0, timerNum, (unsigned char*)&params,  sizeof(params));

	unsigned char idleStateVal = idleState == TMRIS_HIGH ? 1 : 0;
	control = 1 | (idleStateVal & 0x00000001) << 2;
	mIdleState[timerNum] = idleStateVal;

	daqDev().sendCmd(CMD_TMR_CTRL, control, timerNum, NULL, 0);

}

void TmrUsb1208hs::tmrPulseOutStop(int timerNum)
{
	check_TmrPulseOutStop_Args(timerNum);

	unsigned char control = (mIdleState[timerNum] & 0x00000001) << 2;

	daqDev().sendCmd(CMD_TMR_CTRL, control, timerNum, NULL, 0);
}

void TmrUsb1208hs::tmrPulseOutStatus(int timerNum, TmrStatus* status)
{
	check_TmrOutStatus_Args(timerNum);

	unsigned char state = 0;

	daqDev().queryCmd(CMD_TMR_CTRL, 0, timerNum, &state, sizeof(state));

	unsigned char tmrStatus = (state >> 1) & 0x01;

	*status = tmrStatus == 0 ? TMRS_IDLE : TMRS_RUNNING;
}

} /* namespace ul */
