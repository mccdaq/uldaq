/*
 * TmrUsb1208hs.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_TMR_TMRUSB1208HS_H_
#define USB_TMR_TMRUSB1208HS_H_

#include "TmrUsbBase.h"

namespace ul
{

class UL_LOCAL TmrUsb1208hs: public TmrUsbBase
{
public:
	TmrUsb1208hs(const UsbDaqDevice& daqDevice, int numTimers);
	virtual ~TmrUsb1208hs();

	virtual void initialize();

	virtual void tmrPulseOutStart(int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options);
	virtual void tmrPulseOutStop(int timerNum);
	virtual void tmrPulseOutStatus(int timerNum, TmrStatus* status);

private:
	enum { CMD_TMR_CTRL = 0x28, CMD_TMR_PARAMS = 0x2D };

	std::vector<unsigned char> mIdleState;
};

} /* namespace ul */

#endif /* USB_TMR_TMRUSB1208HS_H_ */
