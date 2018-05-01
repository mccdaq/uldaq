/*
 * DaqOInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqOInfo.h"

namespace ul
{
DaqOInfo::DaqOInfo()
{
	mChanTypes = (DaqOutChanType) 0;
	mMinScanRate = 0;
	mMaxScanRate = 0;
	mMaxThroughput = 0;
	mMaxBurstRate = 0;
	mMaxBurstThroughput = 0;
	mScanOptions = SO_DEFAULTIO;
	mFifoSize = 0;
	mMaxQueueLength = 0;
	mDaqOutScanFlags = 0;
	mTriggerTypes = TRIG_NONE;
}

DaqOInfo::~DaqOInfo()
{

}

void DaqOInfo::setChannelTypes(long long chanTypes)
{
	mChanTypes = (DaqOutChanType) chanTypes;
}

DaqOutChanType DaqOInfo::getChannelTypes() const
{
	return mChanTypes;
}


void DaqOInfo::setMinScanRate(double minRate)
{
	mMinScanRate = minRate;
}

double DaqOInfo::getMinScanRate() const
{
	return mMinScanRate;
}

void DaqOInfo::setMaxScanRate(double maxRate)
{
	mMaxScanRate = maxRate;
}

double DaqOInfo::getMaxScanRate() const
{
	return mMaxScanRate;
}

void DaqOInfo::setMaxThroughput(double maxThroughput)
{
	mMaxThroughput = maxThroughput;
}

double DaqOInfo::getMaxThroughput() const
{
	return mMaxThroughput;
}

void DaqOInfo::setMaxBurstThroughput(double maxThroughput)
{
	mMaxBurstThroughput = maxThroughput;
}

double DaqOInfo::getMaxBurstThroughput() const
{
	return mMaxBurstThroughput;
}

void DaqOInfo::setMaxBurstRate(double maxRate)
{
	mMaxBurstRate = maxRate;
}

double DaqOInfo::getMaxBurstRate() const
{
	return mMaxBurstRate;
}


void DaqOInfo::setFifoSize(int size)
{
	mFifoSize = size;
}

int DaqOInfo::getFifoSize() const
{
	return mFifoSize;
}

void DaqOInfo::setScanOptions(long long options)
{
	mScanOptions = (ScanOption) options;
}

ScanOption DaqOInfo::getScanOptions() const
{
	return mScanOptions;
}


void DaqOInfo::setDaqOutScanFlags(long long flags)
{
	mDaqOutScanFlags = flags;
}

long long DaqOInfo::getDaqOutScanFlags() const
{
	return mDaqOutScanFlags;
}

void DaqOInfo::setTriggerTypes(long long triggerTypes)
{
	mTriggerTypes = (TriggerType) triggerTypes;
}


TriggerType DaqOInfo::getTriggerTypes() const
{
	return mTriggerTypes;
}

bool DaqOInfo::supportsTrigger() const
{
	bool supportsTrig = false;

	if(mTriggerTypes)
		supportsTrig = true;

	return supportsTrig;
}

void DaqOInfo::setMaxQueueLength(int length)
{
		mMaxQueueLength = length;
}


int DaqOInfo::getMaxQueueLength() const
{
	return mMaxQueueLength;
}

} /* namespace ul */
