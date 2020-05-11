/*
 * UsbFpgaDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USBFPGADEVICE_H_
#define USB_USBFPGADEVICE_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbFpgaDevice: public UsbDaqDevice
{
public:
	UsbFpgaDevice(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName);
	virtual ~UsbFpgaDevice();

	virtual int sendCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout = 1000) const;
	virtual int queryCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout = 1000, bool checkReplySize = true) const;

private:
	virtual void initilizeHardware() const;
	bool isFpgaLoaded() const;
	void loadFpga() const;
	bool isSpartanFpga() const;
	void reverseFpgaBits(unsigned char* fpgaImage, unsigned long size) const;
	void getFpgaImage(unsigned char** fpgaImage, unsigned int* size, unsigned char** bitReverseBuffer) const;

	unsigned short getRawFpgaVersion();

public:
	enum { CMD_FPGA_CFG = 0x50, CMD_FPGA_DATA = 0x51,CMD_FPGA_VER = 0x52};

private:
	std::string mFpgaFileName;
};

} /* namespace ul */

#endif /* USB_USBFPGADEVICE_H_ */
