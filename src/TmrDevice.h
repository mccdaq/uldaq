/*
 * TmrDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef TMRDEVICE_H_
#define TMRDEVICE_H_

#include "interfaces/UlTmrDevice.h"
#include "ul_internal.h"
#include "IoDevice.h"
#include "TmrInfo.h"

namespace ul
{

class UL_LOCAL TmrDevice: public IoDevice, public UlTmrDevice
{
public:
	TmrDevice(const DaqDevice& daqDevice);
	virtual ~TmrDevice();

	virtual const UlTmrInfo& getTmrInfo() { return mTmrInfo;}

	virtual void tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options);
	virtual void tmrPulseOutStop(int timerNum);
	virtual void tmrPulseOutStatus(int timerNum, TmrStatus* status);
	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);

protected:
	virtual void check_TmrPulseOutStart_Args(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options);
	virtual void check_TmrPulseOutStop_Args(int timerNum);
	virtual void check_TmrOutStatus_Args(int timerNum);
	virtual void check_TmrSetTrigger_Args(TriggerType trigtype, int trigChan, double level, double variance, unsigned int retriggerCount);

protected:
	TmrInfo mTmrInfo;
};

} /* namespace ul */

#endif /* TMRDEVICE_H_ */
