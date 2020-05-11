/*
 * UsbDaqDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USBDAQDEVICE_H_
#define USBDAQDEVICE_H_

#include <libusb-1.0/libusb.h>
#include <vector>
#include <map>

#include "../uldaq.h"
#include "../DaqDevice.h"
#include "../UlException.h"
#include "../utility/SuspendMonitor.h"

namespace ul
{

class UsbScanTransferIn;
class UsbScanTransferOut;

#define NO_PERMISSION_STR		"NO PERMISSION"

class UL_LOCAL UsbDaqDevice: public DaqDevice
{
public:
	enum {MCC_USB_VID = 0x09db, DT_USB_VID = 0x0867};
	typedef enum {CMD_FLASH_LED_KEY = 1, CMD_RESET_KEY = 2, CMD_STATUS_KEY = 3, CMD_SERIAL_NUM_KEY = 4,
		  	  	  CMD_MEM_KEY = 10, CMD_MEM_ADDR_KEY = 11, CMD_MEM_CAL_KEY = 12, CMD_MEM_USER_KEY = 13,
				  CMD_MEM_SETTINGS_KEY = 14, CMD_MEM_RESERVED_KEY = 15} CmdKey;

	enum {MAX_CMD_READ_TRANSFER = 256, MAX_CMD_WRITE_TRANSFER = 256};

public:
	UsbDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbDaqDevice();

	static std::vector<DaqDeviceDescriptor> findDaqDevices();

	virtual void connect();
	virtual void disconnect();

	UsbScanTransferIn* scanTranserIn() const;
	UsbScanTransferOut* scanTranserOut() const;

	virtual void flashLed(int flashCount) const;
	void clearFifo(unsigned char epAddr) const;

	virtual int sendCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout = 1000) const;
	virtual int queryCmd(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char *buff, uint16_t buffLen, unsigned int timeout = 1000, bool checkReplySize = true) const;

	int sendCmd(uint8_t request, unsigned int timeout = 1000) const { return sendCmd(request, 0, 0, NULL, 0, timeout);}

	int memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	int memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	int clearHalt(unsigned char endpoint) const;

	UlError syncBulkTransfer(unsigned char endpoint, unsigned char* buffer, int length, int* transferred, unsigned int timeout) const;

	libusb_transfer* allocTransfer() const;
	UlError asyncBulkTransfer(libusb_transfer* transfer, unsigned char endpoint, unsigned char* buffer, int length,
						libusb_transfer_cb_fn callback, void* userData,  unsigned int timeout) const;

	UlError syncInterruptTransfer(unsigned char endpoint, unsigned char* buffer, int length, int* transferred, unsigned int timeout) const;

	static void terminateEventThread();

	int getBulkEndpointMaxPacketSize(int epAddr) const;

	static void readSerialNumber(libusb_device* dev, libusb_device_descriptor descriptor, char* serialNum);
	static void readProductName(libusb_device *dev, libusb_device_descriptor descriptor, char* productName);

	int getOverrunBitMask() const;
	int getUnderrunBitMask() const;
	int getScanRunningBitMask(ScanDirection direction) const;
	int getScanDoneBitMask() const;

	unsigned char getCmdValue(CmdKey cmdKey) const;

	static void setUsbEventHandlerThreadPriority( int niceValue);
	static int getUsbEventHandlerThreadPriority();

	pthread_mutex_t& getTriggerCmdMutex() const { return mTriggerCmdMutex; }

	virtual void setupTrigger(FunctionType functionType, ScanOption options) const {};

	static void usb_init();
	static void usb_exit();
	static const libusb_context* getLibUsbContext() {return mLibUsbContext;}

protected:
	UlError restablishConnection() const;

	void setCmdValue(CmdKey cmdKey, unsigned char cmdValue);

	void setMemMaxReadSize(MemoryType memType, unsigned char maxSize);
	unsigned char getMemMaxReadSize(MemoryType memType) const;

	void setMemMaxWriteSize(MemoryType memType, unsigned char maxSize);
	unsigned char getMemMaxWriteSize(MemoryType memType) const;

	void setMultiCmdMem(bool multiCmd);
	bool hasMultiCmdMem() const;

	void setOverrunBitMask(int bitMask);
	void setUnderrunBitMask(int bitMask);
	void setScanRunningBitMask(ScanDirection direction, int bitMask);
	void setScanDoneBitMask(int bitMask);

private:
	UlError send(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char* buff, uint16_t buffLen, int* sent, unsigned int timeout) const;
	UlError query(uint8_t request, uint16_t wValue, uint16_t wIndex, unsigned char* buff, uint16_t buffLen, int* received, unsigned int timeout, bool checkReplySize) const;

	void setMemAddress(MemoryType memType, unsigned short address) const;
	virtual int memRead_SingleCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	virtual int memWrite_SingleCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	virtual int memRead_MultiCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	virtual int memWrite_MultiCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	static bool loadFirmware(DaqDeviceDescriptor daqDeviceDescriptor);

	static bool isHidDevice(libusb_device *dev);

	static unsigned int getVirtualProductId(libusb_device* dev, libusb_device_descriptor descriptor);
	static unsigned int getActualProductId(unsigned int vendorId, unsigned int vProductId);

private:
	virtual void establishConnection();
	virtual void initilizeHardware() const {};
	void releaseUsbResources();
	static int hotplugCallback(struct libusb_context* ctx, struct libusb_device* dev, libusb_hotplug_event event, void *user_data);
	static void registerHotplugCallBack();
	static void startEventHandlerThread();
	static void* eventHandlerThread(void* arg);

private:
	libusb_device_handle* 	mDevHandle;
	mutable pthread_mutex_t mConnectionMutex;
	mutable pthread_mutex_t mTriggerCmdMutex;
	std::vector<libusb_endpoint_descriptor> mBulkInEndpointDescs;
	std::vector<libusb_endpoint_descriptor> mBulkOutEndpointDescs;

	static libusb_context* mLibUsbContext;
	static libusb_hotplug_callback_handle mHotplugHandle;
	static pthread_t mUsbEventHandlerThread;
	static pid_t mUsbEventHandlerThreadId;
	static int mUsbEventHandlerThreadNiceValue;
	static bool mUsbEventThreadStarted;
	static bool mTerminateUsbEventThread;

	mutable std::map<CmdKey,uint8_t> mCmdMap;

	UsbScanTransferIn* mScanTransferIn;
	UsbScanTransferOut* mScanTransferOut;

	int mOverrunBitMask;
	int mUnderrunBitMask;
	int mScanRunningMask[2];
	int mScanDoneMask;

	mutable std::map<MemoryType,uint8_t> mMemMaxReadSizeMap;
	mutable std::map<MemoryType,uint8_t> mMemMaxWriteSizeMap;

	bool mMultiCmdMem;
protected:
	mutable pthread_mutex_t mIoMutex;
};

} /* namespace ul */

#endif /* USBDAQDEVICE_H_ */
