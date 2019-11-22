/*
 * Usb1608hs.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_USB1608HS_H_
#define USB_USB1608HS_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb1608hs: public UsbDaqDevice
{
public:
	Usb1608hs(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb1608hs();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

private:
	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum { CMD_SETTRIG = 0x43 };
	enum { TTLLOW = 2212, TTLHIGH = 2457 };
};

} /* namespace ul */

#endif /* USB_USB1608HS_H_ */
