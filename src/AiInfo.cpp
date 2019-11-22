/*
 * AiInfo.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include <algorithm>
#include <bitset>

#include "AiInfo.h"

namespace ul
{

AiInfo::AiInfo()
{
	mResolution = 0;
	mMinScanRate = 0;
	mMaxScanRate = 0;
	mMaxBurstRate = 0;
	mMaxThroughput = 0;
	mMaxBurstThroughput = 0;
	mScanOptions = SO_DEFAULTIO;
	mFifoSize = 0;
	mMaxQueueLengthSE = 0;
	mMaxQueueLengthDiff = 0;
	mMaxQueueLengthPseudoDiff = 0;
	mHasPacer = 0;
	mHasTempChan = 0;
	mCalCoefsCount = 0;
	mCalCoefsStartAddr = -1;
	mCalDateAddr = -1;
	mSampleSize = 0;
	mAInFlags = 0;
	mAInScanFlags = 0;
	mQueueTypes = (AiQueueType) 0;
	mChanQueueLimitations = (AiChanQueueLimitation) 0;
	mTriggerTypes = TRIG_NONE;
	mTypes = (AiChanType) 0;
	mTInFlags = 0;
	mTInArrayFlags = 0;
	mNumCjcChans = 0;

	mSupportsIepe = false;
}

AiInfo::~AiInfo()
{

}

void AiInfo::setNumChans(int numChans)
{
	if(mAiChanInfo.size())
		mAiChanInfo.clear();

	for(int ch = 0; ch < numChans; ch++)
		mAiChanInfo.push_back(AiChanInfo(ch));
}

int AiInfo::getNumChans() const
{
	return mAiChanInfo.size();
}

void AiInfo::setNumChansByMode(AiInputMode mode, int numChans)
{
	for(int ch = 0; ch < numChans; ch++)
		mAiChanInfo[ch].addChanMode(mode);
}

int AiInfo::getNumChansByMode(AiInputMode mode) const
{
	int numChans = 0;

	for(unsigned int ch = 0; ch < mAiChanInfo.size(); ch++)
	{
		std::vector<AiInputMode> inputModes = mAiChanInfo[ch].getChanModes();

		if(std::find(inputModes.begin(), inputModes.end(), mode) != inputModes.end())
			numChans++;
	}

	return numChans;
}

int AiInfo::getNumChansByType(AiChanType chanType) const
{
	int numChans = 0;

	std::bitset<32> typeBitSet(chanType);

	if(typeBitSet.count() == 1)
	{
		for(unsigned int ch = 0; ch < mAiChanInfo.size(); ch++)
		{
			AiChanType chanTypes = mAiChanInfo[ch].getChanTypes();

			if(chanTypes & chanType)
				numChans++;
		}
	}

	return numChans;
}

void AiInfo::setChanTypes(long long type)
{
	mTypes = (AiChanType) type;
}

AiChanType AiInfo::getChanTypes() const
{
	return mTypes;
}


void AiInfo::setResolution(int resolution)
{
	mResolution = resolution;
}

int AiInfo::getResolution() const
{
	return mResolution;
}

void AiInfo::setMinScanRate(double minRate)
{
	mMinScanRate = minRate;
}

double AiInfo::getMinScanRate() const
{
	return mMinScanRate;
}

void AiInfo::setMaxScanRate(double maxRate)
{
	mMaxScanRate = maxRate;
}

double AiInfo::getMaxScanRate() const
{
	return mMaxScanRate;
}

void AiInfo::setMaxThroughput(double maxThroughput)
{
	mMaxThroughput = maxThroughput;
}

double AiInfo::getMaxThroughput() const
{
	return mMaxThroughput;
}

void AiInfo::setMaxBurstThroughput(double maxThroughput)
{
	mMaxBurstThroughput = maxThroughput;
}

double AiInfo::getMaxBurstThroughput() const
{
	return mMaxBurstThroughput;
}

void AiInfo::setMaxBurstRate(double maxRate)
{
	mMaxBurstRate = maxRate;
}

double AiInfo::getMaxBurstRate() const
{
	return mMaxBurstRate;
}

void AiInfo::setFifoSize(int size)
{
	mFifoSize = size;
}

int AiInfo::getFifoSize() const
{
	return mFifoSize;
}

void AiInfo::setScanOptions(long long options)
{
	mScanOptions = (ScanOption) options;
}

ScanOption AiInfo::getScanOptions() const
{
	return mScanOptions;
}

void AiInfo::setAInFlags(long long flags)
{
	mAInFlags = flags;
}

long long AiInfo::getAInFlags() const
{
	return mAInFlags;
}

void AiInfo::setAInScanFlags(long long flags)
{
	mAInScanFlags = flags;
}

long long AiInfo::getAInScanFlags() const
{
	return mAInScanFlags;
}

void AiInfo::setTInFlags(long long flags)
{
	mTInFlags = flags;
}

long long AiInfo::getTInFlags() const
{
	return mTInFlags;
}

void AiInfo::setTInArrayFlags(long long flags)
{
	mTInArrayFlags = flags;
}

long long AiInfo::getTInArrayFlags() const
{
	return mTInArrayFlags;
}

void AiInfo::setNumCjcChans(int numChans)
{
	mNumCjcChans = numChans;
}

int AiInfo::getNumCjcChans() const
{
	return mNumCjcChans;
}

void AiInfo::addRange(AiInputMode mode, Range range)
{
	if(mode == AI_SINGLE_ENDED)
	{
		mSERanges.push_back(range);
	}
	else if (mode == AI_DIFFERENTIAL)
	{
		mDiffRanges.push_back(range);
	}
	else if (mode == AI_PSEUDO_DIFFERENTIAL)
	{
		mPseudoDiffRanges.push_back(range);
	}
}

std::vector<Range> AiInfo::getRanges(AiInputMode mode) const
{
	std::vector<Range> modeRanges;

	if(mode == AI_SINGLE_ENDED)
		modeRanges = mSERanges;
	else if (mode == AI_DIFFERENTIAL )
		modeRanges = mDiffRanges;
	else if (mode == AI_PSEUDO_DIFFERENTIAL)
		modeRanges = mPseudoDiffRanges;

	return modeRanges;
}

void AiInfo::getRanges(AiInputMode mode, Range ranges[], int* count) const
{
	std::vector<Range> modeRanges;

	if(mode == AI_SINGLE_ENDED)
		modeRanges = mSERanges;
	else if (mode == AI_DIFFERENTIAL )
		modeRanges = mDiffRanges;
	else if (mode == AI_PSEUDO_DIFFERENTIAL)
		modeRanges = mPseudoDiffRanges;

	if(modeRanges.size() <= (unsigned int)*count )
		std::copy(modeRanges.begin(), modeRanges.end(), ranges);

	*count = modeRanges.size();
}

int AiInfo::getRangeCountByMode(AiInputMode mode) const
{
	return getRanges(mode).size();
}

Range AiInfo::getRangeByMode(AiInputMode mode, unsigned int index) const
{
	Range range = (Range) 0;

	if(index < getRanges(mode).size())
	{
		range = getRanges(mode).at(index);
	}

	return range;
}

void AiInfo::addInputMode(AiInputMode mode)
{
	mInputModes.push_back(mode);
}


std::vector<AiInputMode> AiInfo::getInputModes() const
{
	return mInputModes;
}

void AiInfo::setTriggerTypes(long long triggerTypes)
{
	mTriggerTypes = (TriggerType) triggerTypes;
}


TriggerType AiInfo::getTriggerTypes() const
{
	return mTriggerTypes;
}


bool AiInfo::supportsTrigger() const
{
	bool supportsTrig = false;

	if(mTriggerTypes)
		supportsTrig = true;

	return supportsTrig;
}

void AiInfo::setMaxQueueLength(AiInputMode mode, int length)
{
	if(mode == AI_SINGLE_ENDED)
		mMaxQueueLengthSE = length;
	else if (mode == AI_DIFFERENTIAL)
		mMaxQueueLengthDiff = length;
	else if (mode == AI_PSEUDO_DIFFERENTIAL)
		mMaxQueueLengthPseudoDiff = length;
}


int AiInfo::getMaxQueueLength(AiInputMode mode) const
{
	int length = 0;
	if(mode == AI_SINGLE_ENDED)
		length =  mMaxQueueLengthSE;
	else if (mode == AI_DIFFERENTIAL)
		length = mMaxQueueLengthDiff;
	else if (mode == AI_PSEUDO_DIFFERENTIAL)
		length = mMaxQueueLengthPseudoDiff;

	return length;
}

void AiInfo::setQueueTypes(long long type)
{
	mQueueTypes = (AiQueueType) type;
}
AiQueueType AiInfo::getQueueTypes() const
{
	return mQueueTypes;
}

void AiInfo::setChanQueueLimitations(long long limitation)
{
	mChanQueueLimitations = (AiChanQueueLimitation) limitation;
}

AiChanQueueLimitation AiInfo::getChanQueueLimitations() const
{
	return mChanQueueLimitations;
}

bool AiInfo::hasPacer() const
{
	return mHasPacer;
}

void AiInfo::hasPacer(bool hasPacer)
{
	mHasPacer = hasPacer;
}


bool AiInfo::hasTempChan() const
{
	return mHasTempChan;
}

void AiInfo::hasTempChan(bool hasTempChan)
{
	mHasTempChan = hasTempChan;
}

void AiInfo::setChanTypes(int firtChan, int lastChan, long long chanTypes)
{
	for(int ch = firtChan; ch <=lastChan; ch++)
		mAiChanInfo[ch].setChanTypes(chanTypes);
}

AiChanInfo AiInfo::getChanInfo(int chan) const
{
	AiChanInfo aiChanInfo(-1);

	if((unsigned int) chan < mAiChanInfo.size())
		aiChanInfo = mAiChanInfo[chan];

	return aiChanInfo;
}

void AiInfo::setCalCoefCount(int count)
{
	mCalCoefsCount = count;
}

int AiInfo::getCalCoefCount() const
{
	return mCalCoefsCount;
}

void AiInfo::setCalCoefsStartAddr(int count)
{
	mCalCoefsStartAddr = count;
}
int AiInfo::getCalCoefsStartAddr() const
{
	return mCalCoefsStartAddr;
}

void AiInfo::setCalDateAddr(int addr)
{
	mCalDateAddr = addr;
}
int AiInfo::getCalDateAddr() const
{
	return mCalDateAddr;
}

void AiInfo::setSampleSize(int size)
{
	mSampleSize = size;
}

int AiInfo::getSampleSize() const
{
	return mSampleSize;
}

bool AiInfo::isInputModeSupported(AiInputMode inputMode) const
{
	bool supported = false;

	std::vector<AiInputMode>::const_iterator itr = std::find(mInputModes.begin(), mInputModes.end(), inputMode);

	if(itr != mInputModes.end())
		supported = true;

	return supported;
}

bool AiInfo::isRangeSupported(AiInputMode inputMode, Range range) const
{
	bool supported = false;

	std::vector<Range> ranges = getRanges(inputMode);

	if(!ranges.empty())
	{
		std::vector<Range>::const_iterator itr = std::find(ranges.begin(), ranges.end(), range);

		if(itr != ranges.end())
			supported = true;
	}

	return supported;
}

void AiInfo::supportsIepe(bool val)
{
	mSupportsIepe = val;
}
bool AiInfo::supportsIepe() const
{
	return mSupportsIepe;
}
} /* namespace ul */
