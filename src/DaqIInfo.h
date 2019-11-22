/*
 * DaqIInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQIINFO_H_
#define DAQIINFO_H_

#include <vector>

#include "interfaces/UlDaqIInfo.h"
#include "ul_internal.h"

namespace ul
{

class UL_LOCAL DaqIInfo: public UlDaqIInfo
{
public:
	DaqIInfo();
	virtual ~DaqIInfo();

	void setChannelTypes(long long chanTypes);
	DaqInChanType getChannelTypes() const;

	void setMinScanRate(double minRate);
	double getMinScanRate() const;
	void setMaxScanRate(double maxRate);
	double getMaxScanRate() const;
	void setMaxThroughput(double maxThroughput);
	double getMaxThroughput() const;
	void setMaxBurstThroughput(double maxThroughput);
	double getMaxBurstThroughput() const;
	void setMaxBurstRate(double maxRate);
	double getMaxBurstRate() const;
	void setFifoSize(int size);
	int getFifoSize() const;
	void setScanOptions(long long options);
	ScanOption getScanOptions() const;
	void setDaqInScanFlags(long long flags);
	long long getDaqInScanFlags() const;

	void setMaxQueueLength(int length);
	int getMaxQueueLength() const;

	void setTriggerTypes(long long triggerTypes);
	TriggerType getTriggerTypes() const;
	bool supportsTrigger() const;

private:
	DaqInChanType mChanTypes;
	TriggerType mTriggerTypes;
	double mMinScanRate;
	double mMaxScanRate;
	double mMaxThroughput;
	double mMaxBurstRate;
	double mMaxBurstThroughput;
	ScanOption mScanOptions;
	int mFifoSize;
	int mMaxQueueLength;
	long long mDaqInScanFlags;
};

} /* namespace ul */

#endif /* DAQIINFO_H_ */
