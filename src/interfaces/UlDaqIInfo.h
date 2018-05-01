/*
 * UlDaqIInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDAQIINFO_H_
#define INTERFACES_ULDAQIINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlDaqIInfo
{
public:
	virtual ~UlDaqIInfo() {};

	virtual DaqInChanType getChannelTypes() const = 0;
	virtual double getMinScanRate() const = 0;
	virtual double getMaxScanRate() const = 0;
	virtual double getMaxThroughput() const = 0;
	virtual int getFifoSize() const = 0;
	virtual ScanOption getScanOptions() const = 0;
	virtual TriggerType getTriggerTypes() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQIINFO_H_ */
