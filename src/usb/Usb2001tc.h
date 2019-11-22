/*
 * Usb2001tc.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_USB2001TC_H_
#define USB_USB2001TC_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb2001tc: public UsbDaqDevice
{
public:
	Usb2001tc(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb2001tc();

	virtual void flashLed(int flashCount) const;
};

} /* namespace ul */

#endif /* USB_USB2001TC_H_ */
