/*
 * HidDaqDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "HidDaqDevice.h"
#include "../DaqDeviceManager.h"
#include "../utility/Endian.h"

#include <stdlib.h>

#define NO_PERMISSION_STR		"NO PERMISSION"

namespace ul
{
bool HidDaqDevice::mInitialized = false;

HidDaqDevice::HidDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor) : DaqDevice(daqDeviceDescriptor)
{
	FnLog log("HidDaqDevice::HidDaqDevice");

	mDevHandle = NULL;
	mConnected = false;

	UlLock::initMutex(mConnectionMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mIoMutex, PTHREAD_MUTEX_RECURSIVE);
}

HidDaqDevice::~HidDaqDevice()
{
	FnLog log("HidDaqDevice::~HidDaqDevice");

	disconnect();

	UlLock::destroyMutex(mIoMutex);
	UlLock::destroyMutex(mConnectionMutex);
}

void HidDaqDevice::hidapi_init()
{
	if(!mInitialized)
	{
		int status = hid_init();

		if(status != 0)
		{
			UL_LOG("hid_init() failed");
		}
		else
			mInitialized = true;
	}
}

void HidDaqDevice::hidapi_exit()
{
	if(mInitialized)
	{
		hid_exit();

		mInitialized = false;
	}
}

std::vector<DaqDeviceDescriptor> HidDaqDevice::findDaqDevices()
{
	std::vector<DaqDeviceDescriptor> descriptorList;

	FnLog log("HidDaqDevice::getDaqDeviceDescriptorList");

	struct hid_device_info *devs, *cur_dev;

	// Flush the input pipe of all mcc hid devices attached to the host
	// If the input pipe's buffer was not emptied for any reason, i.e, application crash or SIGINT
	// then sending setup request during device discovery will cause the device to hang
	hid_flush_input_pipe(MCC_USB_VID);

	devs = hid_enumerate(MCC_USB_VID, 0x0);
	cur_dev = devs;

	while (cur_dev)
	{
		if(DaqDeviceManager::isDaqDeviceSupported(cur_dev->product_id))
		{
			DaqDeviceDescriptor daqDevDescriptor;
			memset(&daqDevDescriptor, 0,sizeof(DaqDeviceDescriptor));

			daqDevDescriptor.productId = cur_dev->product_id;
			daqDevDescriptor.devInterface = USB_IFC;
			std::string productName = DaqDeviceManager::getDeviceName(cur_dev->product_id);

			strncpy(daqDevDescriptor.productName, productName.c_str(), sizeof(daqDevDescriptor.productName) - 1);
			strncpy(daqDevDescriptor.devString, productName.c_str(), sizeof(daqDevDescriptor.devString) - 1);

			if(cur_dev->serial_number)
			{
				if(wcslen(cur_dev->serial_number))
				{
					char serial[128] = {0};
					wcstombs(serial,cur_dev->serial_number, sizeof(serial));
					strcpy(daqDevDescriptor.uniqueId, serial);
				}
				else
					strcpy(daqDevDescriptor.uniqueId, NO_PERMISSION_STR);
			}
			else
				strcpy(daqDevDescriptor.uniqueId, NO_PERMISSION_STR);

			UL_LOG("-----------------------");
			UL_LOG("Product ID : 0x" << std::hex << daqDevDescriptor.productId << std::dec);
			UL_LOG("Product Name: "<< daqDevDescriptor.productName);
			UL_LOG("Serial Number : "<< daqDevDescriptor.uniqueId);
			UL_LOG("-----------------------");

			descriptorList.push_back(daqDevDescriptor);

			/*printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
			printf("\n");
			printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
			printf("  Product:      %ls\n", cur_dev->product_string);
			printf("  Release:      %hx\n", cur_dev->release_number);
			printf("  Interface:    %d\n",  cur_dev->interface_number);
			printf("\n");*/
		}
		cur_dev = cur_dev->next;
	}

	hid_free_enumeration(devs);

	return descriptorList;
}

void HidDaqDevice::connect()
{
	FnLog log("UsbDaqDevice::connect");

	UlLock lock(mConnectionMutex);

	if(mConnected)
	{
		UL_LOG("Device is already connected, disconnecting...");

		disconnect();
	}

	establishConnection();

	mConnected = true;

	//mCurrentSuspendCount = SuspendMonitor::getCurrentSystemSuspendCount();

	initilizeHardware();

	initializeIoDevices();
}

void HidDaqDevice::disconnect()
{
	FnLog log("UsbDaqDevice::disconnect");

	if(mConnected)
	{
		DaqDevice::disconnect();

		releaseHidResources();
	}
}

