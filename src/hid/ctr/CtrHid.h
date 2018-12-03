/*
 * CtrHid.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_CTR_CTRHID_H_
#define HID_CTR_CTRHID_H_

#include "CtrHidBase.h"

namespace ul
{

class UL_LOCAL CtrHid: public CtrHidBase
{
public:
	CtrHid(const HidDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrHid();

	virtual unsigned long long cIn(int ctrNum);
	void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);

private:
	enum { CMD_CINIT = 0x20, CMD_CIN = 0x21 };
};

} /* namespace ul */

#endif /* HID_CTR_CTRHID_H_ */
