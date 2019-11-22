/*
 * CtrInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef CTRINFO_H_
#define CTRINFO_H_

#include "ul_internal.h"
#include <vector>
#include <map>
#include "interfaces/UlCtrInfo.h"

namespace ul
{

class UL_LOCAL CtrInfo: public UlCtrInfo
{
public:
	CtrInfo();
	virtual ~CtrInfo();

	void addCtr(unsigned long long measureTypeMask);
	int getNumCtrs() const;
	unsigned long long getCtrMeasurementTypes(unsigned int ctrNum) const;
	void setCtrMeasurementModes(CounterMeasurementType type ,long long mode);
	CounterMeasurementMode getCtrMeasurementModes(CounterMeasurementType type) const;
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
	void setCInScanFlags(long long flags);
	long long getCInScanFlags() const;

	void setRegisterTypes(long long registerTypes);
	CounterRegisterType getRegisterTypes() const;

	void addDebounceTime(CounterDebounceTime debounceTime);
	std::vector<CounterDebounceTime> getDebounceTimes() const;

	void addTickSize(CounterTickSize tickSize);
	std::vector<CounterTickSize> getTickSizes() const;

	bool hasPacer() const;
	void hasPacer(bool hasPacer);

	void setTriggerTypes(long long triggerTypes);
	TriggerType getTriggerTypes() const;
	bool supportsTrigger() const;

private:
	std::vector<unsigned long long> mCtrMeasureTypeMasks;
	mutable std::map<CounterMeasurementType,CounterMeasurementMode> mCtrMeasureModes;
	CounterRegisterType mCtrRegTypes;
	TriggerType mTriggerTypes;
	std::vector<CounterDebounceTime> mCtrDebounceTimes;
	std::vector<CounterTickSize> mCtrTickSizes;
	int mNumCtrs;
	int mResolution;
	double mMinScanRate;
	double mMaxScanRate;
	double mMaxThroughput;
	double mMaxBurstRate;
	double mMaxBurstThroughput;
	ScanOption mScanOptions;
	long long mCInScanFlags;
	int mFifoSize;
	bool mHasPacer;
};

} /* namespace ul */

#endif /* CTRINFO_H_ */
