/*
 * UsbCtrx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USBCTRX_H_
#define USB_USBCTRX_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL UsbCtrx: public UsbFpgaDevice
{
public:
	UsbCtrx(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~UsbCtrx();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

private:
	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum {CMD_SETTRIG = 0x43};
	enum {EDGE = 1, LEVEL=0, LOW = 0, HIGH = 1, NEG_POL = 0, POS_POL = 1};
};

} /* namespace ul */

#endif /* USB_USBCTRX_H_ */
