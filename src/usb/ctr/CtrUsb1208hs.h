/*
 * CtrUsb1208hs.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSB1208HS_H_
#define USB_CTR_CTRUSB1208HS_H_

#include "CtrUsbBase.h"

namespace ul
{

class UL_LOCAL CtrUsb1208hs: public CtrUsbBase
{
public:
	CtrUsb1208hs(const UsbDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsb1208hs();

	virtual unsigned long long cIn(int ctrNum);
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);

private:
	enum { CMD_CTR = 0x20 };

};

} /* namespace ul */

#endif /* USB_CTR_CTRUSB1208HS_H_ */
