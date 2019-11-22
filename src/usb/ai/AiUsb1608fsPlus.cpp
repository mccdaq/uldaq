/*
 * AiUsb1608fsPlus.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsb1608fsPlus.h"

namespace ul
{
AiUsb1608fs_Plus::AiUsb1608fs_Plus(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_BURSTIO | SO_PACEROUT);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setResolution(16);
	mAiInfo.setMinScanRate(minRate);

	mAiInfo.setMaxScanRate(100000);
	mAiInfo.setMaxThroughput(400000);

	mAiInfo.setMaxBurstRate(100000);
	mAiInfo.setMaxBurstThroughput(800000);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);

	mAiInfo.setCalCoefsStartAddr(0);
	mAiInfo.setCalDateAddr(0x200);
	mAiInfo.setCalCoefCount(64);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x81);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();
}

AiUsb1608fs_Plus::~AiUsb1608fs_Plus()
{
}

void AiUsb1608fs_Plus::initialize()
{
	try
	{
		sendStopCmd();

		loadAdcCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double AiUsb1608fs_Plus::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	unsigned short rangeCode = mapRangeCode(range);

	daqDev().queryCmd(CMD_AIN, channel, rangeCode, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb1608fs_Plus::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	long long totalCount = (long long) samplesPerChan * chanCount;

	//If no i/o mode is specified and scan meets the requirements for burst i/o mode then enable burst i/o mode,
	if(!(options & (SO_SINGLEIO | SO_BLOCKIO | SO_BURSTIO | SO_CONTINUOUS)) &&
		(totalCount <= (mAiInfo.getFifoSize() / mAiInfo.getSampleSize())) && rate > 1000.0)
		options = (ScanOption) (options | SO_BURSTIO);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	if((options & SO_EXTCLOCK) && (options & SO_PACEROUT))
		throw UlException(ERR_BAD_OPTION);

	int epAddr = getScanEndpointAddr();
	TAINSCAN_CFG scanCfg = {0};

	setTransferMode(options, rate);

	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	daqDev().clearFifo(epAddr);

	aInConfig(lowChan, highChan, range);

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	scanCfg = scanConfig(lowChan, highChan, samplesPerChan, rate, options);

	try
	{
		daqDev().sendCmd(CMD_AINSCAN_START, 0, 0, (unsigned char*) &scanCfg, sizeof(scanCfg), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

void AiUsb1608fs_Plus::aInConfig(int lowChan, int highChan, Range range)
{
	int chan;

	unsigned char gainArray[8];
	memset(gainArray, 0, 8);


	if (!queueEnabled())
	{
		for (chan = lowChan; chan <= highChan; chan++)
			gainArray[chan] = mapRangeCode(range);
	}
	else
	{
		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			chan = mAQueue.at(idx).channel;
			gainArray[chan] = mapRangeCode(mAQueue.at(idx).range);
		}
	}

	daqDev().sendCmd(CMD_AINSCAN_CONFIG, 0, 0, gainArray, sizeof(gainArray), 1000);
}

AiUsb1608fs_Plus::TAINSCAN_CFG AiUsb1608fs_Plus::scanConfig(int lowChan, int highChan, int scanCount, double rate, ScanOption options)
{
	TAINSCAN_CFG scanCfg;

	scanCfg.chan_mask = getChannelMask(lowChan, highChan);

	scanCfg.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));

	scanCfg.options = getOptionsCode(options);

	scanCfg.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		scanCfg.scan_count = 0;

	return scanCfg;
}

unsigned char AiUsb1608fs_Plus::getChannelMask(int lowChan, int highChan) const
{
	unsigned char chanMask = 0;

	if(!queueEnabled())
	{
		for(int chan = lowChan; chan <= highChan; chan++)
			chanMask |= 0x01 << chan;
	}
	else
	{
		for(int i = 0; i < queueLength() ; i++)
			chanMask  |=  0x01 << mAQueue[i].channel;
	}

	return chanMask;
}

unsigned char AiUsb1608fs_Plus::getOptionsCode(ScanOption options) const
{
	unsigned char optCode = (getTransferMode() & SO_BLOCKIO) ? 0 : 1;

	if(!(options & SO_EXTCLOCK))
		optCode |= (options & SO_PACEROUT) ? (1 << 1) : 0;

	if(options & SO_EXTTRIGGER)
	{
		unsigned char trigMode = getTrigModeCode();
		optCode |= trigMode << 2;      // enable trigger
	}

	optCode |= 1 << 7; // inhibit stall

	return optCode;
}

int AiUsb1608fs_Plus::mapRangeCode(Range range) const
{
	int rangeCode;

	switch(range)
	{
	case BIP10VOLTS:
		rangeCode = 0;
		break;
	case BIP5VOLTS:
		rangeCode = 1;
		break;
	case BIP2VOLTS:
		rangeCode = 3;
		break;
	case BIP1VOLTS:
		rangeCode = 5;
		break;
	default:
		throw UlException(ERR_BAD_RANGE);
	}

	return rangeCode;
}

int AiUsb1608fs_Plus::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	int gainIndex;

	switch(range)
	{
	case BIP10VOLTS:
		gainIndex = 0;
		break;
	case BIP5VOLTS:
		gainIndex = 1;
		break;
	case BIP2VOLTS:
		gainIndex = 3;
		break;
	case BIP1VOLTS:
		gainIndex = 5;
		break;
	default:
		throw UlException(ERR_BAD_RANGE);
	}

	int calCoefIndex = mAiInfo.getNumChansByMode(inputMode) * gainIndex + channel;
	return calCoefIndex;
}

unsigned char AiUsb1608fs_Plus::getTrigModeCode() const
{
	unsigned char code;
	switch (mTrigCfg.type)
	{
		case TRIG_POS_EDGE:
			code = TRIG_EDGE_RISING;
			break;
		case TRIG_NEG_EDGE:
			code = TRIG_EDGE_FALLING;
			break;
		case TRIG_HIGH:
			code = TRIG_LEVEL_HIGH;
			break;
		case TRIG_LOW:
			code = TRIG_LEVEL_LOW;
			break;
		default:
			throw UlException(ERR_BAD_TRIG_TYPE);
	}

	return code;
}

void AiUsb1608fs_Plus::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP2VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP1VOLTS);
}

void AiUsb1608fs_Plus::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 0);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	// Add queue types
	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);

	// Add queue limitations
	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN | ASCENDING_CHAN);
}

} /* namespace ul */
