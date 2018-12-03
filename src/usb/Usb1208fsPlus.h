/*
 * Usb1208fsPlus.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USB1208FSPLUS_H_
#define USB_USB1208FSPLUS_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb1208fs_Plus: public UsbDaqDevice
{
public:
	Usb1208fs_Plus(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb1208fs_Plus();
};

} /* namespace ul */

#endif /* USB_USB1208FSPLUS_H_ */
