/*
 * CtrUsb24xx.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSB24XX_H_
#define USB_CTR_CTRUSB24XX_H_

#include "CtrUsbBase.h"

namespace ul
{

class UL_LOCAL CtrUsb24xx: public CtrUsbBase
{
public:
	CtrUsb24xx(const UsbDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsb24xx();

	virtual unsigned long long cIn(int ctrNum);
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);

private:
	enum { CMD_CTR = 0x20 };

};

} /* namespace ul */

#endif /* USB_CTR_CTRUSB24XX_H_ */
