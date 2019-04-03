/*
 * TmrUsbQuad08.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "TmrUsbQuad08.h"
#include "../UsbIotech.h"

namespace ul
{

TmrUsbQuad08::TmrUsbQuad08(const UsbDaqDevice& daqDevice, int numTimers) : TmrUsbBase(daqDevice)
{
	mTmrInfo.setMinFrequency(daqDev().getClockFreq() / 0x00ffffffff);
	mTmrInfo.setMaxFrequency(daqDev().getClockFreq() / 2);
	mTmrInfo.setNumTimers(numTimers);
	mTmrInfo.setTimerType(TMR_ADVANCED);

	for(int tmr = 0; tmr < numTimers; tmr++)
		mTmrStatus.push_back(TMRS_IDLE);
}

TmrUsbQuad08::~TmrUsbQuad08()
{

}

void TmrUsbQuad08::initialize()
{
	try
	{
		for(int timerNum = 0; timerNum < mTmrInfo.getNumTimers(); timerNum++)
		{
			tmrPulseOutStop(timerNum); // need to start from a known state for the tmrPulseOutStatus function
		}
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void TmrUsbQuad08::tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options)
{
	check_TmrPulseOutStart_Args(timerNum, frequency, dutyCycle, pulseCount, initialDelay, idleState, options);

	tmrPulseOutStop(timerNum);

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

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)(dutyCycleDivisor & 0xffff), UsbIotech::HWRegTimerDivisor + timerNum, NULL, 0);
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)((dutyCycleDivisor >> 16) & 0xffff), UsbIotech::HWRegTimerDivisor + timerNum, NULL, 0);
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)(scanDivisor & 0xffff), UsbIotech::HWRegTimerDivisor + timerNum, NULL, 0);
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)((scanDivisor >> 16) & 0xffff), UsbIotech::HWRegTimerDivisor + timerNum, NULL, 0);

	unsigned short controlVal = 1;

	if(timerNum == 1)
		controlVal = 0x11;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, controlVal, UsbIotech::HWRegCtrTmrControl, NULL, 0);

	mTmrStatus[timerNum] = TMRS_RUNNING;
}

void TmrUsbQuad08::tmrPulseOutStop(int timerNum)
{
	check_TmrPulseOutStop_Args(timerNum);

	unsigned short controlVal = 0;

	if(timerNum == 1)
		controlVal = 0x10;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, controlVal, UsbIotech::HWRegCtrTmrControl, NULL, 0);

	mTmrStatus[timerNum] = TMRS_IDLE;

}

void TmrUsbQuad08::tmrPulseOutStatus(int timerNum, TmrStatus* status)
{
	check_TmrOutStatus_Args(timerNum);

	*status = mTmrStatus[timerNum];
}

} /* namespace ul */

