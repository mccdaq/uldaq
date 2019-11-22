/*
 * AiUsb20x.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsb20x.h"

namespace ul
{
AiUsb20x::AiUsb20x(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO |SO_BLOCKIO);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setResolution(12);
	mAiInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_204 ||
	   daqDev().getDeviceType() == DaqDeviceId::USB_205)
	{
		mAiInfo.setMaxScanRate(500000);
		mAiInfo.setMaxThroughput(500000);
	}
	else
	{
		mAiInfo.setMaxScanRate(100000);
		mAiInfo.setMaxThroughput(100000);
	}

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);

	mAiInfo.setCalCoefsStartAddr(0);
	mAiInfo.setCalDateAddr(0x40);
	mAiInfo.setCalCoefCount(8);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x81);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();
}

AiUsb20x::~AiUsb20x()
{
}

void AiUsb20x::initialize()
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

double AiUsb20x::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	// call this function only to validate the range
	mapRangeCode(range);

	daqDev().queryCmd(CMD_AIN, channel, 0, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb20x::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	int epAddr = getScanEndpointAddr();
	TAINSCAN_CFG scanCfg = {0};

	setTransferMode(options, rate);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	daqDev().clearHalt(epAddr);

	daqDev().sendCmd(CMD_AINSCAN_CLEAR_FIFO);

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

AiUsb20x::TAINSCAN_CFG AiUsb20x::scanConfig(int lowChan, int highChan, int scanCount, double rate, ScanOption options)
{
	TAINSCAN_CFG scanCfg;
	memset(&scanCfg, 0, sizeof(scanCfg));

	int chanCount = queueEnabled() ? queueLength() : highChan - lowChan  + 1;

	scanCfg.chan_mask = getChannelMask(lowChan, highChan);

	scanCfg.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(chanCount, rate, options));

	scanCfg.options = getOptionsCode(options);

	scanCfg.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		scanCfg.scan_count = 0;

	if(options & SO_EXTTRIGGER)
	{
		scanCfg.trig = 1;
		scanCfg.trig_mode = getTrigModeCode();
	}

	return scanCfg;
}

unsigned char AiUsb20x::getChannelMask(int lowChan, int highChan) const
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

unsigned int AiUsb20x::calcPacerPeriod(int chanCount, double rate, ScanOption options)
{
	unsigned int period = 0;

	if(!(options & SO_EXTCLOCK))
	{
		double clockFreq = mDaqDevice.getClockFreq();

		double periodDbl = clockFreq / (rate * chanCount);

		if (periodDbl > 0)
			--periodDbl;

		double actualRate = clockFreq / (1.0 + (periodDbl * chanCount));

		while(actualRate > mAiInfo.getMaxScanRate())
		{
			period++;
			actualRate = clockFreq / (1.0 + (periodDbl * chanCount));
		}

		if(periodDbl > UINT_MAX)
			periodDbl = UINT_MAX;

		period = periodDbl;

		actualRate = clockFreq / (1ULL + (period * chanCount));

		setActualScanRate(actualRate);
	}
	else
	{
		period = 0;
		setActualScanRate(rate);
	}

	return period;
}

unsigned char AiUsb20x::getOptionsCode(ScanOption options) const
{
	unsigned char optCode = (getTransferMode() & SO_BLOCKIO) ? 0 : 1;

	optCode |= 1 << 7; // inhibit stall

	return optCode;
}

int AiUsb20x::mapRangeCode(Range range) const
{
	int rangeCode;

	switch(range)
	{
	case BIP10VOLTS:
		rangeCode = 0;
		break;
	default:
		throw UlException(ERR_BAD_RANGE);
	}

	return rangeCode;
}

int AiUsb20x::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	int calCoefIndex = channel;

	return calCoefIndex;
}

unsigned char AiUsb20x::getTrigModeCode() const
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

void AiUsb20x::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
}

void AiUsb20x::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 0);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	// Add queue types
	mAiInfo.setQueueTypes(CHAN_QUEUE);

	// Add queue limitations
	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN | ASCENDING_CHAN);
}

} /* namespace ul */
