/*
 * UlTmrDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULTMRDEVICE_H_
#define INTERFACES_ULTMRDEVICE_H_

#include "../uldaq.h"
#include "UlTmrInfo.h"

namespace ul
{

class UlTmrDevice
{
public:
	virtual ~UlTmrDevice() {};

	virtual const UlTmrInfo& getTmrInfo() = 0;

	virtual void tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options) = 0;
	virtual void tmrPulseOutStop(int timerNum) = 0;
	virtual void tmrPulseOutStatus(int timerNum, TmrStatus* status) = 0;
	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULTMRDEVICE_H_ */
