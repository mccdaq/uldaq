/*
 * DioUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSBBASE_H_
#define USB_DIO_DIOUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../DioDevice.h"

namespace ul
{

class UL_LOCAL DioUsbBase: public DioDevice
{
public:
	DioUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~DioUsbBase();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

private:
	const UsbDaqDevice&  mUsbDevice;
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSBBASE_H_ */
