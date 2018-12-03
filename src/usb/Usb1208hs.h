/*
 * Usb1208hs.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USB1208HS_H_
#define USB_USB1208HS_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL Usb1208hs: public UsbFpgaDevice
{
public:
	Usb1208hs(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~Usb1208hs();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

private:
	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum { CMD_SETTRIG = 0x43 };
};

} /* namespace ul */

#endif /* USB_USB1208HS_H_ */
