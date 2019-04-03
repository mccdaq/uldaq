/*
 * UsbQuadxx.h
 *
 *  Author: Measurement Computing Corporation
 */

#ifndef USB_USBQUAD08_H_
#define USB_USBQUAD08_H_

#include "UsbIotech.h"

namespace ul
{

class UL_LOCAL UsbQuad08: public UsbIotech
{
public:
	UsbQuad08(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbQuad08();

	virtual void flashLed(int flashCount) const;
};

} /* namespace ul */

#endif /* USB_USBQUAD08_H_ */
