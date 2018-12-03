/*
 * UsbErbxx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBERBXX_H_
#define HID_USBERBXX_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbErbxx: public HidDaqDevice
{
public:
	UsbErbxx(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbErbxx();
};

} /* namespace ul */

#endif /* HID_USBERBXX_H_ */
