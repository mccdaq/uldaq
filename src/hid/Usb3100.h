/*
 * Usb3100.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USB3100_H_
#define HID_USB3100_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb3100: public HidDaqDevice
{
public:
	Usb3100(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb3100();

	virtual void flashLed(int flashCount) const;

private:
	enum {CMD_FLASH_LED = 0x40 };
};

} /* namespace ul */

#endif /* HID_USB3100_H_ */
