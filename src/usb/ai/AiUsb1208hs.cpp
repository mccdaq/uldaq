/*
 * AiUsb1208hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsb1208hs.h"

namespace ul
{
AiUsb1208hs::AiUsb1208hs(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_BURSTMODE | SO_RETRIGGER);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 4);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setResolution(13);
	mAiInfo.setMinScanRate(minRate);

	mAiInfo.setMaxScanRate(1000000);
	mAiInfo.setMaxThroughput(1000000);

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalCoefsStartAddr(0x4000);
	mAiInfo.setCalDateAddr(-1); // no cal date is stored on this device
	mAiInfo.setCalCoefCount(16);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x86);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();

	memset(&mAInConfig, 0, sizeof(mAInConfig));
	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

AiUsb1208hs::~AiUsb1208hs()
{
}

void AiUsb1208hs::initialize()
{
	try
	{
		sendStopCmd();

		loadAdcCoefficients();

		loadAInConfigs(AI_DIFFERENTIAL, BIP10VOLTS, 0, mAiInfo.getNumChansByMode(AI_DIFFERENTIAL) - 1, false);

		// do one ain because if this is the first time after the FPGA is loaded the first ain sample is garbage.
		aIn(0, AI_DIFFERENTIAL, BIP10VOLTS, AIN_FF_DEFAULT);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double AiUsb1208hs::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	int adcChan = getAdcChanNum(channel, inputMode);
	int modeCode = getModeCode(inputMode);
	int rangeCode = mapRangeCode(inputMode, range);

	if(mAInConfig.modeCode != modeCode || mAInConfig.rangeCode[adcChan] != rangeCode)
		loadAInConfigs(inputMode, range, channel, channel, false);

	daqDev().queryCmd(CMD_AIN, channel, 0, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb1208hs::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	daqDev().setupTrigger(FT_AI, options);

	loadAInConfigs(inputMode, range, lowChan, highChan, queueEnabled());

	daqDev().clearHalt(epAddr);

	daqDev().sendCmd(CMD_AINSCAN_CLEAR_FIFO);

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	setScanConfig(lowChan, highChan, samplesPerChan, rate, options);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_AINSCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

void AiUsb1208hs::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options)
{
	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;

	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(chanCount, rate, options));
	mScanConfig.options = getOptionsCode(options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	unsigned char chanMask = 0x00;

	if(!queueEnabled())
	{
		for(int chan = lowChan; chan <= highChan; chan++)
		{
			chanMask = 0x01 << chan;
			mScanConfig.channels |= chanMask;
		}
	}
	else
	{
		for(int idx = 0; idx < queueLength() ; idx++)
		{
			chanMask  =  0x01 << mAQueue.at(idx).channel;
			mScanConfig.channels |= chanMask;
		}
	}

	int epAddr = getScanEndpointAddr();
	mScanConfig.packet_size = (getTransferMode() == SO_BLOCKIO) ? daqDev().getBulkEndpointMaxPacketSize(epAddr)/2 - 1 : chanCount - 1;

	if(options & SO_RETRIGGER)
	{
		if(mTrigCfg.retrigCount == 0)
			mScanConfig.retrig_count = scanCount;
		else
		{
			if(!(options & SO_CONTINUOUS))
				mScanConfig.retrig_count = mTrigCfg.retrigCount > scanCount ? scanCount : mTrigCfg.retrigCount;
			else
			{
				// different behavior from the UL for windows in continuous mode
				mScanConfig.retrig_count = mTrigCfg.retrigCount;
			}
		}
	}
}

unsigned int AiUsb1208hs::calcPacerPeriod(int chanCount, double rate, ScanOption options)
{
	unsigned int period = 0;
	if(!(options & SO_EXTCLOCK))
	{
		double aggRate = rate * chanCount;

		if(options & SO_BURSTMODE)
			aggRate = rate;

		double clockFreq = mDaqDevice.getClockFreq();
		double periodDbl = clockFreq / aggRate;

		if (periodDbl > 0)
			--periodDbl;

		if(periodDbl > UINT_MAX)
			periodDbl = UINT_MAX;

		period = periodDbl;

		double actualrate = clockFreq / (1ULL + period);

		if(!(options & SO_BURSTMODE))
			actualrate = actualrate / chanCount;

		setActualScanRate(actualrate);
	}
	else
	{
		period = 0;
		setActualScanRate(rate);
	}

	return period;
}

unsigned char AiUsb1208hs::getOptionsCode(ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
	  struct
	  {
		 unsigned char burstMode    : 1;
		 unsigned char reserved0    : 1;
		 unsigned char reserved1    : 1;
		 unsigned char exttrigger   : 1;
		 unsigned char reserved2    : 1;
		 unsigned char reserved3    : 1;
		 unsigned char retrigger    : 1;
		 unsigned char reserved4	: 1;
	  };
	  unsigned char code;
	} option;
#pragma pack()

	option.code = 0;

	if (options & SO_RETRIGGER)
	{
		option.retrigger = 1;
		option.exttrigger = 1;
	}
	else if (options & SO_EXTTRIGGER)
	{
		option.retrigger = 0;
		option.exttrigger = 1;
	}

	option.burstMode = ((options & SO_BURSTMODE) ? 1 : 0);

	return option.code;
}

void AiUsb1208hs::loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const
{
	int rangeCode;
	int chan;
	int adcChan;

	// commented out, becasue if this function is called from AIn the following line changes the range for all channels
	//memset(&mAInConfig, 0, sizeof(mAInConfig));

	if(!queueEnabled)
	{
		mAInConfig.modeCode = getModeCode(inputMode);
		rangeCode = mapRangeCode(inputMode, range);

		for(chan = lowChan; chan <= highChan; chan++)
		{
			adcChan = getAdcChanNum(chan, inputMode);
			mAInConfig.rangeCode[adcChan] = rangeCode;
		}
	}
	else
	{
		mAInConfig.modeCode = getModeCode(mAQueue.at(0).inputMode);

		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			chan = mAQueue.at(idx).channel;
			rangeCode = mapRangeCode(mAQueue.at(idx).inputMode, mAQueue.at(idx).range);

			adcChan = getAdcChanNum(chan, mAQueue.at(idx).inputMode);

			mAInConfig.rangeCode[adcChan] = rangeCode;
		}
	}

	daqDev().sendCmd(CMD_AIN_CONFIG, 0, 0, (unsigned char*)&mAInConfig, sizeof(mAInConfig));
}

int AiUsb1208hs::getAdcChanNum(int chan, AiInputMode inputMode) const
{
	int adcChan  = 0;

	if(inputMode == AI_SINGLE_ENDED)
		adcChan = chan;
	else
		adcChan = chan * 2;  // gain must be set for chan 0, 2,4 ,6 in diff mode

	return adcChan;
}

int AiUsb1208hs::mapRangeCode(AiInputMode inputMode, Range range) const
{
	int rangeCode;

	if(inputMode == AI_SINGLE_ENDED)
	{
		switch(range)
		{
		case BIP10VOLTS:
			rangeCode = 0;
			break;
		case BIP5VOLTS:
			rangeCode = 1;
			break;
		case BIP2PT5VOLTS:
			rangeCode = 2;
			break;
		case UNI10VOLTS:
			rangeCode = 3;
			break;
		default:
			throw UlException(ERR_BAD_RANGE);
		}
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
		default:
			throw UlException(ERR_BAD_RANGE);
		}
	}

	return rangeCode;
}

int AiUsb1208hs::getModeCode(AiInputMode inputMode) const
{
	int mode;

	if(inputMode == AI_SINGLE_ENDED)
	{
		mode = 0;
	}
	else
	{
		mode = 2;
	}

	return mode;
}

int AiUsb1208hs::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	union
	{
		struct
		{
			unsigned short range  : 2;
			unsigned short mode   : 2;
			unsigned short zero   : 12;
		};
		unsigned short   index;
	} aiCalCfg;


	aiCalCfg.zero = 0;
	aiCalCfg.mode = getModeCode(inputMode);
	aiCalCfg.range = mapRangeCode(inputMode, range);

	return aiCalCfg.index;
}

void AiUsb1208hs::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP2PT5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, UNI10VOLTS);

	mAiInfo.addRange(AI_DIFFERENTIAL, BIP20VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP5VOLTS);
}

void AiUsb1208hs::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 4);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);

	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN | ASCENDING_CHAN);
}

} /* namespace ul */
