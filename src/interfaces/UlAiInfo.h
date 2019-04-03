/*
 * UlAiInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULAIINFO_H_
#define INTERFACES_ULAIINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlAiInfo
{
public:
	virtual ~UlAiInfo() {};

	virtual int getNumChans() const = 0;
	virtual int getNumChansByMode(AiInputMode mode) const = 0;
	virtual int getNumChansByType(AiChanType chanType) const = 0;
	virtual int getResolution() const = 0;
	virtual double getMinScanRate() const = 0;
	virtual double getMaxScanRate() const = 0;
	virtual double getMaxThroughput() const = 0;
	virtual double getMaxBurstThroughput() const = 0;
	virtual double getMaxBurstRate()const = 0;
	virtual int getFifoSize() const = 0;
	virtual ScanOption getScanOptions() const = 0;
	virtual bool hasPacer() const = 0;
	virtual AiChanType getChanTypes() const = 0;

	virtual void getRanges(AiInputMode mode, Range ranges[], int* count) const = 0;
	virtual int getRangeCountByMode(AiInputMode mode) const = 0;
	virtual Range getRangeByMode(AiInputMode mode, unsigned int index) const = 0;

	virtual bool supportsTrigger() const = 0;
	virtual TriggerType getTriggerTypes() const = 0;

	virtual int getMaxQueueLength(AiInputMode mode) const = 0;
	virtual AiQueueType getQueueTypes() const = 0;

	virtual AiChanQueueLimitation getChanQueueLimitations() const = 0;

	virtual long long getTInFlags() const = 0;
	virtual long long getTInArrayFlags() const = 0;
	virtual int getNumCjcChans() const = 0;

	virtual void supportsIepe(bool val) = 0;
	virtual bool supportsIepe() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULAIINFO_H_ */
