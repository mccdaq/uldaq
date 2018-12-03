/*
 * UsbDio32hs.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USBDIO32HS_H_
#define USB_USBDIO32HS_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL UsbDio32hs: public UsbFpgaDevice
{
public:
	UsbDio32hs(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~UsbDio32hs();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;

private:
	unsigned char getTrigModeCode(TriggerType type) const;

private:
	enum {CMD_TRIG_CONFIG = 0x43, CMD_PATTERN_TRIG_CONFIG = 0x44};
	enum {EDGE = 1, LEVEL=0, LOW = 0, HIGH = 1, NEG_POL = 0, POS_POL = 1};
};

} /* namespace ul */

#endif /* USB_USBDIO32HS_H_ */
