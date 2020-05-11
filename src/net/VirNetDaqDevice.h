/*
 * VirNetDaqDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_VIRNETDAQDEVICE_H_
#define NET_VIRNETDAQDEVICE_H_

#include "NetDaqDevice.h"
#include "../virnet.h"

namespace ul
{

class UL_LOCAL VirNetDaqDevice: public NetDaqDevice
{
public:
	VirNetDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~VirNetDaqDevice();

	virtual void flashLed(int flashCount) const;
	virtual unsigned short readStatus() const;

	virtual UlError openDataSocket(int timeout /* ms */) const;
	virtual void flushCmdSocket() const;
	virtual bool isDataSocketReady() const;

	virtual int memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	virtual int memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	TXferInState getXferInState() const;

private:
	virtual void closeDataSocketFromDevice() const;

/*private:
	enum { DEV_CMD = 1, AI_CMD = 2};
	enum { FLASH_LED = 1 };

#pragma pack(1)
	typedef struct
	{
		unsigned char function;
		unsigned char flashCount;
	}TFlashLed;
#pragma pack()*/

};

} /* namespace ul */

#endif /* NET_VIRNETDAQDEVICE_H_ */
