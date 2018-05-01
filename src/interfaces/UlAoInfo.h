/*
 * UlAoInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULAOINFO_H_
#define INTERFACES_ULAOINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlAoInfo
{
public:
	virtual ~UlAoInfo() {};

	virtual int getNumChans() const = 0;
	virtual int getResolution() const = 0;
	virtual double getMinScanRate() const = 0;
	virtual double getMaxScanRate() const = 0;
	virtual double getMaxThroughput() const = 0;
	virtual int getFifoSize() const = 0;
	virtual ScanOption getScanOptions() const = 0;
	virtual bool hasPacer() const = 0;

	virtual void getRanges(Range ranges[], int* count) const = 0;
	virtual int getRangeCount() const = 0;
	virtual Range getRange(unsigned int index) const = 0;

	virtual bool supportsTrigger() const = 0;
	virtual TriggerType getTriggerTypes() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULAOINFO_H_ */
