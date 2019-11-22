/*
 * CtrUsbDio24.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef HID_CTR_CTRUSBDIO24_H_
#define HID_CTR_CTRUSBDIO24_H_

#include "CtrHidBase.h"

namespace ul
{

class UL_LOCAL CtrUsbDio24: public CtrHidBase
{
public:
	CtrUsbDio24(const HidDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsbDio24();

	virtual unsigned long long cIn(int ctrNum);
	void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);

private:
	enum { CMD_CIN = 0x04, CMD_CINIT = 0x05};
};

} /* namespace ul */

#endif /* HID_CTR_CTRUSBDIO24_H_ */
