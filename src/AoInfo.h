/*
 * AoInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AOINFO_H_
#define AOINFO_H_

#include "ul_internal.h"
#include <vector>
#include "interfaces/UlAoInfo.h"

namespace ul
{

class UL_LOCAL AoInfo: public UlAoInfo
{
public:
	AoInfo();
	virtual ~AoInfo();

	void addRange(Range range);
	std::vector<Range> getRanges() const;

	void setNumChans(int numChans);
	int getNumChans() const;
	void setResolution(int resolution);
	int getResolution() const;
	void setMinScanRate(double minRate);
	double getMinScanRate() const;
	void setMaxScanRate(double maxRate);
	double getMaxScanRate() const;
	void setMaxThroughput(double maxThroughput);
	double getMaxThroughput() const;
	void setFifoSize(int size);
	int getFifoSize() const;
	void setScanOptions(long long options);
	ScanOption getScanOptions() const;

	void setAOutFlags(long long flags);
	long long getAOutFlags() const;
	void setAOutArrayFlags(long long flags);
	long long getAOutArrayFlags() const;
	void setAOutScanFlags(long long flags);
	long long getAOutScanFlags() const;

	bool hasPacer() const;
	void hasPacer(bool hasPacer);

	void setTriggerTypes(long long triggerTypes);
	TriggerType getTriggerTypes() const;
	bool supportsTrigger() const;

	void getRanges(Range ranges[], int* count) const;
	int getRangeCount() const;
	Range getRange(unsigned int index) const;

	void setCalCoefCount(int count);
	int getCalCoefCount() const;

	void setCalCoefsStartAddr(int count);
	int getCalCoefsStartAddr() const;

	void setCalDateAddr(int addr);
	int getCalDateAddr() const;

	void setSampleSize(int size);
	int getSampleSize() const;

	bool isRangeSupported(Range range) const;

private:
	std::vector<Range> mRanges;
	TriggerType mTriggerTypes;
	int mNumChans;
	int mResolution;
	double mMinScanRate;
	double mMaxScanRate;
	double mMaxThroughput;
	ScanOption mScanOptions;
	int mFifoSize;
	bool mHasPacer;

	int mCalCoefsCount;
	int mCalCoefsStartAddr;
	int mCalDateAddr;

	int mSampleSize;

	long long mAOutFlags;
	long long mAOutArrayFlags;
	long long mAOutScanFlags;
};

} /* namespace ul */

#endif /* AOINFO_H_ */
