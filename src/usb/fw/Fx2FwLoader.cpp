/*
 * Fx2FwLoader.cpp
 *
 * Author: Measurement Computing Corporation
 */
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <algorithm>
#include <unistd.h>


#include "../../DaqDeviceManager.h"
#include "../UsbDaqDevice.h"
#include "../../DaqDeviceId.h"
#include "../../utility/FnLog.h"
#include "../../utility/UlLock.h"
#include "../../utility/Endian.h"

#include "Fx2FwLoader.h"

//
// FX2 constants
//

#define MAX_RECORD_SIZE						512
#define ANCHOR_LOAD_INTERNAL				0xA0
#define PID_OFFSET							0xE000
#define CPUCS_REG_FX2						0xE600

#define VR_GETFWVERSION  0xb0 // read the firmware version
#define VR_DBG_CONTROL   0xb1 // read/write debug control variable
#define VR_FPGA_INIT     0xb2 // initialize fpga image
#define VR_FPGA_DOWNLOAD 0xb3 // download block of fpga image data
#define VR_FPGA_REGIO    0xb4 // read/write fpga register

//#define PID_PDAQ3KLD  	0x0470
//#define PID_QUAD08		0x00CA

namespace ul
{
void Fx2FwLoader::prepareHardware()
{
	FnLog log("Fx2FwLoader::prepareHardware");

	libusb_device** devs;
	libusb_device* dev;
	libusb_device_handle* 	devHandle;
	int ret = 0;
	bool fwloaded = false;

	const libusb_context* ctx = UsbDaqDevice::getLibUsbContext();

	if(ctx == NULL)
		std::cout << "libusb_context is not initialized" << std::endl;

	int numDevs = libusb_get_device_list ((libusb_context*)ctx, &devs);

	if(numDevs > 0)
	{
		int devNum = 0;
		while ((dev = devs[devNum++]) != NULL)
		{
			struct libusb_device_descriptor desc;
			memset(&desc, 0,sizeof(libusb_device_descriptor));

			ret = libusb_get_device_descriptor(dev, &desc);

			if (ret < 0)
			{
				UL_LOG("failed to get device descriptor");
			}

			if(desc.idVendor == UsbDaqDevice::MCC_USB_VID)
			{
				if(desc.idProduct == DaqDeviceId::PDAQ3KLD)
				{
					int status = libusb_open(dev, &devHandle);

					if (status == LIBUSB_SUCCESS)
					{
						status = libusb_claim_interface(devHandle, 0);

						if (status == LIBUSB_SUCCESS)
						{
							Fx2FwLoader::downloadFirmware(devHandle);

							fwloaded = true;

							libusb_release_interface(devHandle, 0);
						}
						else
						{
							UL_LOG("libusb_claim_interface() failed: " << libusb_error_name(status));
						}


						libusb_close(devHandle);
					}
				}
			}
		}

	}

	libusb_free_device_list(devs, 1);


	if(fwloaded) // if fw is loaded to at least one device we need wait for the dev(s) to enumerate and load FPGA
	{
		sleep(5);

		numDevs = libusb_get_device_list ((libusb_context*)ctx, &devs);

		if(numDevs > 0)
		{
			int devNum = 0;
			while ((dev = devs[devNum++]) != NULL)
			{
				struct libusb_device_descriptor desc;
				memset(&desc, 0,sizeof(libusb_device_descriptor));

				ret = libusb_get_device_descriptor(dev, &desc);

				if (ret < 0)
				{
					UL_LOG("failed to get device descriptor");
				}

				if(desc.idVendor == UsbDaqDevice::MCC_USB_VID)
				{
					if(desc.idProduct == DaqDeviceId::USB_QUAD08)
					{
						int status = libusb_open(dev, &devHandle);

						if (status == LIBUSB_SUCCESS)
						{
							status = libusb_claim_interface(devHandle, 0);

							/*unsigned char buff[2];
							int recevied = 0;
							query(devHandle, VR_FPGA_REGIO, 0, 0x5f, buff, sizeof(buff), &recevied, 1000);*/

							if (status == LIBUSB_SUCCESS)
							{
								if(!isFpgaLoaded(devHandle))
								{
									downloadFpga(devHandle, desc.idProduct);
								}

								/*unsigned char buff[2];
								int recevied = 0;
								query(devHandle, VR_FPGA_REGIO, 0, 0x5f, buff, sizeof(buff), &recevied, 1000);*/

								libusb_release_interface(devHandle, 0);
							}
							else
							{
								UL_LOG("libusb_claim_interface() failed: " << libusb_error_name(status));
							}


							libusb_close(devHandle);
						}
					}
				}
			}

		}

		libusb_free_device_list(devs, 1);
	}

}

int Fx2FwLoader::downloadFirmware(libusb_device_handle* devHandle)
{
	int status = 0;

	set_interface(devHandle);

	// First download loader firmware.  The loader firmware implements a vendor
	// specific command that will allow us to anchor load to external ram

	reset8051(devHandle, 1);
	downloadIntelHex(devHandle, FX2LDR_FW_Image, 0x3FFF);
	reset8051(devHandle, 0);

	downloadIntelHex(devHandle, PDAQ3K_FW_Image, 0x3FFF);

	reset8051(devHandle, 1);
	reset8051(devHandle, 0);


	return status;
}

int Fx2FwLoader::downloadIntelHex(libusb_device_handle* devHandle, PINTEL_HEX_RECORD hexRecord, unsigned short maxIntRam)
{
	int status = 0;
	int sent;

	PINTEL_HEX_RECORD ptr = hexRecord;

	while (ptr->Type == 0)
	{
		if (ptr->Address > maxIntRam)
		{
			status = send(devHandle, ANCHOR_LOAD_INTERNAL, ptr->Address, 0, ptr->Data, ptr->Length, &sent, 2000);
			if (status < 0)
				break;
		}
		ptr++;
	}

	//
	// Now download all of the records that are in internal RAM.  Before starting
	// the download, stop the 8051.
	//
	reset8051(devHandle, 1);

	ptr = hexRecord;
	while (ptr->Type == 0)
	{
		if (ptr->Address <= maxIntRam)
		{
			status = send(devHandle, ANCHOR_LOAD_INTERNAL, ptr->Address, 0, ptr->Data, ptr->Length, &sent, 2000);

			if (status < 0)
				break;
		}
		ptr++;
	}

	return status;
}

int Fx2FwLoader::reset8051(libusb_device_handle* devHandle, unsigned char reset)
{
	/*++

	Routine Description:
	    Uses the ANCHOR LOAD vendor specific command to either set or release the
	    8051 reset bit in the EZ-USB chip.
	--*/

	int sent;
	int status = send(devHandle, ANCHOR_LOAD_INTERNAL, CPUCS_REG_FX2, 0, &reset, sizeof(reset), &sent, 2000);

	return status;
}


int Fx2FwLoader::downloadFpga(libusb_device_handle* devHandle, unsigned short productId)
{
	int status = 0;

	unsigned char* image;
	unsigned int imageSize;
	unsigned int imageStart;
	unsigned short imageIndex;

	if(productId == DaqDeviceId::USB_QUAD08)
	{
		image = (unsigned char*) USBQuad06Fpga;
		imageSize = sizeof(USBQuad06Fpga);
		imageStart = 0x4E;
		imageIndex = 0;
	}
	else
	{
		std::cout << "downloadFpga error: unknown device" << std::endl;
		return status;
	}

	downloadFpgaImage(devHandle, imageIndex, &image[imageStart], imageSize - imageStart);

	return status;
}

int Fx2FwLoader::downloadFpgaImage(libusb_device_handle* devHandle, unsigned short nImage, unsigned char* pFpga, unsigned int FpgaLen)
{
	int status = 0;
	unsigned char* ptr = pFpga;

	int sent = 0;

	status = send(devHandle, VR_FPGA_INIT, 0, nImage, NULL, 0, &sent, 2000);

	unsigned int writeSize;
	while( status >= 0 && FpgaLen )
	{
		writeSize = ((FpgaLen > 2048) ? 2048 : FpgaLen);

		status = send(devHandle, VR_FPGA_DOWNLOAD, 0, nImage, ptr, writeSize, &sent, 2000);

		ptr += writeSize;
		FpgaLen -= writeSize;
	}

	if(!isFpgaLoaded(devHandle))
	{
		std::cout << "Loading FPGA image failed." << std::endl;
	}

	return status;
}

int Fx2FwLoader::send(libusb_device_handle* devHandle, uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, int* sent, unsigned int timeout)
{
	//FnLog log("UsbDaqDevice::send");

	int status = 0;

	uint8_t requestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;

	if(devHandle)
	{
		status = libusb_control_transfer(devHandle, requestType, request, wValue, wIndex, buff, buffLen, timeout);

		if (status != buffLen)
		{
			if (status < 0)
				UL_LOG("#### libusb_control_transfer failed : " << libusb_error_name(status));
			else
				UL_LOG("#### libusb_control_transfer failed : " << status);
		}
		else
		{
			*sent = buffLen;
			UL_LOG("Bytes sent: " << *sent);
		}
	}

	return status;
}

int Fx2FwLoader::query(libusb_device_handle* devHandle, uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, int* recevied, unsigned int timeout)
{
	//FnLog log("UsbDaqDevice::query");

	int status = 0;

	uint8_t requestType = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;

	if(devHandle)
	{
		status = libusb_control_transfer(devHandle, requestType, request, wValue, wIndex, buff, buffLen, timeout);

		if (status != buffLen)
		{
			if (status < 0)
				UL_LOG("#### libusb_control_transfer failed : " << libusb_error_name(status));
			else
				UL_LOG("#### libusb_control_transfer failed : " << status);

		}
		else
		{
			*recevied = buffLen;
			UL_LOG("Bytes received: " << *recevied);
		}
	}

	return status;
}

int Fx2FwLoader::set_interface(libusb_device_handle* devHandle)
{
	//FnLog log("UsbDaqDevice::send");
	uint8_t request = LIBUSB_REQUEST_SET_INTERFACE;
	uint16_t wValue = 0;
	uint16_t wIndex = 0;
	unsigned char *buff = NULL;
	uint16_t buffLen = 0;
	unsigned int timeout = 2000;

	int status = 0;

	uint8_t requestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_INTERFACE;

	if(devHandle)
	{
		status = libusb_control_transfer(devHandle, requestType, request, wValue, wIndex, buff, buffLen, timeout);

		if (status != buffLen)
		{
			if (status < 0)
				UL_LOG("#### libusb_control_transfer failed : " << libusb_error_name(status));
			else
				UL_LOG("#### libusb_control_transfer failed : " << status);
		}
		/*else
		{
			/sent = buffLen;
			UL_LOG("Bytes sent: " << *sent);
		}*/
	}

	return status;
}

bool Fx2FwLoader::isFpgaLoaded(libusb_device_handle* devHandle)
{
	bool fpgaLoaded = false;
	int __attribute__((unused)) status = 0;

	unsigned char fpgaStatus = 0;
	int recevied;
	uint16_t fpgaIndex = 0;



	status = query(devHandle, VR_FPGA_INIT, 0, fpgaIndex, &fpgaStatus, sizeof(fpgaStatus), &recevied, 2000);

	if(fpgaStatus == 1)
		fpgaLoaded = true;

	return fpgaLoaded;
}

/*
int Fx2FwLoader::test(libusb_device_handle* devHandle)
{
	//FnLog log("UsbDaqDevice::query");

	uint8_t request = LIBUSB_REQUEST_GET_DESCRIPTOR;
	uint16_t wValue = 0x0303;
	uint16_t wIndex = 0x0409;
	unsigned char buff[32];
	uint16_t buffLen = 32;
    unsigned int timeout = 2000;

	int status = 0;

	uint8_t requestType = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD  | LIBUSB_RECIPIENT_DEVICE;

	if(devHandle)
	{
		status = libusb_control_transfer(devHandle, requestType, request, wValue, wIndex, buff, buffLen, timeout);

		if (status != buffLen)
		{
			if (status < 0)
				UL_LOG("#### libusb_control_transfer failed : " << libusb_error_name(status));
			else
				UL_LOG("#### libusb_control_transfer failed : " << status);

		}

	}

	return status;
}*/

/*
void Fx2FwLoader::readSerialNumber(libusb_device_handle* devHandle, libusb_device_descriptor descriptor, char* serialNum)
{
	unsigned char iLangID = 0;
	unsigned short langIDBuf[2] = {0};

	//int numBytes = libusb_get_string_descriptor(devHandle, iLangID, 0, (unsigned char*)langIDBuf, sizeof(langIDBuf));

	unsigned char serial[128] = {0};
	int numBytes = libusb_get_string_descriptor_ascii(devHandle, 0x03, serial, sizeof(serial));

	if(numBytes > 0)
	{
		unsigned short langID =  0x0409;//Endian::cpu_to_le_ui16(langIDBuf[1]);

		unsigned char serial[32] = {0};
		numBytes = libusb_get_string_descriptor(devHandle, 0x03, langID, serial, sizeof(serial));
		{
			if(numBytes > 0)
			{
				int snSize = (numBytes - 2) / 2;
				for(int i = 0; i < snSize; i++)
					serialNum[i] = serial[(i*2) + 3];
			}
		}
	}
}*/
} /* namespace ul */
