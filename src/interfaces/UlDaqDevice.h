/*
 * UlDaqDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDAQDEVICE_H_
#define INTERFACES_ULDAQDEVICE_H_

#include "UlAiDevice.h"
#include "UlAoDevice.h"
#include "UlCtrDevice.h"
#include "UlDaqDeviceConfig.h"
#include "UlDaqDeviceInfo.h"
#include "UlDaqIDevice.h"
#include "UlDaqODevice.h"
#include "UlDioDevice.h"
#include "UlTmrDevice.h"

namespace ul
{

class UlDaqDevice
{
public:
	virtual ~UlDaqDevice() {};

	virtual DaqDeviceDescriptor getDescriptor() const = 0;

	virtual void connect() = 0;
	virtual void disconnect() = 0;
	virtual void connectionCode(long long code) = 0;
	virtual void flashLed(int flashCount) const = 0;
	virtual const UlDaqDeviceInfo& getDevInfo() const = 0;
	virtual UlDaqDeviceConfig& getDevConfig() = 0;
	virtual UlAiDevice& getAiDevice() const = 0;
	virtual UlAoDevice& getAoDevice() const = 0;
	virtual UlDioDevice& getDioDevice() const = 0;
	virtual UlCtrDevice& getCtrDevice() const = 0;
	virtual UlTmrDevice& getTmrDevice() const = 0;
	virtual UlDaqIDevice& getDaqIDevice() const = 0;
	virtual UlDaqODevice& getDaqODevice() const = 0;

	virtual int memRead(MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const = 0;
	virtual int memWrite(MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQDEVICE_H_ */
