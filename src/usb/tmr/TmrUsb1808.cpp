/*
 * TmrUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "TmrUsb1808.h"
#include "../daqi/DaqIUsb1808.h"

namespace ul
{

TmrUsb1808::TmrUsb1808(const UsbDaqDevice& daqDevice, int numTimers) : TmrUsbBase(daqDevice)
{
	mTmrInfo.setMinFrequency(daqDev().getClockFreq() / 0x00ffffffff);
	mTmrInfo.setMaxFrequency(daqDev().getClockFreq() / 2);
	mTmrInfo.setNumTimers(numTimers);
	mTmrInfo.setTimerType(TMR_ADVANCED);
	mTmrInfo.setScanOptions(SO_EXTTRIGGER|SO_RETRIGGER);
	mTmrInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	for(int tmr = 0; tmr < numTimers; tmr++)
		mIdleState.push_back(0);

}

TmrUsb1808::~TmrUsb1808()
{

}

void TmrUsb1808::initialize()
{
	try
	{
		unsigned char timerCtrlVal;

		for(int timerNum = 0; timerNum < mTmrInfo.getNumTimers(); timerNum++)
		{
			timerCtrlVal = 0;

			daqDev().queryCmd(CMD_TMR_CTRL, 0, timerNum, &timerCtrlVal, sizeof(timerCtrlVal));

			mIdleState[timerNum] =  (timerCtrlVal >> 2) & 0x01;
		}
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void TmrUsb1808::tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options)
{
	check_TmrPulseOutStart_Args(timerNum, frequency, dutyCycle, pulseCount, initialDelay, idleState, options);

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

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

	unsigned char enable_timer = 1;

	if((options & PO_EXTTRIGGER) || (options & PO_RETRIGGER))
	{
		daqDev().setupTrigger(FT_TMR, (ScanOption) options);

		enable_timer = 0;
	}

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
	control = enable_timer | (idleStateVal & 0x00000001) << 2;
	mIdleState[timerNum] = idleStateVal;

	if(options & SO_EXTTRIGGER)
		control |= 0x01 << 4;

	if(options & SO_RETRIGGER)
	{
		control |= 0x01 << 4;
		control |= 0x01 << 6;
	}

	daqDev().sendCmd(CMD_TMR_CTRL, control, timerNum, NULL, 0);

}

void TmrUsb1808::tmrPulseOutStop(int timerNum)
{
	check_TmrPulseOutStop_Args(timerNum);

	unsigned char control = (mIdleState[timerNum] & 0x00000001) << 2;

	daqDev().sendCmd(CMD_TMR_CTRL, control, timerNum, NULL, 0);
}

void TmrUsb1808::tmrPulseOutStatus(int timerNum, TmrStatus* status)
{
	check_TmrOutStatus_Args(timerNum);

	unsigned char state = 0;

	// Note: this command currently does not read the counter state correctly. FW/FPGA change required

	daqDev().queryCmd(CMD_TMR_CTRL, 0, timerNum, &state, sizeof(state));

	unsigned char tmrStatus = (state >> 1) & 0x01;

	*status = tmrStatus == 0 ? TMRS_IDLE : TMRS_RUNNING;
}

} /* namespace ul */