void HidDaqDevice::establishConnection()
{
	FnLog log("HidDaqDevice::establishConnection");

	if(std::strcmp(mDaqDeviceDescriptor.uniqueId, NO_PERMISSION_STR) == 0)
		throw UlException(ERR_USB_DEV_NO_PERMISSION);

	wchar_t serial[128];
	memset(serial, 0, sizeof(serial));
	mbstowcs( serial, mDaqDeviceDescriptor.uniqueId, strlen(mDaqDeviceDescriptor.uniqueId));

	UlError err = ERR_NO_ERROR;
	hid_device_info devInfo;
	mDevHandle = hid_open(MCC_USB_VID, mDaqDeviceDescriptor.productId, serial, &devInfo, &err);

	if(mDevHandle)
	{
		mRawFwVersion = devInfo.release_number;
	}
	else
	{
		if(err)
			throw UlException(err);
		else
			throw UlException(ERR_DEV_NOT_FOUND);
	}
}


void HidDaqDevice::releaseHidResources()
{
	FnLog log("UsbDaqDevice::releaseUsbResources");

	if(mDevHandle)
	{
		UlLock lock(mIoMutex);

		hid_close(mDevHandle);

		mDevHandle = NULL;
	}
}

void HidDaqDevice::sendCmd(unsigned char cmd) const
{
	size_t outLength = 1;

	sendRawCmd(&cmd, &outLength);
}

void HidDaqDevice::sendCmd(unsigned char cmd, unsigned char param) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param;
	}outData;
#pragma pack()

	outData.cmd = cmd;
	outData.param = param;

	size_t outLength = sizeof(outData);

	sendRawCmd((unsigned char*) &outData, &outLength);
}

void HidDaqDevice::sendCmd(unsigned char cmd, unsigned char param1, unsigned char param2) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param1;
		unsigned char param2;
	}outData;
#pragma pack()

	outData.cmd = cmd;
	outData.param1 = param1;
	outData.param2 = param2;

	size_t outLength = sizeof(outData);

	sendRawCmd((unsigned char*) &outData, &outLength);
}

void HidDaqDevice::sendCmd(unsigned char cmd, unsigned char param1, unsigned char param2, unsigned char param3) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param1;
		unsigned char param2;
		unsigned char param3;
	}outData;
#pragma pack()

	outData.cmd = cmd;
	outData.param1 = param1;
	outData.param2 = param2;
	outData.param3 = param3;

	size_t outLength = sizeof(outData);

	sendRawCmd((unsigned char*) &outData, &outLength);
}

void HidDaqDevice::sendCmd(unsigned char cmd, unsigned char param1, unsigned short param2, unsigned char param3) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param1;
		unsigned short param2;
		unsigned char param3;
	}outData;
#pragma pack()

	outData.cmd = cmd;
	outData.param1 = param1;
	outData.param2 = param2;
	outData.param3 = param3;

	size_t outLength = sizeof(outData);

	sendRawCmd((unsigned char*) &outData, &outLength);
}

void HidDaqDevice::sendCmd(unsigned char cmd, unsigned char* dataBuffer, unsigned int dataBufferSize) const
{
	size_t outLength = dataBufferSize + 1;
	unsigned char* outData = new unsigned char[outLength];

	outData[0] = cmd;

	memcpy(&outData[1], dataBuffer, dataBufferSize);

	sendRawCmd(outData, &outLength);

	delete [] outData;
}

void HidDaqDevice::sendCmd(unsigned char cmd, unsigned short param, unsigned char* dataBuffer, unsigned int dataBufferSize) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned short param;
	}outParams;
#pragma pack()

	size_t outLength = sizeof(outParams) +  dataBufferSize;

	unsigned char* outData = new unsigned char[outLength];

	outParams.cmd = cmd;
	outParams.param = Endian::cpu_to_le_ui16(param);

	memcpy(&outData[0], &outParams, sizeof(outParams));

	int dataIdx = sizeof(outParams);
	memcpy(&outData[dataIdx], dataBuffer, dataBufferSize);

	sendRawCmd(outData, &outLength);

	delete [] outData;
}

int HidDaqDevice::sendCmd(unsigned char cmd, unsigned short param1,  unsigned char param2, unsigned char* dataBuffer, unsigned int dataBufferSize) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned short param1;
		unsigned char param2;
	}outParams;
