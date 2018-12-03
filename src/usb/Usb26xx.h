/*
 * Usb26xx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USB26XX_H_
#define USB_USB26XX_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL Usb26xx: public UsbFpgaDevice
{
public:
	Usb26xx(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~Usb26xx();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

	virtual void setCalOutput (unsigned int index) const;

private:
	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum { CMD_SETTRIG = 0x43, CMD_CALCFG = 0x44 };
};

} /* namespace ul */

#endif /* USB_USB26XX_H_ */
