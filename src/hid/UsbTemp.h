/*
 * UsbTemp.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBTEMP_H_
#define HID_USBTEMP_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbTemp: public HidDaqDevice
{
public:
	UsbTemp(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbTemp();
};

} /* namespace ul */

#endif /* HID_USBTEMP_H_ */