#pragma pack()

	size_t outLength = sizeof(outParams) +  dataBufferSize;

	unsigned char* outData = new unsigned char[outLength];

	outParams.cmd = cmd;
	outParams.param1 = Endian::cpu_to_le_ui16(param1);
	outParams.param2 = param2;

	memcpy(&outData[0], &outParams, sizeof(outParams));

	int dataIdx = sizeof(outParams);
	memcpy(&outData[dataIdx], dataBuffer, dataBufferSize);

	sendRawCmd(outData, &outLength);

	delete [] outData;

	int sent = outLength - sizeof(outParams);

	return sent;
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned char* data, unsigned int timeout) const
{
	size_t outLength = 1;

#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char data;
	}indata;
#pragma pack()

	size_t inLength = sizeof(indata);

	queryRawCmd(&cmd, outLength, (unsigned char*) &indata, &inLength, timeout);

	*data = indata.data;
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned int* data, unsigned int timeout) const
{
	size_t outLength = 1;

#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned int data;
	}indata;
#pragma pack()

	size_t inLength = sizeof(indata);

	queryRawCmd(&cmd, outLength, (unsigned char*) &indata, &inLength, timeout);

	*data = Endian::le_ui32_to_cpu(indata.data);
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned char param, unsigned char* data, unsigned int timeout) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param;
	}outData;

	struct
	{
		unsigned char cmd;
		unsigned char data;
	}inData;
#pragma pack()

	outData.cmd = cmd;
	outData.param = param;

	size_t outLength = sizeof(outData);
	size_t inLength = sizeof(inData);

	queryRawCmd((unsigned char*) &outData, outLength, (unsigned char*) &inData, &inLength, timeout);

	*data = inData.data;
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned char param1, unsigned char param2, unsigned char* data, unsigned int timeout) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param1;
		unsigned char param2;
	}outData;

	struct
	{
		unsigned char cmd;
		unsigned char data;
	}inData;
#pragma pack()

	outData.cmd = cmd;
	outData.param1 = param1;
	outData.param2 = param2;

	size_t outLength = sizeof(outData);
	size_t inLength = sizeof(inData);

	queryRawCmd((unsigned char*) &outData, outLength, (unsigned char*) &inData, &inLength, timeout);

	*data = inData.data;
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned short* data, unsigned int timeout) const
{
	size_t outLength = 1;

#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned short data;
	}inData;
#pragma pack()

	size_t inLength = sizeof(inData);

	queryRawCmd(&cmd, outLength, (unsigned char*) &inData, &inLength, timeout);

	*data = Endian::le_ui16_to_cpu(inData.data);
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned char* dataBuffer, unsigned int dataBufferSize, unsigned int timeout) const
{
	size_t outLength = 1;

	size_t inLength = dataBufferSize + 1;

	unsigned char* inData = new unsigned char[inLength];

	queryRawCmd(&cmd, outLength, inData, &inLength, timeout);

	memcpy(dataBuffer, &inData[1], dataBufferSize);

	delete [] inData;
}

unsigned int HidDaqDevice::queryCmd(unsigned char cmd, unsigned char param1, unsigned char param2, unsigned char param3, unsigned char* dataBuffer, unsigned int dataBufferSize, unsigned int timeout) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param1;
		unsigned char param2;
		unsigned char param3;
	}outData;
#pragma pack()

	unsigned int bytesRead = 0;

	outData.cmd = cmd;
	outData.param1 = Endian::cpu_to_le_ui16(param1);
	outData.param2 = param2;
	outData.param3 = param3;

	size_t outLength = sizeof(outData);

	size_t inLength = dataBufferSize + 1;

	unsigned char* inData = new unsigned char[inLength];

	queryRawCmd((unsigned char*) &outData, outLength, inData, &inLength, timeout);

	if(inLength > 0)
	{
		bytesRead = inLength - 1;
		memcpy(dataBuffer, &inData[1], bytesRead);
	}

	delete [] inData;

	return bytesRead;
}

unsigned int HidDaqDevice::queryCmd(unsigned char cmd, unsigned short param1, unsigned char param2, unsigned char param3, unsigned char* dataBuffer, unsigned int dataBufferSize, unsigned int timeout) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned short param1;
		unsigned char param2;
		unsigned char param3;
	}outData;
#pragma pack()

	unsigned int bytesRead = 0;

	outData.cmd = cmd;
	outData.param1 = Endian::cpu_to_le_ui16(param1);
	outData.param2 = param2;
	outData.param3 = param3;

	size_t outLength = sizeof(outData);

	size_t inLength = dataBufferSize + 1;

	unsigned char* inData = new unsigned char[inLength];

	queryRawCmd((unsigned char*) &outData, outLength, inData, &inLength, timeout);

	if(inLength > 0)
	{
		bytesRead = inLength - 1;
		memcpy(dataBuffer, &inData[1], bytesRead);
	}

	delete [] inData;


	return bytesRead;
}

