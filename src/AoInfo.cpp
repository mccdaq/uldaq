/*
 * AoInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoInfo.h"
#include <algorithm>

namespace ul
{

AoInfo::AoInfo()
{
	mNumChans = 0;
	mResolution = 0;
	mMinScanRate = 0;
	mMaxScanRate = 0;
	mMaxThroughput = 0;
	mScanOptions = SO_DEFAULTIO;
	mFifoSize = 0;
	mHasPacer = 0;
	mCalCoefsCount = 0;
	mCalCoefsStartAddr = -1;
	mCalDateAddr = -1;
	mSampleSize = 0;
	mAOutFlags = 0;
	mAOutArrayFlags = 0;
	mAOutScanFlags = 0;
	mTriggerTypes = TRIG_NONE;
}

AoInfo::~AoInfo()
{

}

void AoInfo::setNumChans(int numChans)
{
	mNumChans = numChans;
}

int AoInfo::getNumChans() const
{
	return mNumChans;
}

void AoInfo::setResolution(int resolution)
{
	mResolution = resolution;
}

int AoInfo::getResolution() const
{
	return mResolution;
}

void AoInfo::setMinScanRate(double minRate)
{
	mMinScanRate = minRate;
}

double AoInfo::getMinScanRate() const
{
	return mMinScanRate;
}

void AoInfo::setMaxScanRate(double maxRate)
{
	mMaxScanRate = maxRate;
}

double AoInfo::getMaxScanRate() const
{
	return mMaxScanRate;
}

void AoInfo::setMaxThroughput(double maxThroughput)
{
	mMaxThroughput = maxThroughput;
}

double AoInfo::getMaxThroughput() const
{
	return mMaxThroughput;
}

void AoInfo::setFifoSize(int size)
{
	mFifoSize = size;
}

int AoInfo::getFifoSize() const
{
	return mFifoSize;
}

void AoInfo::setScanOptions(long long options)
{
	mScanOptions = (ScanOption) options;
}

ScanOption AoInfo::getScanOptions() const
{
	return mScanOptions;
}

void AoInfo::setAOutFlags(long long flags)
{
	mAOutFlags = flags;
}

long long AoInfo::getAOutFlags() const
{
	return mAOutFlags;
}

void AoInfo::setAOutArrayFlags(long long flags)
{
	mAOutArrayFlags = flags;
}

long long AoInfo::getAOutArrayFlags() const
{
	return mAOutArrayFlags;
}

void AoInfo::setAOutScanFlags(long long flags)
{
	mAOutScanFlags = flags;
}

long long AoInfo::getAOutScanFlags() const
{
	return mAOutScanFlags;
}

void AoInfo::addRange(Range range)
{
	mRanges.push_back(range);
}

std::vector<Range> AoInfo::getRanges() const
{
	return mRanges;
}

void AoInfo::getRanges(Range ranges[], int* count) const
{

	if(mRanges.size() <= (unsigned int)*count )
		std::copy(mRanges.begin(), mRanges.end(), ranges);

	*count = mRanges.size();
}

int AoInfo::getRangeCount() const
{
	return getRanges().size();
}

Range AoInfo::getRange(unsigned int index) const
{
	Range range = (Range) 0;

	if(index < getRanges().size())
	{
		range = getRanges().at(index);
	}

	return range;
}

void AoInfo::setTriggerTypes(long long triggerTypes)
{
	mTriggerTypes = (TriggerType) triggerTypes;
}


TriggerType AoInfo::getTriggerTypes() const
{
	return mTriggerTypes;
}

bool AoInfo::supportsTrigger() const
{
	bool supportsTrig = false;

	if(mTriggerTypes)
		supportsTrig = true;

	return supportsTrig;
}


bool AoInfo::hasPacer() const
{
	return mHasPacer;
}

void AoInfo::hasPacer(bool hasPacer)
{
	mHasPacer = hasPacer;
}

void AoInfo::setCalCoefCount(int count)
{
	mCalCoefsCount = count;
}

int AoInfo::getCalCoefCount() const
{
	return mCalCoefsCount;
}

void AoInfo::setCalCoefsStartAddr(int count)
{
	mCalCoefsStartAddr = count;
}
int AoInfo::getCalCoefsStartAddr() const
{
	return mCalCoefsStartAddr;
}

void AoInfo::setCalDateAddr(int addr)
{
	mCalDateAddr = addr;
}
int AoInfo::getCalDateAddr() const
{
	return mCalDateAddr;
}

void AoInfo::setSampleSize(int size)
{
	mSampleSize = size;
}

int AoInfo::getSampleSize() const
{
	return mSampleSize;
}

bool AoInfo::isRangeSupported(Range range) const
{
	bool supported = false;

	std::vector<Range> ranges = getRanges();

	if(!ranges.empty())
	{
		std::vector<Range>::const_iterator itr = std::find(ranges.begin(), ranges.end(), range);

		if(itr != ranges.end())
			supported = true;
	}

	return supported;
}
} /* namespace ul */
