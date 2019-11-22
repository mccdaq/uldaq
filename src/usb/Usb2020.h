/*
 * Usb2020.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_USB2020_H_
#define USB_USB2020_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL Usb2020: public UsbFpgaDevice
{
public:
	Usb2020(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~Usb2020();

	//virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

//	virtual void setCalOutput (unsigned int index) const;

//private:
//	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum { CMD_SETTRIG = 0x43, CMD_CALCFG = 0x44 };
};

} /* namespace ul */

#endif /* USB_USB2020_H_ */
