/*
 * DaqOInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQOINFO_H_
#define DAQOINFO_H_

#include <vector>

#include "interfaces/UlDaqOInfo.h"
#include "ul_internal.h"

namespace ul
{

class UL_LOCAL DaqOInfo: public UlDaqOInfo
{
public:
	DaqOInfo();
	virtual ~DaqOInfo();

	void setChannelTypes(long long chanTypes);
	DaqOutChanType getChannelTypes() const;

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
	void setDaqOutScanFlags(long long flags);
	long long getDaqOutScanFlags() const;

	void setMaxQueueLength(int length);
	int getMaxQueueLength() const;

	void setTriggerTypes(long long triggerTypes);
	TriggerType getTriggerTypes() const;
	bool supportsTrigger() const;

private:
	DaqOutChanType mChanTypes;
	TriggerType mTriggerTypes;
	double mMinScanRate;
	double mMaxScanRate;
	double mMaxThroughput;
	double mMaxBurstRate;
	double mMaxBurstThroughput;
	ScanOption mScanOptions;
	int mFifoSize;
	int mMaxQueueLength;
	long long mDaqOutScanFlags;
};

} /* namespace ul */

#endif /* DAQOINFO_H_ */
