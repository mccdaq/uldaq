/*
 * CtrUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSBBASE_H_
#define USB_CTR_CTRUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../CtrDevice.h"

namespace ul
{

class UL_LOCAL CtrUsbBase: public CtrDevice
{
public:
	CtrUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~CtrUsbBase();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

private:
	const UsbDaqDevice&  mUsbDevice;
};

} /* namespace ul */

#endif /* USB_CTR_CTRUSBBASE_H_ */
