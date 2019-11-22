/*
 * AiInfo.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef AIINFO_H_
#define AIINFO_H_

#include "ul_internal.h"
#include "AiChanInfo.h"
#include <vector>
#include "interfaces/UlAiInfo.h"

namespace ul
{

class UL_LOCAL AiInfo: public UlAiInfo
{
public:
	AiInfo();
	virtual ~AiInfo();

	void addRange(AiInputMode mode, Range range);
	std::vector<Range> getRanges(AiInputMode mode) const;

	void setNumChans(int numChans);
	int getNumChans() const;
	void setNumChansByMode(AiInputMode mode, int numChans);
	int getNumChansByMode(AiInputMode mode) const;
	int getNumChansByType(AiChanType chanType) const;
	void setChanTypes(long long types);
	AiChanType getChanTypes() const;
	void setResolution(int resolution);
	int getResolution() const;
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
	bool supportsTrigger() const;

	void setAInFlags(long long flags);
	long long getAInFlags() const;
	void setAInScanFlags(long long flags);
	long long getAInScanFlags() const;
	void setTInFlags(long long flags);
	long long getTInFlags() const;
	void setTInArrayFlags(long long flags);
	long long getTInArrayFlags() const;

	void addInputMode(AiInputMode mode);
	std::vector<AiInputMode> getInputModes() const;
	void setMaxQueueLength(AiInputMode mode, int length);
	int getMaxQueueLength(AiInputMode mode) const;
	void setQueueTypes(long long type);
	AiQueueType getQueueTypes() const;
	void setChanQueueLimitations(long long limitations);
	AiChanQueueLimitation getChanQueueLimitations() const;
	void setChanTypes(int firtChan, int lastChan, long long chanTypes);
	AiChanInfo getChanInfo(int chan) const;

	bool hasPacer() const;
	void hasPacer(bool hasPacer);
	bool hasTempChan() const;
	void hasTempChan(bool hasTempChan);

	void getRanges(AiInputMode mode, Range ranges[], int* count) const;
	int getRangeCountByMode(AiInputMode mode) const;
	Range getRangeByMode(AiInputMode mode, unsigned int index) const;

	void setTriggerTypes(long long triggerTypes);
	TriggerType getTriggerTypes() const;

	void setCalCoefCount(int count);
	int getCalCoefCount() const;

	void setCalCoefsStartAddr(int count);
	int getCalCoefsStartAddr() const;

	void setCalDateAddr(int addr);
	int getCalDateAddr() const;

	void setSampleSize(int size);
	int getSampleSize() const;

	bool isInputModeSupported(AiInputMode inputMode) const;
	bool isRangeSupported(AiInputMode inputMode, Range range) const;

	void setNumCjcChans(int numChans);
	int getNumCjcChans() const;

	void supportsIepe(bool val);
	bool supportsIepe() const;

private:
	std::vector<AiChanInfo> mAiChanInfo;
	std::vector<Range> mSERanges;
	std::vector<Range> mDiffRanges;
	std::vector<Range> mPseudoDiffRanges;
	std::vector<AiInputMode> mInputModes;
	AiChanType mTypes;
	TriggerType mTriggerTypes;
	AiQueueType mQueueTypes;
	AiChanQueueLimitation mChanQueueLimitations;
	int mResolution;
	double mMinScanRate;
	double mMaxScanRate;
	double mMaxBurstRate;
	double mMaxThroughput;
	double mMaxBurstThroughput;
	ScanOption mScanOptions;
	int mFifoSize;
	int mMaxQueueLengthSE;
	int mMaxQueueLengthDiff;
	int mMaxQueueLengthPseudoDiff;
	bool mHasPacer;
	bool mHasTempChan;

	int mCalCoefsCount;
	int mCalCoefsStartAddr;
	int mCalDateAddr;

	int mSampleSize;

	long long mAInFlags;
	long long mAInScanFlags;
	long long mTInFlags;
	long long mTInArrayFlags;

	int mNumCjcChans;
	bool mSupportsIepe;

};

} /* namespace ul */

#endif /* AIINFO_H_ */
