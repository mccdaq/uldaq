/*
 * Usb24xx.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_USB24XX_H_
#define USB_USB24XX_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL Usb24xx: public UsbDaqDevice
{
public:
	Usb24xx(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb24xx();

private:
	virtual void initilizeHardware() const;

	virtual int memRead_SingleCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

private:
	enum { CMD_FW_VER = 0x49};
};

} /* namespace ul */

#endif /* USB_USB24XX_H_ */
