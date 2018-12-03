/*
 * Usb1608fsPlus.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB1608FSPLUS_H_
#define USB1608FSPLUS_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb1608fs_Plus : public UsbDaqDevice
{
public:
	Usb1608fs_Plus(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb1608fs_Plus();
};

} /* namespace ul */

#endif /* USB1608FSPLUS_H_ */