void HidDaqDevice::queryCmd(unsigned char cmd, unsigned char param1, unsigned char param2, float* data, unsigned int timeout) const
{
#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned char param1;
		unsigned char param2;
	}outData;

	struct
	{
		unsigned char cmd;
		unsigned char data[4];
	}inData;
#pragma pack()

	outData.cmd = cmd;
	outData.param1 = param1;
	outData.param2 = param2;

	size_t outLength = sizeof(outData);
	size_t inLength = sizeof(inData);

	queryRawCmd((unsigned char*) &outData, outLength, (unsigned char*) &inData, &inLength, timeout);

	*data = Endian::Instance().le_ptr_to_cpu_f32(inData.data);
}


void HidDaqDevice::sendRawCmd(const unsigned char *data, size_t* length) const
{
	UlLock lock(mIoMutex);

	//size_t len = length;
	//int sent = 0;

	UlError err = send(data, length);

	//sent = len;

	if(err)
		throw UlException(err);

	//return sent;
}

UlError HidDaqDevice::send(const unsigned char *data, size_t* length) const
{
	UlError err = ERR_NO_ERROR;
	int sent = 0;

	if(mConnected)
	{
		if(mDevHandle)
		{
			sent = hid_write(mDevHandle, data, *length);

			if (sent == -1)
			{
				UL_LOG("#### hid_write failed");

				err = ERR_DEV_NOT_CONNECTED;
			}
			else
				*length = sent;
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED;

	return err;
}

void HidDaqDevice::queryRawCmd(const unsigned char *outdata, size_t outLength, unsigned char *indata, size_t* inLength, unsigned int timeout) const
{
	UlLock lock(mIoMutex);

	UlError err = query(outdata, outLength, indata, inLength, timeout);

	if(err)
		throw UlException(err);
}

UlError HidDaqDevice::query(const unsigned char *outdata, size_t outLength, unsigned char *indata, size_t* inLength, unsigned int timeout) const
{
	UlError err = ERR_NO_ERROR;
	int sent = 0;

	if(mConnected)
	{
		if(mDevHandle)
		{
			sent = hid_write(mDevHandle, outdata, outLength);

			if(sent == (int) outLength)
			{
				int received = 0;
				received = hid_read_timeout(mDevHandle, indata, *inLength, timeout);

				if(received == -1)
				{
					UL_LOG("#### hid_read failed");

					err = ERR_DEV_NOT_CONNECTED;
				}
				else
				{
					if (received == 0)
						err = ERR_DEAD_DEV;

					*inLength = received;
				}
			}
			else if (sent == -1)
			{
				UL_LOG("#### hid_write failed");

				err = ERR_DEV_NOT_CONNECTED;
			}
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED;

	return err;
}

void HidDaqDevice::flashLed(int flashCount) const
{
	sendCmd(CMD_FLASH_LED);
}

int HidDaqDevice::memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count, false);

	unsigned char bytesToRead;
	int totalBytesRead = 0;
	int bytesRead = 0;
	int remaining = count;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = 62;

	unsigned char cmd = CMD_MEM_READ;
	unsigned char* readBuff = buffer;
	unsigned short addr = address;

	do
	{
		bytesToRead = (remaining > maxTransfer ? maxTransfer : remaining);

		bytesRead = queryCmd(cmd, addr, 0, bytesToRead, readBuff, bytesToRead);

		remaining-= bytesRead;
		totalBytesRead += bytesRead;
		addr += bytesRead;
		readBuff += bytesRead;
	}
	while(remaining > 0);


	return totalBytesRead;
}

int HidDaqDevice::memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_WRITE, address, buffer, count, false);

	unsigned char bytesToWrite;
	int totalBytesWritten = 0;
	int bytesWritten = 0;
	int bytesRemaining = count;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = 59;

	unsigned char cmd = CMD_MEM_WRITE;
	unsigned char* writeBuff = buffer;
	unsigned short addr = address;

	while(bytesRemaining > 0)
	{
		bytesToWrite = bytesRemaining > maxTransfer ? maxTransfer : bytesRemaining;

		bytesWritten = sendCmd(cmd, addr, bytesToWrite, writeBuff, bytesToWrite);

		bytesRemaining -= bytesWritten;
		totalBytesWritten += bytesWritten;
		addr += bytesWritten;
		writeBuff += bytesWritten;
	}

	return totalBytesWritten;
}

} /* namespace ul */
