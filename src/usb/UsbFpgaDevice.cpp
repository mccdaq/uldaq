/*
 * UsbFpgaDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "UsbFpgaDevice.h"
#include "../utility/UlLock.h"

extern const unsigned char USB_1208HS_rbf[];
extern const unsigned int USB_1208HS_rbf_len;

extern const unsigned char USB_1608G_rbf[];
extern const unsigned int USB_1608G_rbf_len;

extern const unsigned char USB_1608G_2_rbf[];
extern const unsigned int USB_1608G_2_rbf_len;

extern const unsigned char USB_1808_bin[];
extern const unsigned int USB_1808_bin_len;

extern const unsigned char usb_2020_bin[];
extern const unsigned int usb_2020_bin_len;

extern const unsigned char USB_26xx_rbf[];
extern const unsigned int USB_26xx_rbf_len;

extern const unsigned char USB_CTR_bin[];
extern const unsigned int USB_CTR_bin_len;

extern const unsigned char USB_DIO32HS_bin[];
extern const unsigned int USB_DIO32HS_bin_len;


//#define FPGA_FILES_PATH		"/etc/uldaq/fpga/"

namespace ul
{

UsbFpgaDevice::UsbFpgaDevice(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName) : UsbDaqDevice(daqDeviceDescriptor)
{
	mFpgaFileName = fpgaFileName;
}

UsbFpgaDevice::~UsbFpgaDevice()
{
}

void UsbFpgaDevice::initilizeHardware() const
{
	if(!isFpgaLoaded())
	{
		loadFpga();

		if(!isFpgaLoaded())
		{
			const_cast<UsbFpgaDevice*>(this)->disconnect();

			throw UlException(ERR_NO_FPGA);
		}
	}


	const_cast<UsbFpgaDevice*>(this)->mRawFpgaVersion = const_cast<UsbFpgaDevice*>(this)->getRawFpgaVersion();

}

int UsbFpgaDevice::sendCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout) const
{
	if(mCurrentSuspendCount != SuspendMonitor::instance().getCurrentSystemSuspendCount())
	{
		if(!isFpgaLoaded())
			throw UlException(ERR_DEV_NOT_CONNECTED);
		else
			mCurrentSuspendCount = SuspendMonitor::instance().getCurrentSystemSuspendCount();
	}

	return UsbDaqDevice::sendCmd(request, wValue, wIndex, buff, buffLen, timeout);
}

int UsbFpgaDevice::queryCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout, bool checkReplySize) const
{
	//FnLog log("UsbDaqDevice::queryCmd");

	if(mCurrentSuspendCount != SuspendMonitor::instance().getCurrentSystemSuspendCount())
	{
		if(!isFpgaLoaded())
			throw UlException(ERR_DEV_NOT_CONNECTED);
		else
			mCurrentSuspendCount = SuspendMonitor::instance().getCurrentSystemSuspendCount();
	}

	return UsbDaqDevice::queryCmd(request, wValue, wIndex, buff, buffLen, timeout);
}

bool UsbFpgaDevice::isFpgaLoaded() const
{
	bool loaded = false;

	unsigned char cmd = getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned short status = 0;

	// do not call UsbFpgaDevice::queryCmd() here
	UsbDaqDevice::queryCmd(cmd, 0, 0, (unsigned char*)&status, sizeof(status));

	if((status & 0x100) == 0x100)
		loaded = true;

	return loaded;
}

// loads fpga image from file
/*
void UsbFpgaDevice::loadFpga() const
{
	UlError __attribute__((unused)) error = ERR_NO_ERROR;
	std::string fpga_path(FPGA_FILES_PATH + mFpgaFileName);

	std::ifstream fpgaFileStream(fpga_path.c_str(), std::ios::in| std::ios::binary| std::ios::ate);
	std::ifstream::pos_type size;

	if(fpgaFileStream)
	{
		size = fpgaFileStream.tellg();
		unsigned char* fpgaImage = new unsigned char[size];

		if(fpgaImage)
		{
			fpgaFileStream.seekg (0, std::ios::beg);
			fpgaFileStream.read ((char*)fpgaImage, size);
			fpgaFileStream.close();

			// enter config mode
			unsigned char unlockCode = 0xAD;

			try
			{
				unsigned long num_bytes = sizeof(unlockCode);
				UsbDaqDevice::sendCmd(CMD_FPGA_CFG, 0, 0, &unlockCode, num_bytes);

				 if(getDeviceType() == DaqDeviceId::USB_2020)
				 {
					 reverseFpgaBits(fpgaImage, size);
				 }

				// transfer data
				int remaining = size;
				unsigned char* ptr = fpgaImage;
				do
				{
					if(remaining > 64)
						num_bytes = 64;
					else
						num_bytes = remaining;

					UsbDaqDevice::sendCmd(CMD_FPGA_DATA, 0, 0, ptr, num_bytes);

					ptr += num_bytes;
					remaining -= num_bytes;

				} while (remaining > 0);

				if(isSpartanFpga())
				{
					unsigned char dummyData[2] = {0 , 0};

					UsbDaqDevice::sendCmd(CMD_FPGA_DATA, 0, 0, dummyData, sizeof(dummyData));
				}
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}

			delete[] fpgaImage;

			if(error)
				throw UlException(error);
		}
		else
			std::cout << "**** insufficient memory to load the fpga image" << std::endl;
	}
	else
	{
		if(errno == ENOENT) // file does not exist
		{
			throw UlException(ERR_FPGA_FILE_NOT_FOUND);
		}
		else
			throw UlException(ERR_UNABLE_TO_READ_FPGA_FILE);
	}
}*/

