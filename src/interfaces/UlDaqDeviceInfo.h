/*
 * UlDaqDeviceInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDAQDEVICEINFO_H_
#define INTERFACES_ULDAQDEVICEINFO_H_

#include "../uldaq.h"
#include "UlDevMemInfo.h"
#

namespace ul
{

class UlDaqDeviceInfo
{
public:
	virtual ~UlDaqDeviceInfo() {};

	virtual unsigned int getProductId() const = 0;
	virtual bool hasAiDevice() const = 0;
	virtual bool hasAoDevice() const = 0;
	virtual bool hasDioDevice() const = 0;
	virtual bool hasCtrDevice() const = 0;
	virtual bool hasTmrDevice() const = 0;
	virtual bool hasDaqIDevice() const = 0;
	virtual bool hasDaqODevice() const = 0;
	virtual DaqEventType getEventTypes() const = 0;
	virtual UlDevMemInfo& getMemInfo() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQDEVICEINFO_H_ */
