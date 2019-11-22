/*
 * AiUsb1608hs.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AiUsb1608hs.h"
#include <unistd.h>

namespace ul
{
AiUsb1608hs::AiUsb1608hs(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mAiInfo.setTriggerTypes(TRIG_ABOVE | TRIG_BELOW | TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 8);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setResolution(16);
	mAiInfo.setMinScanRate(minRate);

	mAiInfo.setMaxScanRate(250000);
	mAiInfo.setMaxThroughput(8 * 250000);

	mAiInfo.setMaxBurstRate(0); // disabled for consistency  with the win support. it is supported in the fW (not sure if it works)
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalCoefsStartAddr(0x20);
	mAiInfo.setCalDateAddr(0x132); // no cal date is stored on this device
	mAiInfo.setCalCoefCount(32);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x81);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();

	memset(&mScanConfig, 0, sizeof(mScanConfig));

	for(int chan = 0; chan < mAiInfo.getNumChans(); chan++)
	{
		mAInConfig[chan].modeCode = GROUND_MODE;
		mAInConfig[chan].rangeCode = mapRangeCode(BIP10VOLTS);
	}
}

AiUsb1608hs::~AiUsb1608hs()
{
}

void AiUsb1608hs::initialize()
{
	try
	{
		sendStopCmd();

		loadAdcCoefficients();

		writeAInConfigs();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AiUsb1608hs::disconnect()
{
	IoDevice::disconnect();

	try
	{
		resetAInConfigs();
	}
	catch(UlException& e)
	{
	}
}

double AiUsb1608hs::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal[8];

	int modeCode = getModeCode(inputMode);
	int rangeCode = mapRangeCode(range);

	if(mAInConfig[channel].modeCode != modeCode || mAInConfig[channel].rangeCode != rangeCode)
	{
		loadAInConfigs(inputMode, range, channel, channel, false);

		// coverity[sleep]
		usleep(20000);
	}

	daqDev().queryCmd(CMD_AIN, channel, 0, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal[channel] = Endian::le_ui16_to_cpu(rawVal[channel]);

	data = calibrateData(channel, inputMode, range, rawVal[channel], flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb1608hs::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	if(!(options & SO_CONTINUOUS) && (samplesPerChan > 0x00FFFFFF))
	{
		throw UlException(ERR_BAD_SAMPLE_COUNT);
	}

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	// if the actual extclock rate is greater than 25kHz then in single i/o mode not all samples is transfered to the host therefore
	// in extclock mode regardless of the rate the i/o mode is set to block i/o unless user specifies single i/o mode

	if((options & SO_EXTCLOCK) && !(options & SO_SINGLEIO))
	{
		mTransferMode = SO_BLOCKIO;
	}

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	daqDev().setupTrigger(FT_AI, options);

	unsigned char prevAInConfig[64] = {0};
	memcpy(prevAInConfig, mAInConfig, sizeof(mAInConfig));

	for(int chan = 0; chan < mAiInfo.getNumChans(); chan++)
		mAInConfig[chan].modeCode = GROUND_MODE;

	loadAInConfigs(inputMode, range, lowChan, highChan, queueEnabled());

	if(memcmp(prevAInConfig, mAInConfig, sizeof(mAInConfig)))
	{
		// coverity[sleep]
		usleep(20000);
	}

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	setScanConfig(lowChan, highChan, samplesPerChan, rate, options);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_AINSCAN_START, 0, 0, NULL, 0, 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

void AiUsb1608hs::writeAInConfigs() const
{
	daqDev().sendCmd(CMD_AIN_CONFIG, 0, 0, (unsigned char*)&mAInConfig, sizeof(mAInConfig));
}

void AiUsb1608hs::resetAInConfigs() const
{
	for(int chan = 0; chan < mAiInfo.getNumChans(); chan++)
		mAInConfig[chan].modeCode = GROUND_MODE;

	writeAInConfigs();
}

void AiUsb1608hs::loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const
{
	int chan;

	if(!queueEnabled)
	{
		for(chan = lowChan; chan <= highChan; chan++)
		{
			mAInConfig[chan].modeCode = getModeCode(inputMode);
			mAInConfig[chan].rangeCode = mapRangeCode(range);
		}
	}
	else
	{
		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			chan = mAQueue.at(idx).channel;

			mAInConfig[chan].modeCode = getModeCode(mAQueue.at(idx).inputMode);
			mAInConfig[chan].rangeCode = mapRangeCode(mAQueue.at(idx).range);
		}
	}

	writeAInConfigs();
}



void AiUsb1608hs::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options)
{
	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;

	memset(&mScanConfig, 0, sizeof(mScanConfig)); // ground unused channels

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(chanCount, rate, options));
	mScanConfig.options = getOptionsCode(options);

	mScanConfig.low_chan = queueEnabled() ? mAQueue.at(0).channel : lowChan;
	mScanConfig.chan_count = chanCount - 1;


	unsigned int scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		scan_count = 0;
	else
	{
		// workaround  for an FW issue
		// if in finite mode, if number total samples is multiple of 512 then fw does not send a zero size packet to terminate the scan.
		// as a workaround if the above criteria is met we add one extra sample to the scan count to avoid the issue. (the extra sample won't be
		// copied to the user buffer.

		int epAddr = getScanEndpointAddr();
		int maxPacketSize = daqDev().getBulkEndpointMaxPacketSize(epAddr);

		if(((chanCount * scan_count * mAiInfo.getSampleSize()) % maxPacketSize) == 0)
		{
			scan_count++;
		}

	}

	if(options & SO_RETRIGGER)
	{
		if(mTrigCfg.retrigCount == 0)
			scan_count = scanCount;
		else
		{
			// different behavior from the UL for windows in continuous mode
			if(!(options & SO_CONTINUOUS))
				scan_count = mTrigCfg.retrigCount > scanCount ? scanCount : mTrigCfg.retrigCount;
			else
			{
				// different behavior from the UL for windows in continuous mode
				scan_count = mTrigCfg.retrigCount;
			}
		}
	}

	memcpy(mScanConfig.scan_count, &scan_count, sizeof(mScanConfig.scan_count));

	daqDev().sendCmd(CMD_AINSCAN_CONFIG, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);
}

unsigned int AiUsb1608hs::calcPacerPeriod(int chanCount, double rate, ScanOption options)
{
	unsigned int period = 0;
	if(!(options & SO_EXTCLOCK))
	{
		double clockFreq = mDaqDevice.getClockFreq();
		double periodDbl = clockFreq / rate;

		if (periodDbl > 0)
			--periodDbl;

		if(periodDbl > UINT_MAX)
			periodDbl = UINT_MAX;

		period = periodDbl;

		double actualrate = clockFreq / (1ULL + period);

		setActualScanRate(actualrate);
	}
	else
	{
		period = 0;
		setActualScanRate(rate);
	}

	return period;
}

unsigned char AiUsb1608hs::getOptionsCode(ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
	  struct
	  {
		 unsigned char finiteMode   : 1;
		 unsigned char burstMode    : 1;
		 unsigned char xferMode     : 1;
		 unsigned char exttrigger   : 1;
		 unsigned char extclock     : 1;
		 unsigned char debugMode    : 1;
		 unsigned char retrigger    : 1;
		 unsigned char din   		: 1;
	  };
	  unsigned char code;
	} option;
#pragma pack()

	option.code = 0;

	option.finiteMode =  ((options & SO_CONTINUOUS) ? 0 : 1);
	option.extclock =  ((options & SO_EXTCLOCK) ? 1 : 10);

	if (options & SO_RETRIGGER)
	{
		option.retrigger = 1;
		option.exttrigger = 1;
		option.finiteMode = 0;
	}
	else if (options & SO_EXTTRIGGER)
	{
		option.retrigger = 0;
		option.exttrigger = 1;
	}

	option.burstMode = ((options & SO_BURSTMODE) ? 1 : 0);
	option.xferMode = (getTransferMode() & SO_BLOCKIO) ? 0 : 1;

	return option.code;
}

int AiUsb1608hs::mapRangeCode(Range range) const
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
		rangeCode = 2;
		break;
	case BIP1VOLTS:
		rangeCode = 3;
		break;
	default:
		throw UlException(ERR_BAD_RANGE);
	}

	return rangeCode;
}

int AiUsb1608hs::getModeCode(AiInputMode inputMode) const
{
	int mode;

	if(inputMode == AI_SINGLE_ENDED)
	{
		mode = SE_MODE;
	}
	else
	{
		mode = DIFF_MODE;
	}

	return mode;
}

int AiUsb1608hs::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	union
	{
		struct
		{
			unsigned short range  : 2;
			unsigned short chan   : 3;
			unsigned short zero   : 11;
		};
		unsigned short   index;
	} aiCalCfg;

	aiCalCfg.zero = 0;
	aiCalCfg.chan = channel;

	aiCalCfg.range = mapRangeCode(range);

	return aiCalCfg.index;
}

UlError AiUsb1608hs::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned char cmd = daqDev().getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned char status =0;

	try
	{
		daqDev().queryCmd(cmd, 0, 0, (unsigned char*)&status, sizeof(status));

		if(status & daqDev().getOverrunBitMask())
		{
			err = ERR_OVERRUN;
		}
	}
	catch(UlException& e)
	{
		err = e.getError();
	}
	catch(...)
	{
		err = ERR_UNHANDLED_EXCEPTION;
	}

	return err;
}

void AiUsb1608hs::readCalDate()
{
	unsigned char calDateBuf[6];
	int calDateAddr = mAiInfo.getCalDateAddr();

	if(calDateAddr != -1 && getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

		if(bytesReceived == sizeof(calDateBuf))
		{
			tm time;
			memset(&time, 0, sizeof(time));

			time.tm_year = calDateBuf[0];
			time.tm_mon = calDateBuf[1] - 1;
			time.tm_mday = calDateBuf[2];
			time.tm_hour = calDateBuf[3];
			time.tm_min = calDateBuf[4];
			time.tm_sec = calDateBuf[5];
			time.tm_isdst = -1;

			// make sure the date is valid, mktime does not validate the range
			if(time.tm_mon <= 11 && time.tm_mday <= 31 && time.tm_hour <= 23 && time.tm_min <= 59 && time.tm_sec <= 60)
			{
				time_t cal_date_sec = mktime(&time); // seconds since unix epoch

				if(cal_date_sec > 0) // mktime returns  -1 if cal date is invalid
					mCalDate = cal_date_sec;

				// convert seconds to string

				/*struct tm *timeinfo;
				timeinfo = localtime(&cal_date_sec);
				char b[100];
				strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
				std::cout << b << std::endl;*/
			}
		}
	}
}

void AiUsb1608hs::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP2VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP1VOLTS);

	mAiInfo.addRange(AI_DIFFERENTIAL, BIP10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP2VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP1VOLTS);
}

void AiUsb1608hs::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 8);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	mAiInfo.setQueueTypes(GAIN_QUEUE | MODE_QUEUE);

	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN | ASCENDING_CHAN);
}

} /* namespace ul */