void UsbFpgaDevice::loadFpga() const
{
	UlError __attribute__((unused)) error = ERR_NO_ERROR;

	unsigned int size = 0;
	const unsigned char* fpgaImage = NULL;
	unsigned char* bitReverseBuffer = NULL;

	getFpgaImage(&fpgaImage, &size, &bitReverseBuffer);

	if(fpgaImage)
	{
		// enter config mode
		unsigned char unlockCode = 0xAD;

		try
		{
			unsigned long num_bytes = sizeof(unlockCode);
			UsbDaqDevice::sendCmd(CMD_FPGA_CFG, 0, 0, &unlockCode, num_bytes);

			// transfer data
			int remaining = size;
			const unsigned char* ptr = fpgaImage;
			do
			{
				if(remaining > 64)
					num_bytes = 64;
				else
					num_bytes = remaining;

				UsbDaqDevice::sendCmd(CMD_FPGA_DATA, 0, 0, const_cast<unsigned char*>(ptr), num_bytes);

				ptr += num_bytes;
				remaining -= num_bytes;

			} while (remaining > 0);

			if(isSpartanFpga())
			{
				unsigned char dummyData[2] = {0 , 0};

				UsbDaqDevice::sendCmd(CMD_FPGA_DATA, 0, 0, dummyData, sizeof(dummyData));
			}
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}

		if(bitReverseBuffer)
			delete[] bitReverseBuffer;

		if(error)
			throw UlException(error);
	}
	else
		std::cout << "**** the fpga image not included" << std::endl;
}

void UsbFpgaDevice::getFpgaImage(const unsigned char** fpgaImage, unsigned int* size, unsigned char** bitReverseBuffer) const
{
	unsigned int devType = getDeviceType();
	switch(devType)
	{
		case DaqDeviceId::USB_1208HS:
		case DaqDeviceId::USB_1208HS_2AO:
		case DaqDeviceId::USB_1208HS_4AO:
			*fpgaImage = USB_1208HS_rbf;
			*size = USB_1208HS_rbf_len;
			break;

		case DaqDeviceId::USB_1608G:
		case DaqDeviceId::USB_1608GX:
		case DaqDeviceId::USB_1608GX_2AO:
			*fpgaImage = USB_1608G_rbf;
			*size = USB_1608G_rbf_len;
			break;
		case DaqDeviceId::USB_1608G_2:
		case DaqDeviceId::USB_1608GX_2:
		case DaqDeviceId::USB_1608GX_2AO_2:
			*fpgaImage = USB_1608G_2_rbf;
			*size = USB_1608G_2_rbf_len;
		break;

		case DaqDeviceId::USB_1808:
		case DaqDeviceId::USB_1808X:
			*fpgaImage = USB_1808_bin;
			*size = USB_1808_bin_len;
		break;

		case DaqDeviceId::USB_2623:
		case DaqDeviceId::USB_2627:
		case DaqDeviceId::USB_2633:
		case DaqDeviceId::USB_2637:
			*fpgaImage = USB_26xx_rbf;
			*size = USB_26xx_rbf_len;
		break;

		case DaqDeviceId::USB_DIO32HS:
			*fpgaImage = USB_DIO32HS_bin;
			*size = USB_DIO32HS_bin_len;
		break;

		case DaqDeviceId::USB_CTR04:
		case DaqDeviceId::USB_CTR08:
			*fpgaImage = USB_CTR_bin;
			*size = USB_CTR_bin_len;
		break;
		case DaqDeviceId::USB_2020:

			// do not reverse the global image (usb_2020_bin[])
			 *bitReverseBuffer = new unsigned char[usb_2020_bin_len];

			 if(*bitReverseBuffer)
			 {
				 memcpy(*bitReverseBuffer, usb_2020_bin, usb_2020_bin_len);
				 reverseFpgaBits(*bitReverseBuffer, usb_2020_bin_len);

				 *fpgaImage = *bitReverseBuffer;
				 *size = usb_2020_bin_len;
			 }
			 else
				std::cout << "**** insufficient memory to reverse the fpga image bits" << std::endl;

		break;
		default:
			std::cout << "undefined FPGA device" << std::endl;

	}
}

void UsbFpgaDevice::reverseFpgaBits(unsigned char* fpgaImage, unsigned long size) const
{
	for(unsigned int index = 0; index < size; index++)
	{
		if(fpgaImage[index] != 0)
		{
			fpgaImage[index] = (fpgaImage[index] & 0x0f) <<  4  |  (fpgaImage[index] & 0xf0) >>  4;
			fpgaImage[index] = (fpgaImage[index] & 0x33) <<  2  |  (fpgaImage[index] & 0xcc) >>  2;
			fpgaImage[index] = (fpgaImage[index] & 0x55) <<  1  |  (fpgaImage[index] & 0xaa) >>  1;
		}
	}
}

bool UsbFpgaDevice::isSpartanFpga() const
{
	bool spartan_fpga = false;

	switch(getDeviceType())
	{
	case DaqDeviceId::USB_1808:
	case DaqDeviceId::USB_1808X:
	case DaqDeviceId::USB_CTR08:
	case DaqDeviceId::USB_CTR04:
	case DaqDeviceId::USB_DIO32HS:
	case DaqDeviceId::USB_2020:
		spartan_fpga = true;
		break;
	}

	return spartan_fpga;
}

unsigned short UsbFpgaDevice::getRawFpgaVersion()
{
	unsigned short rawFpgaVer;

	queryCmd(CMD_FPGA_VER, 0, 0, (unsigned char*)&rawFpgaVer, sizeof(rawFpgaVer));

	return rawFpgaVer;
}


} /* namespace ul */
