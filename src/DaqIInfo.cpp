/*
 * DaqIInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqIInfo.h"

#include <algorithm>

namespace ul
{

DaqIInfo::DaqIInfo()
{
	mChanTypes = (DaqInChanType) 0;
	mMinScanRate = 0;
	mMaxScanRate = 0;
	mMaxThroughput = 0;
	mMaxBurstRate = 0;
	mMaxBurstThroughput = 0;
	mScanOptions = SO_DEFAULTIO;
	mFifoSize = 0;
	mMaxQueueLength = 0;
	mDaqInScanFlags = 0;
	mTriggerTypes = TRIG_NONE;
}

DaqIInfo::~DaqIInfo()
{

}

void DaqIInfo::setChannelTypes(long long chanTypes)
{
	mChanTypes = (DaqInChanType) chanTypes;
}

DaqInChanType DaqIInfo::getChannelTypes() const
{
	return mChanTypes;
}


void DaqIInfo::setMinScanRate(double minRate)
{
	mMinScanRate = minRate;
}

double DaqIInfo::getMinScanRate() const
{
	return mMinScanRate;
}

void DaqIInfo::setMaxScanRate(double maxRate)
{
	mMaxScanRate = maxRate;
}

double DaqIInfo::getMaxScanRate() const
{
	return mMaxScanRate;
}

void DaqIInfo::setMaxThroughput(double maxThroughput)
{
	mMaxThroughput = maxThroughput;
}

double DaqIInfo::getMaxThroughput() const
{
	return mMaxThroughput;
}

void DaqIInfo::setMaxBurstThroughput(double maxThroughput)
{
	mMaxBurstThroughput = maxThroughput;
}

double DaqIInfo::getMaxBurstThroughput() const
{
	return mMaxBurstThroughput;
}

void DaqIInfo::setMaxBurstRate(double maxRate)
{
	mMaxBurstRate = maxRate;
}

double DaqIInfo::getMaxBurstRate() const
{
	return mMaxBurstRate;
}

void DaqIInfo::setFifoSize(int size)
{
	mFifoSize = size;
}

int DaqIInfo::getFifoSize() const
{
	return mFifoSize;
}

void DaqIInfo::setScanOptions(long long options)
{
	mScanOptions = (ScanOption) options;
}

ScanOption DaqIInfo::getScanOptions() const
{
	return mScanOptions;
}


void DaqIInfo::setDaqInScanFlags(long long flags)
{
	mDaqInScanFlags = flags;
}

long long DaqIInfo::getDaqInScanFlags() const
{
	return mDaqInScanFlags;
}

void DaqIInfo::setTriggerTypes(long long triggerTypes)
{
	mTriggerTypes = (TriggerType) triggerTypes;
}


TriggerType DaqIInfo::getTriggerTypes() const
{
	return mTriggerTypes;
}

bool DaqIInfo::supportsTrigger() const
{
	bool supportsTrig = false;

	if(mTriggerTypes)
		supportsTrig = true;

	return supportsTrig;
}

void DaqIInfo::setMaxQueueLength(int length)
{
		mMaxQueueLength = length;
}


int DaqIInfo::getMaxQueueLength() const
{
	return mMaxQueueLength;
}



} /* namespace ul */
