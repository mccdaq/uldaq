/*
 * UsbTc32.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USBTC32_H_
#define USB_USBTC32_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbTc32: public UsbDaqDevice
{
public:
	UsbTc32(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbTc32();

private:
	virtual void initilizeHardware() const;

	void readMeasurementFwVersions() const;

private:
	enum {CMD_VERSION = 0x43 };
};

} /* namespace ul */

#endif /* USB_USBTC32_H_ */
