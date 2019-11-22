/*
 * UsbDaqDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "UsbDaqDevice.h"
#include "../DaqDeviceManager.h"
#include "../utility/UlLock.h"
#include "UsbScanTransferIn.h"
#include "UsbScanTransferOut.h"
#include "UsbDtDevice.h"

#if LIBUSBX_API_VERSION < 0x01000102
#error libusb version 1.0.16 or later is required to compile this package.
#endif

namespace ul
{

libusb_context* UsbDaqDevice::mLibUsbContext = NULL;
libusb_hotplug_callback_handle UsbDaqDevice::mHotplugHandle;
pthread_t UsbDaqDevice::mUsbEventHandlerThread;


bool UsbDaqDevice::mUsbEventThreadStarted = false;
bool UsbDaqDevice::mTerminateUsbEventThread = false;

pid_t UsbDaqDevice::mUsbEventHandlerThreadId = 0;
int UsbDaqDevice::mUsbEventHandlerThreadNiceValue = 0;

UsbDaqDevice::UsbDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor) : DaqDevice(daqDeviceDescriptor)
{
	FnLog log("UsbDaqDevice::UsbDaqDevice");

	mDevHandle = NULL;
	mConnected = false;

	mScanDoneMask = 0;
	mOverrunBitMask = 0;
	mUnderrunBitMask = 0;
	memset(mScanRunningMask, 0, sizeof(mScanRunningMask));

	UlLock::initMutex(mConnectionMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mIoMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mTriggerCmdMutex, PTHREAD_MUTEX_RECURSIVE);

	mScanTransferIn = new UsbScanTransferIn(*this);
	mScanTransferOut = new UsbScanTransferOut(*this);

	mMultiCmdMem = false;

	setCmdValue(CMD_FLASH_LED_KEY, 0x40);
	setCmdValue(CMD_RESET_KEY, 0x41);
	setCmdValue(CMD_STATUS_KEY, 0x44);

	setCmdValue(CMD_MEM_KEY, 0x30);
	setCmdValue(CMD_MEM_ADDR_KEY, 0x31);

	setCmdValue(CMD_MEM_CAL_KEY, 0x30);
	setCmdValue(CMD_MEM_USER_KEY, 0x31);

	setMemMaxReadSize(MT_EEPROM, 64);
	setMemMaxWriteSize(MT_EEPROM, 64);
}

UsbDaqDevice::~UsbDaqDevice()
{
	FnLog log("UsbDaqDevice::~UsbDaqDevice");

	disconnect();

	delete mScanTransferIn;
	mScanTransferIn = NULL;

	delete mScanTransferOut;
	mScanTransferOut = NULL;

	UlLock::destroyMutex(mIoMutex);
	UlLock::destroyMutex(mConnectionMutex);
	UlLock::destroyMutex(mTriggerCmdMutex);
}

void UsbDaqDevice::usb_init()
{
	if(mLibUsbContext == NULL)
	{
		int status = libusb_init(&mLibUsbContext);

		if(status != LIBUSB_SUCCESS)
			UL_LOG("libusb_init() failed :" << libusb_error_name(status));
	}
}

void UsbDaqDevice::usb_exit()
{
	if(mLibUsbContext)
	{
		terminateEventThread();

		libusb_exit(mLibUsbContext);
	}
}

std::vector<DaqDeviceDescriptor> UsbDaqDevice::findDaqDevices()
{
	std::vector<DaqDeviceDescriptor> descriptorList;

	FnLog log("UsbDaqDevice::getDaqDeviceDescriptorList");

	libusb_device** devs;
	libusb_device* dev;
	int ret = 0;

	int numDevs = libusb_get_device_list (mLibUsbContext, &devs);

	int mccDaqDevNum = 0;

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

			if((desc.idVendor == MCC_USB_VID || desc.idVendor == DT_USB_VID) &&
			   DaqDeviceManager::isDaqDeviceSupported(desc.idProduct, desc.idVendor))
			{
				if(!isHidDevice(dev))
				{
					DaqDeviceDescriptor daqDevDescriptor;
					memset(&daqDevDescriptor, 0,sizeof(DaqDeviceDescriptor));

					daqDevDescriptor.productId = getVirtualProductId(dev, desc);
					daqDevDescriptor.devInterface = USB_IFC;
					std::string productName = DaqDeviceManager::getDeviceName(daqDevDescriptor.productId, desc.idVendor);

					strncpy(daqDevDescriptor.productName, productName.c_str(), sizeof(daqDevDescriptor.productName) - 1);
					strncpy(daqDevDescriptor.devString, productName.c_str(), sizeof(daqDevDescriptor.devString) - 1);

					readSerialNumber(dev, desc, daqDevDescriptor.uniqueId);

					UL_LOG("-----------------------");
					UL_LOG("Product ID : 0x" << std::hex << daqDevDescriptor.productId << std::dec);
					UL_LOG("Product Name: "<< daqDevDescriptor.productName);
					UL_LOG("Serial Number : "<< daqDevDescriptor.uniqueId);
					UL_LOG("-----------------------");

					descriptorList.push_back(daqDevDescriptor);

					mccDaqDevNum++;
				}
			}
		}
	}
	else
		UL_LOG("No USB DAQ device found!");

	libusb_free_device_list(devs, 1);

	return descriptorList;
}

void UsbDaqDevice::connect()
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

	mCurrentSuspendCount = SuspendMonitor::instance().getCurrentSystemSuspendCount();

	initilizeHardware();

	initializeIoDevices();

	// start the daq event handler if daq events are already enabled
	if(mEventHandler->getEnabledEventTypes())
		mEventHandler->start();
}

void UsbDaqDevice::disconnect()
{
	FnLog log("UsbDaqDevice::disconnect");

	if(mConnected)
	{
		DaqDevice::disconnect();

		releaseUsbResources();
	}
}

void UsbDaqDevice::releaseUsbResources()
{
	FnLog log("UsbDaqDevice::releaseUsbResources");

	if(mDevHandle)
	{
		UlLock lock(mIoMutex);

		libusb_release_interface(mDevHandle, 0);
		libusb_close(mDevHandle);

		mDevHandle = NULL;
	}
}

void UsbDaqDevice::establishConnection()
{
	FnLog log("UsbDaqDevice::establishConnection");

	libusb_device **devs;
	libusb_device *dev;
	int status;
	bool found = false;

	if(std::strcmp(mDaqDeviceDescriptor.uniqueId, NO_PERMISSION_STR) == 0)
		throw UlException(ERR_USB_DEV_NO_PERMISSION);

	int numDevs = libusb_get_device_list (mLibUsbContext, &devs);

	if(numDevs > 0)
	{
		int devNum = 0;
		unsigned int actualProductId;

		while ((dev = devs[devNum++]) != NULL)
		{
			struct libusb_device_descriptor desc;
			memset(&desc, 0,sizeof(libusb_device_descriptor));

			status = LIBUSB_SUCCESS;

			status = libusb_get_device_descriptor(dev, &desc);

			if (status != LIBUSB_SUCCESS)
			{
				UL_LOG("failed to get device descriptor");
				continue;
			}

			actualProductId = getActualProductId(desc.idVendor, mDaqDeviceDescriptor.productId);

			if((((desc.idVendor == MCC_USB_VID || desc.idVendor == DT_USB_VID) && DaqDeviceManager::isDaqDeviceSupported(desc.idProduct, desc.idVendor))) &&
			   (actualProductId == desc.idProduct))
			{
				char serialNum[128] = {0};

				readSerialNumber(dev, desc, serialNum);

				if(std::strcmp(serialNum, mDaqDeviceDescriptor.uniqueId) == 0)
				{
					UL_LOG("-----------------------");
					UL_LOG("USB Device Found.");

					mRawFwVersion = desc.bcdDevice;

					if(mRawFwVersion < mMinRawFwVersion)
					{
						libusb_free_device_list(devs, 1);
						throw UlException(ERR_INCOMPATIBLE_FIRMWARE);
					}

					int status = libusb_open(dev, &mDevHandle);

					if (status == LIBUSB_SUCCESS)
					{
						status = libusb_claim_interface(mDevHandle, 0);

						if (status == LIBUSB_SUCCESS)
						{
							found = true;

							struct libusb_config_descriptor *config;
							status = libusb_get_config_descriptor(dev, 0, &config);

							if (status == LIBUSB_SUCCESS)
							{
								if(config->bNumInterfaces > 0)
								{
									UL_LOG("NumInterfaces: " << (int) config->bNumInterfaces);

									int  numEP = config->interface[0].altsetting[0].bNumEndpoints;

									UL_LOG("NumEndpoints: " << numEP);

									mBulkInEndpointDescs.clear();
									mBulkOutEndpointDescs.clear();

									for(int i = 0; i < numEP; i++)
									{
										libusb_endpoint_descriptor endpointDesc = config->interface[0].altsetting[0].endpoint[i];
										if(endpointDesc.bmAttributes & LIBUSB_TRANSFER_TYPE_BULK)
										{
											UL_LOG("EndpointAddress: 0x" << std::hex << (int) endpointDesc.bEndpointAddress << std::dec);

											if(endpointDesc.bEndpointAddress & LIBUSB_ENDPOINT_IN)
												mBulkInEndpointDescs.push_back(endpointDesc);
											else
												mBulkOutEndpointDescs.push_back(endpointDesc);
										}
									}
								}

								libusb_free_config_descriptor(config);

								if(!mUsbEventThreadStarted)
								{
									// register Hotplug callback and start the event handler thread only once for all usb devices
									registerHotplugCallBack();
									startEventHandlerThread();
								}
							}
							else
								UL_LOG("libusb_get_config_descriptor() failed: " << libusb_error_name(status));
						}
						else
						{
							UL_LOG("libusb_claim_interface() failed: " << libusb_error_name(status));
							libusb_free_device_list(devs, 1);

							throw UlException(ERR_USB_INTERFACE_CLAIMED);
						}
					}
					else
						UL_LOG("libusb_open() failed: " << libusb_error_name(status));

					UL_LOG("-----------------------");

					break;
				}
			}
		}
	}
	else
		UL_LOG("No USB device found!");


	libusb_free_device_list(devs, 1);

	if(!found)
		throw UlException(ERR_DEV_NOT_FOUND);

#ifdef __APPLE__
	usleep(100000);
#endif
}

UlError UsbDaqDevice::restablishConnection() const
{
	FnLog log("UsbDaqDevice::restablishConnection");

	UlError err = ERR_NO_ERROR;
	try
	{
		const_cast<UsbDaqDevice*>(this)->releaseUsbResources();
		const_cast<UsbDaqDevice*>(this)->establishConnection();

		initilizeHardware();
	}
	catch(UlException& e)
	{
		err = e.getError();
	}
	catch(...)
	{
		err = ERR_UNHANDLED_EXCEPTION;
	}

	return err;
}

int UsbDaqDevice::sendCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout) const
{
	int sent = 0;

	UlLock lock(mIoMutex);

	UlError err = send(request, wValue, wIndex, buff, buffLen, &sent, timeout);

	if(err)
		throw UlException(err);

	return sent;
}

// this function is not thread safe. Always use sendCmd
UlError UsbDaqDevice::send(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, int* sent, unsigned int timeout) const
{
	UlError err = ERR_NO_ERROR;
	int status = 0;

	if(mConnected)
	{
		uint8_t requestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;

		if(mDevHandle)
		{
			status = libusb_control_transfer(mDevHandle, requestType, request, wValue, wIndex, buff, buffLen, timeout);

			if (status != buffLen)
			{
				if (status < 0)
					UL_LOG("#### libusb_control_transfer failed : " << libusb_error_name(status));
				else
					UL_LOG("#### libusb_control_transfer failed : " << status);

				if(status == LIBUSB_ERROR_NO_DEVICE)
					err = ERR_DEV_NOT_CONNECTED; //ERR_DEV_NOT_FOUND;
				else
					err = ERR_DEAD_DEV;
			}
			else
			{
				*sent = buffLen;
				UL_LOG("Bytes sent: " << *sent);
			}
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED; // ERR_DEV_NOT_CONNECTED;

	return err;
}

int UsbDaqDevice::queryCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout, bool checkReplySize) const
{
	int received = 0;

	UlLock lock(mIoMutex);

	UlError err = query(request, wValue, wIndex, buff, buffLen, &received, timeout, checkReplySize);

	if(err)
		throw UlException(err);

	return received;
}

// this function is not thread safe. Always use sendCmd
UlError UsbDaqDevice::query(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, int* recevied, unsigned int timeout, bool checkReplySize) const
{
	UlError err = ERR_NO_ERROR;
	int status = 0;

	if(mConnected)
	{
		uint8_t requestType = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;

		if(mDevHandle)
		{
			status = libusb_control_transfer(mDevHandle, requestType, request, wValue, wIndex, buff, buffLen, timeout);

			if (status < 0)
			{
				if (status < 0)
					UL_LOG("#### libusb_control_transfer failed : " << libusb_error_name(status));
				else
					UL_LOG("#### libusb_control_transfer failed : " << status);

				if(status == LIBUSB_ERROR_NO_DEVICE)
					err = ERR_DEV_NOT_CONNECTED; // ERR_DEV_NOT_FOUND;
				else if(status == LIBUSB_ERROR_OVERFLOW)
					err = ERR_BAD_BUFFER_SIZE;
				else
					err = ERR_DEAD_DEV;
			}
			else
			{
				if(checkReplySize)
				{
					if(status != buffLen)
						err = ERR_DEAD_DEV;
				}
				*recevied = buffLen;

				UL_LOG("Bytes received: " << *recevied);
			}
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED; //ERR_DEV_NOT_CONNECTED;

	return err;
}

UsbScanTransferIn* UsbDaqDevice::scanTranserIn() const
{
	if(mScanTransferIn == NULL)
		UL_LOG("mScanTransferIn is NULL");

	return mScanTransferIn;
}

UsbScanTransferOut* UsbDaqDevice::scanTranserOut() const
{
	if(mScanTransferOut == NULL)
		UL_LOG("mScanTransferOut is NULL");

	return mScanTransferOut;
}

void UsbDaqDevice::flashLed(int flashCount) const
{
	unsigned char buff = flashCount;

	uint8_t cmd = getCmdValue(CMD_FLASH_LED_KEY);

	sendCmd(cmd, 0, 0, &buff, sizeof(buff));
}

void UsbDaqDevice::readSerialNumber(libusb_device *dev, libusb_device_descriptor descriptor, char* serialNum)
{
	if(descriptor.idVendor == DT_USB_VID)
		return UsbDtDevice::readSerialNumber(dev, descriptor, serialNum);

	libusb_device_handle *devHandle = NULL;

	int status = libusb_open(dev, &devHandle);

	if (status == 0)
	{
		unsigned char serial[128] = {0};
		int numBytes = libusb_get_string_descriptor_ascii(devHandle, descriptor.iSerialNumber, serial, sizeof(serial));

		if(numBytes > 0 )
			strcpy(serialNum, (char*)serial);

		libusb_close(devHandle);
	}
	else
	{
		if(status == LIBUSB_ERROR_ACCESS)
			strcpy(serialNum, NO_PERMISSION_STR);

		std::cout << "libusb_open() failed: " << libusb_error_name(status);
	}
}

void UsbDaqDevice::readProductName(libusb_device *dev, libusb_device_descriptor descriptor, char* productName)
{
	libusb_device_handle *devHandle = NULL;

	int status = libusb_open(dev, &devHandle);

	if (status == 0)
	{
		unsigned char name[128] = {0};
		int numBytes = libusb_get_string_descriptor_ascii(devHandle, descriptor.iProduct, name, sizeof(name));

		if(numBytes > 0 )
			strcpy(productName, (char*)name);

		libusb_close(devHandle);
	}
	else
	{
		if(status == LIBUSB_ERROR_ACCESS)
			strcpy(productName, NO_PERMISSION_STR);

		std::cout << "libusb_open() failed: " << libusb_error_name(status);
	}
}

int UsbDaqDevice::memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	if(hasMultiCmdMem())
		return memRead_MultiCmd(memType, memRegionType, address, buffer, count);
	else
		return memRead_SingleCmd(memType, memRegionType, address, buffer, count);
}
int UsbDaqDevice::memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	if(hasMultiCmdMem())
		return memWrite_MultiCmd(memType, memRegionType, address, buffer, count);
	else
		return memWrite_SingleCmd(memType, memRegionType, address, buffer, count);
}


void UsbDaqDevice::setMemAddress(MemoryType memType, unsigned short address) const
{
	unsigned short buff = Endian::cpu_to_le_ui16(address);

	uint8_t cmd = getCmdValue(CMD_MEM_ADDR_KEY);

	sendCmd(cmd, 0, 0, (unsigned char*)&buff, sizeof(buff));
}

int UsbDaqDevice::memRead_SingleCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count, false);

	int bytesToRead;
	int totalBytesRead = 0;
	int bytesRead = 0;
	int remaining = count;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = getMemMaxReadSize(memType);

	if(maxTransfer)
	{
		setMemAddress(memType, address);

		unsigned char cmd = getCmdValue(CMD_MEM_KEY);

		unsigned char* readBuff = buffer;

		do
		{
			bytesToRead = (remaining > maxTransfer ? maxTransfer : remaining);

			bytesRead  = queryCmd(cmd, 0, 0, readBuff, bytesToRead);

			remaining-= bytesRead;
			totalBytesRead += bytesRead;
			readBuff += bytesRead;
		}
		while(remaining > 0);

	}
	else
		throw UlException(ERR_BAD_MEM_TYPE);


	return totalBytesRead;
}

int UsbDaqDevice::memWrite_SingleCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_WRITE, address, buffer, count, false);

	if(isScanRunning(FT_AO))
		throw UlException(ERR_DEV_UNAVAILABLE);

	int bytesToWrite;
	int totalBytesWritten = 0;
	int bytesWritten = 0;
	int bytesRemaining = count;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = getMemMaxWriteSize(memType);

	if(maxTransfer)
	{
		setMemAddress(memType, address);

		unsigned char cmd = getCmdValue(CMD_MEM_KEY);

		unsigned char* writeBuff = buffer;

		while(bytesRemaining > 0)
		{
			bytesToWrite = bytesRemaining > maxTransfer ? maxTransfer : bytesRemaining;

			bytesWritten = sendCmd(cmd, 0, 0, writeBuff, bytesToWrite);

			bytesRemaining -= bytesWritten;
			totalBytesWritten += bytesWritten;
			writeBuff += bytesWritten;
		}
	}
	else
		throw UlException(ERR_BAD_MEM_TYPE);

	return totalBytesWritten;
}

int UsbDaqDevice::memRead_MultiCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count, false);

	int bytesToRead;
	int totalBytesRead = 0;
	int bytesRead = 0;
	int remaining = count;
	unsigned char cmd;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = MAX_CMD_READ_TRANSFER;

	if(memRegionType == MR_CAL)
		cmd = getCmdValue(CMD_MEM_CAL_KEY);
	else if(memRegionType == MR_USER)
		cmd = getCmdValue(CMD_MEM_USER_KEY);
	else if(memRegionType == MR_SETTINGS)
		cmd = getCmdValue(CMD_MEM_SETTINGS_KEY);
	else if(memRegionType == MR_RESERVED0)
		cmd = getCmdValue(CMD_MEM_RESERVED_KEY);
	else
		throw UlException(ERR_BAD_MEM_REGION);

	unsigned char* readBuff = buffer;

	int addr = address;

	do
	{
		bytesToRead = remaining > maxTransfer ? maxTransfer : remaining;

		bytesRead  = queryCmd(cmd, addr, 0, readBuff, bytesToRead);

		remaining-= bytesRead;
		totalBytesRead += bytesRead;
		addr += bytesRead;
		readBuff += bytesRead;
	}
	while(remaining > 0);

	return totalBytesRead;
}

int UsbDaqDevice::memWrite_MultiCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_WRITE, address, buffer, count, false);

	if(isScanRunning(FT_AO))
		throw UlException(ERR_DEV_UNAVAILABLE);

	int bytesToWrite;
	int totalBytesWritten = 0;
	int bytesWritten = 0;
	int bytesRemaining = count;
	unsigned char cmd;

	if(buffer == NULL)
			throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = MAX_CMD_WRITE_TRANSFER;


	if(memRegionType == MR_CAL)
		cmd = getCmdValue(CMD_MEM_CAL_KEY);
	else if(memRegionType == MR_USER)
		cmd = getCmdValue(CMD_MEM_USER_KEY);
	else if(memRegionType == MR_SETTINGS)
		cmd = getCmdValue(CMD_MEM_SETTINGS_KEY);
	else if(memRegionType == MR_RESERVED0)
		cmd = getCmdValue(CMD_MEM_RESERVED_KEY);
	else
		throw UlException(ERR_BAD_MEM_REGION);

	int addr = address;

	unsigned char* writeBuff = buffer;

	while(bytesRemaining > 0)
	{
		bytesToWrite = bytesRemaining > maxTransfer ? maxTransfer : bytesRemaining;

		bytesWritten = sendCmd(cmd, addr, 0, writeBuff, bytesToWrite);

		bytesRemaining -= bytesWritten;
		totalBytesWritten += bytesWritten;
		addr += bytesWritten;
		writeBuff += bytesWritten;
	}

	return totalBytesWritten;
}

void UsbDaqDevice::setOverrunBitMask(int bitMask)
{
	mOverrunBitMask = bitMask;
}

int UsbDaqDevice::getOverrunBitMask() const
{
	return mOverrunBitMask;
}

void UsbDaqDevice::setUnderrunBitMask(int bitMask)
{
	mUnderrunBitMask = bitMask;
}

int UsbDaqDevice::getUnderrunBitMask() const
{
	return mUnderrunBitMask;
}

void UsbDaqDevice::setScanRunningBitMask(ScanDirection direction, int bitMask)
{
	mScanRunningMask[direction - SD_INPUT] = bitMask;
}

int UsbDaqDevice::getScanRunningBitMask(ScanDirection direction) const
{
	return mScanRunningMask[direction - SD_INPUT];
}

void UsbDaqDevice::setScanDoneBitMask(int bitMask)
{
	mScanDoneMask = bitMask;
}

int UsbDaqDevice::getScanDoneBitMask() const
{
	return mScanDoneMask;
}

void UsbDaqDevice::registerHotplugCallBack()
{
	FnLog log("UsbDaqDevice::registerHotplugCallBack");

	if(libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
	{
		int status = libusb_hotplug_register_callback(mLibUsbContext, (libusb_hotplug_event) (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
												  	  (libusb_hotplug_flag)0, MCC_USB_VID, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
												  	  hotplugCallback, NULL, &mHotplugHandle);
		if (status != LIBUSB_SUCCESS)
			UL_LOG("#### Error creating a hotplug callback");
	}

}

int UsbDaqDevice::hotplugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
	FnLog log("UsbDaqDevice::hotplugCallback");

	struct libusb_device_descriptor desc;

	(void)libusb_get_device_descriptor(dev, &desc);

	if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
	{
		UL_LOG("LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED");
	}
	else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)
	{
		UL_LOG("LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT");
	}
	else
		UL_LOG("Unhandled event " << event);

	UL_LOG("***************************");
	UL_LOG("Product ID : 0x" << std::hex << desc.idProduct << std::dec);
	std::string productName = DaqDeviceManager::getDeviceName(desc.idProduct);
	UL_LOG("Product Name: "<< productName);
	UL_LOG("***************************");

	return 0;
}

void UsbDaqDevice::startEventHandlerThread()
{
	FnLog log("UsbDaqDevice::startEventHandlerThread");

	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(!status)
	{
		status = pthread_create(&mUsbEventHandlerThread, &attr, &eventHandlerThread, NULL);

#ifndef __APPLE__
		pthread_setname_np(mUsbEventHandlerThread, "usb_xfer_td");
#endif

		if(status)
			UL_LOG("#### Unable to start the event handler thread");

		status = pthread_attr_destroy(&attr);
	}
	else
		UL_LOG("#### Unable to initialize attributes for the event handler thread");
}

void UsbDaqDevice::setUsbEventHandlerThreadPriority( int niceValue)
{
#ifndef __APPLE__
	if(niceValue >= -20 && niceValue <=0)  // don't allow nice values 1 to 19, it may cause overrun or underrun errors
	{
		if(mUsbEventThreadStarted)
			setpriority(PRIO_PROCESS, mUsbEventHandlerThreadId, niceValue);
		else
			mUsbEventHandlerThreadNiceValue = niceValue;
	}
	else
		throw UlException(ERR_BAD_CONFIG_VAL);
#endif
}

int UsbDaqDevice::getUsbEventHandlerThreadPriority()
{
#ifndef __APPLE__
	if(mUsbEventThreadStarted)
	{
		int niceValue = getpriority(PRIO_PROCESS, mUsbEventHandlerThreadId);
		return niceValue;
	}
	else
#endif
		return mUsbEventHandlerThreadNiceValue;
}

void* UsbDaqDevice::eventHandlerThread(void *arg)
{
	UL_LOG("USB Event handler started");

#ifndef __APPLE__
	mUsbEventHandlerThreadId = syscall(SYS_gettid); // note: syscall is deprecated in osx use pthread_threadid_np if this feature is needed

	if(mUsbEventHandlerThreadNiceValue != 0)
		setpriority(PRIO_PROCESS, 0, mUsbEventHandlerThreadNiceValue);
#endif

	mUsbEventThreadStarted = true;

	while (!mTerminateUsbEventThread)
	{
		libusb_handle_events(mLibUsbContext);
	}

	UL_LOG("USB Event handler terminated");

	return NULL;
}

void UsbDaqDevice::terminateEventThread()
{
	FnLog log("terminateEventThread");

	mTerminateUsbEventThread = true;

	if(libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
	{
		libusb_hotplug_deregister_callback(mLibUsbContext, mHotplugHandle); // This wakes up libusb_handle_events()
	}

	UL_LOG("waiting for event handler thread to complete....");

	if(mUsbEventHandlerThread)
		pthread_join(mUsbEventHandlerThread, NULL);

	mUsbEventHandlerThread = 0;
}

void UsbDaqDevice::setCmdValue(CmdKey cmdKey, uint8_t cmdValue)
{
	if(mCmdMap.count(cmdKey))
		mCmdMap.erase(cmdKey);

	mCmdMap[cmdKey] = cmdValue;
}


uint8_t UsbDaqDevice::getCmdValue(CmdKey cmdKey) const
{
	uint8_t cmdVal = 0;
	if(mCmdMap.count(cmdKey))
		cmdVal = mCmdMap[cmdKey];
	else
		UL_LOG("#### command is not defined");

	return cmdVal;
}

void UsbDaqDevice::setMemMaxReadSize(MemoryType memType, unsigned char maxSize)
{
	if(mMemMaxReadSizeMap.count(memType))
		mMemMaxReadSizeMap.erase(memType);

	mMemMaxReadSizeMap[memType] = maxSize;
}


unsigned char UsbDaqDevice::getMemMaxReadSize(MemoryType memType) const
{
	unsigned char maxSize = 0;
	if(mMemMaxReadSizeMap.count(memType))
		maxSize = mMemMaxReadSizeMap[memType];
	else
		UL_LOG("#### mem type not found");

	return maxSize;
}

void UsbDaqDevice::setMemMaxWriteSize(MemoryType memType, unsigned char maxSize)
{
	if(mMemMaxWriteSizeMap.count(memType))
		mMemMaxWriteSizeMap.erase(memType);

	mMemMaxWriteSizeMap[memType] = maxSize;
}


unsigned char UsbDaqDevice::getMemMaxWriteSize(MemoryType memType) const
{
	unsigned char maxSize = 0;
	if(mMemMaxWriteSizeMap.count(memType))
		maxSize = mMemMaxWriteSizeMap[memType];
	else
		UL_LOG("#### mem type not found");

	return maxSize;
}

void UsbDaqDevice::setMultiCmdMem(bool multiCmd)
{
	mMultiCmdMem = multiCmd;
}
bool UsbDaqDevice::hasMultiCmdMem() const
{
	return mMultiCmdMem;
}

void UsbDaqDevice::clearFifo(unsigned char epAddr) const
{
	int maxPacketSize = getBulkEndpointMaxPacketSize(epAddr);

	if(maxPacketSize > 0)
	{
		unsigned char* buffer = new unsigned char [getBulkEndpointMaxPacketSize(epAddr)];

		int transferred = 0;
		int ret = ERR_NO_ERROR;

		do
		{
			ret =  syncBulkTransfer(epAddr, buffer, getBulkEndpointMaxPacketSize(epAddr), &transferred, 1);
		}
		while(ret == ERR_NO_ERROR);

		delete[] buffer;
	}
	else
		std::cout << "*** invalid endpoint" << std::endl;
}

int UsbDaqDevice::getBulkEndpointMaxPacketSize(int epAddr) const
{
	int maxPacketSize = -1;

	if(epAddr & LIBUSB_ENDPOINT_IN)
	{
		for(std::vector<libusb_endpoint_descriptor>::const_iterator itr = mBulkInEndpointDescs.begin(); itr != mBulkInEndpointDescs.end(); itr++)
		{
			if((*itr).bEndpointAddress == epAddr)
			{
				maxPacketSize = (*itr).wMaxPacketSize;
				break;
			}
		}
	}
	else
	{
		for(std::vector<libusb_endpoint_descriptor>::const_iterator itr = mBulkOutEndpointDescs.begin(); itr != mBulkOutEndpointDescs.end(); itr++)
		{
			if((*itr).bEndpointAddress == epAddr)
			{
				maxPacketSize = (*itr).wMaxPacketSize;
				break;
			}
		}
	}

	return maxPacketSize;
}

int UsbDaqDevice::clearHalt(unsigned char endpoint) const
{
	return libusb_clear_halt(mDevHandle, endpoint);
}

UlError UsbDaqDevice::syncBulkTransfer(unsigned char endpoint, unsigned char* buffer, int length, int* transferred, unsigned int timeout) const
{
	UlError err = ERR_NO_ERROR;
	int status = 0;

	if(mConnected)
	{
		if(mDevHandle)
		{
			status = libusb_bulk_transfer(mDevHandle, endpoint, buffer, length, transferred, timeout);

			if (status != LIBUSB_SUCCESS)
			{
				UL_LOG("#### libusb_bulk_transfer failed : " << libusb_error_name(status));

				if(status == LIBUSB_ERROR_NO_DEVICE)
					err = ERR_DEV_NOT_CONNECTED;
				else
					err = ERR_DEAD_DEV;
			}
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED;

	return err;
}

UlError UsbDaqDevice::syncInterruptTransfer(unsigned char endpoint, unsigned char* buffer, int length, int* transferred, unsigned int timeout) const
{
	UlError err = ERR_NO_ERROR;
	int status = 0;

	if(mConnected)
	{
		if(mDevHandle)
		{
			status = libusb_interrupt_transfer(mDevHandle, endpoint, buffer, length, transferred, timeout);

			if (status != LIBUSB_SUCCESS)
			{
				UL_LOG("#### libusb_interrupt_transfer failed : " << libusb_error_name(status));

				if(status == LIBUSB_ERROR_NO_DEVICE)
					err = ERR_DEV_NOT_CONNECTED;
				else if(status == LIBUSB_ERROR_TIMEOUT)
					err = ERR_TIMEDOUT;
				else
					err = ERR_DEAD_DEV;
			}
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED;

	return err;
}

libusb_transfer* UsbDaqDevice::allocTransfer() const
{
	return libusb_alloc_transfer(0);
}

UlError UsbDaqDevice::asyncBulkTransfer(libusb_transfer* transfer, unsigned char endpoint, unsigned char* buffer, int length,
									libusb_transfer_cb_fn callback, void* userData,  unsigned int timeout) const
{
	UlError err = ERR_NO_ERROR;
	int status = 0;

	if(mConnected)
	{
		if(mDevHandle)
		{
			libusb_fill_bulk_transfer(transfer, mDevHandle, endpoint, buffer, length, callback, userData, timeout );
			status = libusb_submit_transfer(transfer);

			if (status != LIBUSB_SUCCESS)
			{
				UL_LOG("#### libusb_submit_transfer failed : " << libusb_error_name(status));

				if(status == LIBUSB_ERROR_NO_DEVICE)
					err = ERR_DEV_NOT_CONNECTED;
				else
					err = ERR_DEAD_DEV;
			}
		}
		else
			err = ERR_DEV_NOT_FOUND;
	}
	else
		err = ERR_NO_CONNECTION_ESTABLISHED;

	return err;
}

bool UsbDaqDevice::isHidDevice(libusb_device* dev)
{
	bool hidDevice = false;
	struct libusb_config_descriptor *conf_desc = NULL;

	int status = libusb_get_config_descriptor(dev, 0, &conf_desc);

	if (status == LIBUSB_SUCCESS)
	{
		if (conf_desc->bNumInterfaces > 0)
		{
			const struct libusb_interface *intf = &conf_desc->interface[0];

			if(intf->num_altsetting > 0)
			{
				const struct libusb_interface_descriptor *intf_desc;
				intf_desc = &intf->altsetting[0];
				if (intf_desc->bInterfaceClass == LIBUSB_CLASS_HID)
					hidDevice = true;
			}
		}

		libusb_free_config_descriptor(conf_desc);
	}

	return hidDevice;
}

unsigned int UsbDaqDevice::getVirtualProductId(libusb_device* dev, libusb_device_descriptor descriptor)
{
	unsigned int vProductId = descriptor.idProduct;

	if(descriptor.idVendor == DT_USB_VID)
	{
		vProductId = UsbDtDevice::getVirtualProductId(dev, descriptor);
	}

	return vProductId;
}

unsigned int UsbDaqDevice::getActualProductId(unsigned int vendorId, unsigned int vProductId)
{
	unsigned int productId = vProductId;

	if(vendorId == DT_USB_VID)
	{
		productId = UsbDtDevice::getActualProductId(vProductId);
	}

	return productId;
}

} /* namespace ul */
