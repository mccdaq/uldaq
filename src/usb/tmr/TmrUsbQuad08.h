/*
 * TmrUsbQuad08.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_TMR_TMRUSBQUAD08_H_
#define USB_TMR_TMRUSBQUAD08_H_

#include "TmrUsbBase.h"

namespace ul
{

class UL_LOCAL TmrUsbQuad08: public TmrUsbBase
{
public:
	TmrUsbQuad08(const UsbDaqDevice& daqDevice, int numTimers);
	virtual ~TmrUsbQuad08();

	virtual void initialize();

	virtual void tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options);
	virtual void tmrPulseOutStop(int timerNum);
	virtual void tmrPulseOutStatus(int timerNum, TmrStatus* status);

private:

	std::vector<TmrStatus> mTmrStatus;
};

} /* namespace ul */

#endif /* USB_TMR_TMRUSBQUAD08_H_ */
