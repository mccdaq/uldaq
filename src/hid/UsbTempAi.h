/*
 * UsbTempAi.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBTEMPAI_H_
#define HID_USBTEMPAI_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbTempAi: public HidDaqDevice
{
public:
	UsbTempAi(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbTempAi();
};

} /* namespace ul */

#endif /* HID_USBTEMPAI_H_ */
