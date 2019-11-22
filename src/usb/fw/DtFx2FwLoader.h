/*
 * DtFx2FwLoader.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_FW_DTFX2FWLOADER_H_
#define USB_FW_DTFX2FWLOADER_H_

#include "IntelHexRec.h"

extern INTEL_HEX_RECORD DT_FX2LDR_FW_Image[];
extern INTEL_HEX_RECORD DT9837A_FW_Image[];

namespace ul
{

class DtFx2FwLoader
{
public:
	static void prepareHardware();
	static bool writeImage(libusb_device_handle* devHandle, int productId, bool firstTry);

private:
	static int downloadFirmware(libusb_device_handle* devHandle, unsigned int productId);
	static int downloadIntelHex(libusb_device_handle* devHandle, PINTEL_HEX_RECORD hexRecord, unsigned short maxIntRam);
	static int reset8051(libusb_device_handle* devHandle, unsigned char reset);

	//static int downloadFpga(libusb_device_handle* devHandle, unsigned short productId);
	//static int downloadFpgaImage(libusb_device_handle* devHandle, unsigned short nImage, unsigned char* pFpga, unsigned int FpgaLen);

	static int send(libusb_device_handle* devHandle, uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char* buff, uint16_t buffLen, int* sent, unsigned int timeout);
	static int query(libusb_device_handle* devHandle, uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char* buff, uint16_t buffLen, int* received, unsigned int timeout);

	//static int set_interface(libusb_device_handle* devHandle);
	//static bool isFpgaLoaded(libusb_device_handle* devHandle);
	//static void readSerialNumber(libusb_device_handle* devHandle, libusb_device_descriptor descriptor, char* serialNum);
	//static int test(libusb_device_handle* devHandle);
};

} /* namespace ul */

#endif /* USB_FW_DTFX2FWLOADER_H_ */
