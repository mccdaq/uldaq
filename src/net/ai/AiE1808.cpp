/*
 * AiE1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiE1808.h"
#include <limits.h>

namespace ul
{

AiE1808::AiE1808(const NetDaqDevice& daqDevice) : AiVirNetBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 8);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setResolution(18);
	mAiInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::E_1808X)
	{
		mAiInfo.setMaxScanRate(200000);
		mAiInfo.setMaxThroughput(8 * 200000);
	}
	else
	{
		mAiInfo.setMaxScanRate(50000);
		mAiInfo.setMaxThroughput(8 * 50000);
	}

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalCoefsStartAddr(0x7000);
	mAiInfo.setCalDateAddr(0x7110);
	mAiInfo.setCalCoefCount(32);
	mAiInfo.setSampleSize(4);

	addSupportedRanges();
	addQueueInfo();

	// no used but required for validation
	initCustomScales();
}

AiE1808::~AiE1808()
{
	// TODO Auto-generated destructor stub
}


void AiE1808::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, UNI10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, UNI5VOLTS);

	mAiInfo.addRange(AI_DIFFERENTIAL, BIP10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, UNI10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, UNI5VOLTS);
}

void AiE1808::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 8);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE | MODE_QUEUE);

	// Add queue limitations
	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN);
}

} /* namespace ul */
