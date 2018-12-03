/*
 * UsbPdiso8.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBPDISO8_H_
#define HID_USBPDISO8_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbPdiso8: public HidDaqDevice
{
public:
	UsbPdiso8(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbPdiso8();
};

} /* namespace ul */

#endif /* HID_USBPDISO8_H_ */
