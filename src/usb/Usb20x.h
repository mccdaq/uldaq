/*
 * Usb20x.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USB20X_H_
#define USB_USB20X_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb20x: public UsbDaqDevice
{
public:
	Usb20x(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb20x();
};

} /* namespace ul */

#endif /* USB_USB20X_H_ */
