/*
 * UsbDio24.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_USBDIO24_H_
#define HID_USBDIO24_H_

#include "HidDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbDio24: public HidDaqDevice
{
public:
	UsbDio24(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbDio24();

	virtual void flashLed(int flashCount) const;

public:
	enum { MAX_PACKET_SIZE = 8 }; // do not set the packet size to 9. if set to 9 on macOS the hid driver sends a packet with no data which causes the device to hang

private:

	enum { CMD_FLASH_LED = 0x0B };
};

} /* namespace ul */

#endif /* HID_USBDIO24_H_ */
