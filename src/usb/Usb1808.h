/*
 * Usb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USB1808_H_
#define USB_USB1808_H_

#include "UsbFpgaDevice.h"

namespace ul
{

class UL_LOCAL Usb1808: public UsbFpgaDevice
{
public:
	Usb1808(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~Usb1808();

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const;


private:
	enum {CMD_TRIG_CONFIG = 0x43, CMD_PATTERN_TRIG_CONFIG = 0x44};
	enum {EDGE = 1, LEVEL=0, LOW = 0, HIGH = 1, NEG_POL = 0, POS_POL = 1};

};

} /* namespace ul */

#endif /* USB_USB1808_H_ */
