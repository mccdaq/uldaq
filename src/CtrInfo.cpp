/*
 * CtrInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrInfo.h"
#include <algorithm>

namespace ul
{

CtrInfo::CtrInfo()
{
	mNumCtrs = 0;
	mResolution = 0;
	mMinScanRate = 0;
	mMaxScanRate = 0;
	mMaxThroughput = 0;
	mMaxBurstRate = 0;
	mMaxBurstThroughput = 0;
	mScanOptions = SO_DEFAULTIO;
	mCInScanFlags = 0;
	mFifoSize = 0;
	mHasPacer = 0;
	mTriggerTypes = TRIG_NONE;
	mCtrRegTypes = CRT_COUNT;
}

CtrInfo::~CtrInfo()
{

}

void CtrInfo::addCtr(unsigned long long measureTypeMask)
{
	mCtrMeasureTypeMasks.push_back(measureTypeMask);
}

int CtrInfo::getNumCtrs() const
{
	return mCtrMeasureTypeMasks.size();
}

unsigned long long CtrInfo::getCtrMeasurementTypes(unsigned int ctrNum) const
{
	unsigned long long type = 0;

	if(ctrNum < mCtrMeasureTypeMasks.size())
		type = mCtrMeasureTypeMasks[ctrNum];

	return type;
}

void CtrInfo::setCtrMeasurementModes(CounterMeasurementType type ,long long mode)
{
	mCtrMeasureModes.insert(std::pair<CounterMeasurementType,CounterMeasurementMode>(type, (CounterMeasurementMode) mode));
}

CounterMeasurementMode CtrInfo::getCtrMeasurementModes(CounterMeasurementType type) const
{
	CounterMeasurementMode mode = (CounterMeasurementMode) 0;

	std::map<CounterMeasurementType,CounterMeasurementMode>::iterator itr = mCtrMeasureModes.find(type);

	if(itr != mCtrMeasureModes.end())
		mode = mCtrMeasureModes[type];

	return mode;
}

void CtrInfo::setResolution(int resolution)
{
	mResolution = resolution;
}

int CtrInfo::getResolution() const
{
	return mResolution;
}

void CtrInfo::setMinScanRate(double minRate)
{
	mMinScanRate = minRate;
}

double CtrInfo::getMinScanRate() const
{
	return mMinScanRate;
}

void CtrInfo::setMaxScanRate(double maxRate)
{
	mMaxScanRate = maxRate;
}

double CtrInfo::getMaxScanRate() const
{
	return mMaxScanRate;
}

void CtrInfo::setMaxThroughput(double maxThroughput)
{
	mMaxThroughput = maxThroughput;
}

double CtrInfo::getMaxThroughput() const
{
	return mMaxThroughput;
}

void CtrInfo::setMaxBurstThroughput(double maxThroughput)
{
	mMaxBurstThroughput = maxThroughput;
}

double CtrInfo::getMaxBurstThroughput() const
{
	return mMaxBurstThroughput;
}

void CtrInfo::setMaxBurstRate(double maxRate)
{
	mMaxBurstRate = maxRate;
}

double CtrInfo::getMaxBurstRate() const
{
	return mMaxBurstRate;
}

void CtrInfo::setFifoSize(int size)
{
	mFifoSize = size;
}

int CtrInfo::getFifoSize() const
{
	return mFifoSize;
}

void CtrInfo::setScanOptions(long long options)
{
	mScanOptions = (ScanOption) options;
}

ScanOption CtrInfo::getScanOptions() const
{
	return mScanOptions;
}
/*
void CtrInfo::addTriggerType(TriggerType triggerType)
{
	mTriggerTypes.push_back(triggerType);
}*/

void CtrInfo::setCInScanFlags(long long flags)
{
	mCInScanFlags = flags;
}

long long CtrInfo::getCInScanFlags() const
{
	return mCInScanFlags;
}

void CtrInfo::setTriggerTypes(long long triggerTypes)
{
	mTriggerTypes = (TriggerType) triggerTypes;
}


TriggerType CtrInfo::getTriggerTypes() const
{
	return mTriggerTypes;
}


bool CtrInfo::supportsTrigger() const
{
	bool supportsTrig = false;

	if(mTriggerTypes)
		supportsTrig = true;

	return supportsTrig;
}

void CtrInfo::setRegisterTypes(long long registerTypes)
{
	mCtrRegTypes = (CounterRegisterType) registerTypes;
}

CounterRegisterType CtrInfo::getRegisterTypes() const
{
	return mCtrRegTypes;
}

void CtrInfo::addDebounceTime(CounterDebounceTime debounceTime)
{
	mCtrDebounceTimes.push_back(debounceTime);
}


std::vector<CounterDebounceTime> CtrInfo::getDebounceTimes() const
{
	return mCtrDebounceTimes;
}

void CtrInfo::addTickSize(CounterTickSize tickSize)
{
	mCtrTickSizes.push_back(tickSize);
}


std::vector<CounterTickSize> CtrInfo::getTickSizes() const
{
	return mCtrTickSizes;
}


bool CtrInfo::hasPacer() const
{
	return mHasPacer;
}

void CtrInfo::hasPacer(bool hasPacer)
{
	mHasPacer = hasPacer;
}

} /* namespace ul */
