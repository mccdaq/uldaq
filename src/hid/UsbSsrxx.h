/*
 * UsbSsrxx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBSSRXX_H_
#define HID_USBSSRXX_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbSsrxx: public HidDaqDevice
{
public:
	UsbSsrxx(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbSsrxx();
};

} /* namespace ul */

#endif /* HID_USBSSRXX_H_ */
