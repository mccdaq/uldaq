/*
 * HidDaqDevice.h
 *
 *    Author: Measurement Computing Corporation
 */

#ifndef HID_HIDDAQDEVICE_H_
#define HID_HIDDAQDEVICE_H_

#include <vector>

#include "hidapi.h"
#include "../DaqDevice.h"
#include "../uldaq.h"
#include "../UlException.h"
#include "../utility/UlLock.h"

namespace ul
{

class UL_LOCAL HidDaqDevice: public DaqDevice
{
public:
	HidDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~HidDaqDevice();

	static void hidapi_init();
	static void hidapi_exit();

	static std::vector<DaqDeviceDescriptor> findDaqDevices();

	virtual void connect();
	virtual void disconnect();

	void sendCmd(unsigned char cmd) const;
	void sendCmd(unsigned char cmd, unsigned char param) const;
	void sendCmd(unsigned char cmd, unsigned char param1, unsigned char param2) const;
	void sendCmd(unsigned char cmd, unsigned char param1, unsigned char param2, unsigned char param3) const;
	void sendCmd(unsigned char cmd, unsigned char param1, unsigned short param2, unsigned char param3) const;
	void sendCmd(unsigned char cmd, unsigned char* dataBuffer, unsigned int dataBufferSize) const;
	void sendCmd(unsigned char cmd, unsigned short param, unsigned char* dataBuffer, unsigned int dataBufferSize) const;
	int sendCmd(unsigned char cmd, unsigned short param1, unsigned char param2, unsigned char* dataBuffer, unsigned int dataBufferSize) const;

	void queryCmd(unsigned char cmd, unsigned char* data, unsigned int timeout = 2000) const;
	void queryCmd(unsigned char cmd, unsigned int* data, unsigned int timeout = 2000) const;
	void queryCmd(unsigned char cmd, unsigned char param, unsigned char* data, unsigned int timeout = 2000) const;
	void queryCmd(unsigned char cmd, unsigned char param1, unsigned char param2, unsigned char* data, unsigned int timeout = 2000) const;
	void queryCmd(unsigned char cmd, unsigned short* data, unsigned int timeout = 2000) const;
	void queryCmd(unsigned char cmd, unsigned char* dataBuffer, unsigned int dataBufferSize, unsigned int timeout = 2000) const;
	unsigned int queryCmd(unsigned char cmd, unsigned char param1, unsigned char param2, unsigned char param3, unsigned char* dataBuffer, unsigned int dataBufferSize, unsigned int timeout = 2000) const;
	unsigned int queryCmd(unsigned char cmd, unsigned short param1, unsigned char param2, unsigned char param3, unsigned char* dataBuffer, unsigned int dataBufferSize, unsigned int timeout = 2000) const;
	void queryCmd(unsigned char cmd, unsigned char param1, unsigned char param2, float* data, unsigned int timeout = 2000) const;
	void sendRawCmd(const unsigned char *data, size_t* length) const;
	void queryRawCmd(const unsigned char *outdata, size_t outLength, unsigned char *indata, size_t* inLength, unsigned int timeout = 2000) const;

	virtual void flashLed(int flashCount) const;

	int memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	int memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

private:
	void establishConnection();
	virtual void initilizeHardware() const {};
	void releaseHidResources();

	virtual UlError send(const unsigned char *data, size_t* length) const;
	virtual UlError query(const unsigned char *outdata, size_t outLength, unsigned char *indata, size_t* inLength, unsigned int timeout) const;

public:
	enum { MCC_USB_VID = 0x09db };
	enum { CMD_MEM_READ = 0x30, CMD_MEM_WRITE = 0x31, CMD_FLASH_LED = 0x40 };

private:
	hid_device* mDevHandle;
	static bool mInitialized;
	mutable pthread_mutex_t mConnectionMutex;
	mutable pthread_mutex_t mIoMutex;

};

} /* namespace ul */

#endif /* HID_HIDDAQDEVICE_H_ */
