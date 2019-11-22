/*
 * AiUsb1208fsPlus.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsb1208fsPlus.h"

namespace ul
{
AiUsb1208fs_Plus::AiUsb1208fs_Plus(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO |SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 4);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1408FS_PLUS)
	{
		mAiInfo.setResolution(14);
		mAiInfo.setMaxScanRate(48000);
		mAiInfo.setMaxThroughput(48000);
	}
	else
	{
		mAiInfo.setResolution(12);
		mAiInfo.setMaxScanRate(52000);
		mAiInfo.setMaxThroughput(52000);
	}


	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalCoefsStartAddr(0);
	mAiInfo.setCalDateAddr(0x200);
	mAiInfo.setCalCoefCount(40);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x81);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();
}

AiUsb1208fs_Plus::~AiUsb1208fs_Plus()
{
}

void AiUsb1208fs_Plus::initialize()
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

double AiUsb1208fs_Plus::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	unsigned short rangeCode = mapRangeCode(inputMode, range);

	unsigned short  mode = inputMode == AI_SINGLE_ENDED ? 0 : 1;
	mode = mode << 8;

	daqDev().queryCmd(CMD_AIN, channel | mode, rangeCode, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb1208fs_Plus::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
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

	daqDev().clearFifo(epAddr);

	aInConfig(lowChan, highChan, inputMode, range);

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	scanCfg = scanConfig(lowChan, highChan, inputMode, samplesPerChan, rate, options);

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

void AiUsb1208fs_Plus::aInConfig(int lowChan, int highChan, AiInputMode inputMode, Range range)
{
	int chan;

	unsigned char gainArray[8];
	memset(gainArray, 0, 8);


	if (!queueEnabled())
	{
		for (chan = lowChan; chan <= highChan; chan++)
			gainArray[chan] = mapRangeCode(inputMode, range);
	}
	else
	{
		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			chan = mAQueue.at(idx).channel;
			gainArray[chan] = mapRangeCode(mAQueue.at(idx).inputMode, mAQueue.at(idx).range);
		}
	}

	daqDev().sendCmd(CMD_AINSCAN_CONFIG, 0, 0, gainArray, sizeof(gainArray), 1000);
}

AiUsb1208fs_Plus::TAINSCAN_CFG AiUsb1208fs_Plus::scanConfig(int lowChan, int highChan, AiInputMode inputMode, unsigned int scanCount, double rate, ScanOption options)
{
	TAINSCAN_CFG scanCfg = {0, 0, 0 ,0 ,0} ;

	int chanCount = queueEnabled() ? queueLength() : highChan - lowChan  + 1;

	scanCfg.chan_mask = getChannelMask(lowChan, highChan);

	scanCfg.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(chanCount, rate, options));

	scanCfg.options = getOptionsCode(inputMode, options);

	scanCfg.scan_count = Endian::cpu_to_le_ui32(scanCount * chanCount);

	if(options & SO_CONTINUOUS)
		scanCfg.scan_count = 0;

	if(options & SO_RETRIGGER)
	{
		if(mTrigCfg.retrigCount == 0)
			scanCfg.retrig_count = scanCount * chanCount;
		else
		{
			if(!(options & SO_CONTINUOUS))
				scanCfg.retrig_count = mTrigCfg.retrigCount > scanCount ? scanCount * chanCount : mTrigCfg.retrigCount * chanCount;
			else
			{
				// different behavior from the UL for windows in continuous mode
				scanCfg.retrig_count = mTrigCfg.retrigCount * chanCount;
			}
		}
	}

	return scanCfg;
}

unsigned int AiUsb1208fs_Plus::calcPacerPeriod(int chanCount, double rate, ScanOption options)
{
	unsigned int period = 0;

	if(!(options & SO_EXTCLOCK))
	{
		double clockFreq = mDaqDevice.getClockFreq();

		double minADCRate = clockFreq / 0x100000000;

		if(rate < minADCRate)
			rate = minADCRate;

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

unsigned char AiUsb1208fs_Plus::getChannelMask(int lowChan, int highChan) const
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

unsigned char AiUsb1208fs_Plus::getOptionsCode(AiInputMode inputMode, ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
	  struct
	  {
		 unsigned char xferMode     : 1;
		 unsigned char inputMode    : 1;
		 unsigned char trigModeCode	: 3;
		 unsigned char retrig	    : 1;
		 unsigned char reserved     : 2;
	  };
	  unsigned char code;
	} option;
#pragma pack()

	option.code = 0;

	option.xferMode = (getTransferMode() & SO_BLOCKIO) ? 0 : 1;

	if(!queueEnabled())
		option.inputMode = inputMode == AI_SINGLE_ENDED ? 0 : 1;
	else
		option.inputMode = mAQueue[0].inputMode == AI_SINGLE_ENDED ? 0 : 1;

	if((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		option.trigModeCode = getTrigModeCode();

		if(options & SO_RETRIGGER)
			option.retrig = 1;

	}

	return option.code;
}

int AiUsb1208fs_Plus::mapRangeCode(AiInputMode inputMode, Range range) const
{
	int rangeCode;

	if (inputMode == AI_SINGLE_ENDED)
	{
		if (range ==BIP10VOLTS)
			rangeCode = 0;
		else
			throw UlException(ERR_BAD_RANGE);
	}
	else
	{
		switch(range)
		{
		case BIP20VOLTS:
			rangeCode = 0;
			break;
		case BIP10VOLTS:
			rangeCode = 1;
			break;
		case BIP5VOLTS:
			rangeCode = 2;
			break;
		case BIP4VOLTS:
			rangeCode = 3;
			break;
		case BIP2PT5VOLTS:
			rangeCode = 4;
			break;
		case BIP2VOLTS:
			rangeCode = 5;
			break;
		case BIP1PT25VOLTS:
			rangeCode = 6;
			break;
		case BIP1VOLTS:
			rangeCode = 7;
			break;

		default:
			throw UlException(ERR_BAD_RANGE);
		}
	}

	return rangeCode;
}

int AiUsb1208fs_Plus::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	int gainIndex = 0;

	if (inputMode == AI_SINGLE_ENDED)
	{
		if (range ==BIP10VOLTS)
			gainIndex = 8;
		else
			throw UlException(ERR_BAD_RANGE);
	}
	else
	{
		switch(range)
		{
		case BIP20VOLTS:
			gainIndex = 0;
			break;
		case BIP10VOLTS:
			gainIndex = 1;
			break;
		case BIP5VOLTS:
			gainIndex = 2;
			break;
		case BIP4VOLTS:
			gainIndex = 3;
			break;
		case BIP2PT5VOLTS:
			gainIndex = 4;
			break;
		case BIP2VOLTS:
			gainIndex = 5;
			break;
		case BIP1PT25VOLTS:
			gainIndex = 6;
			break;
		case BIP1VOLTS:
			gainIndex = 7;
			break;

		default:
			throw UlException(ERR_BAD_RANGE);
		}
	}

	int calCoefIdx = (mAiInfo.getNumChans() / 2) * gainIndex + channel;

	return calCoefIdx;
}

unsigned char AiUsb1208fs_Plus::getTrigModeCode() const
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

void AiUsb1208fs_Plus::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);

	mAiInfo.addRange(AI_DIFFERENTIAL, BIP20VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP4VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP2PT5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP2VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP1PT25VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP1VOLTS);
}

void AiUsb1208fs_Plus::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 4);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	// Add queue types
	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);

	// Add queue limitations
	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN | ASCENDING_CHAN);
}

} /* namespace ul */
