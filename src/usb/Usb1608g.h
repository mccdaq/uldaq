/*
 * Usb1608g.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USB1608G_H_
#define USB_USB1608G_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL Usb1608g: public UsbFpgaDevice
{
public:
	Usb1608g(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~Usb1608g();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

	virtual void setCalOutput (unsigned int index) const;

private:
	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum { CMD_SETTRIG = 0x43, CMD_CALCFG = 0x44 };
};

} /* namespace ul */

#endif /* USB_USB1608G_H_ */
