/*
 * UsbDio96h.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBDIO96H_H_
#define HID_USBDIO96H_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbDio96h: public HidDaqDevice
{
public:
	UsbDio96h(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbDio96h();
};

} /* namespace ul */

#endif /* HID_USBDIO96H_H_ */
