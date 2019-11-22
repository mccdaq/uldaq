/*
 * AiUsb2020.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AiUsb2020.h"

namespace ul
{
AiUsb2020::AiUsb2020(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = 1000;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_BURSTIO | SO_RETRIGGER | SO_PACEROUT);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | GATE_HIGH | GATE_LOW |
			TRIG_RISING | TRIG_FALLING | TRIG_ABOVE | TRIG_BELOW | GATE_ABOVE | GATE_BELOW | GATE_IN_WINDOW | GATE_OUT_WINDOW);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(2);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 2);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 1, AI_VOLTAGE);
	mAiInfo.setResolution(12);
	mAiInfo.setMinScanRate(minRate);

	mAiInfo.setMaxScanRate(8000000);
	mAiInfo.setMaxThroughput(8000000);

	mAiInfo.setMaxBurstRate(20000000);
	mAiInfo.setMaxBurstThroughput(40000000);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);

	mAiInfo.setCalCoefsStartAddr(0x7000);
	mAiInfo.setCalDateAddr(0x7098);
	mAiInfo.setCalCoefCount(8);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x86);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

AiUsb2020::~AiUsb2020()
{
}

void AiUsb2020::initialize()
{
	try
	{
		sendStopCmd();

		loadAdcCoefficients();

		loadAInConfigs(BIP10VOLTS, 0, mAiInfo.getNumChansByMode(AI_SINGLE_ENDED) - 1, false);

		// do one ain because if this is the first time after the FPGA is loaded the first ain sample is garbage.
		aIn(0, AI_DIFFERENTIAL, BIP10VOLTS, AIN_FF_DEFAULT);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double AiUsb2020::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex); // call before check_AIn_Args() so it can detect ALREADY_ACTIVE error

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	loadAInConfigs(range, channel, channel, false);

	daqDev().queryCmd(CMD_AIN, channel, 0, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb2020::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex); // call before check_AInScan_Args() so it can detect ALREADY_ACTIVE error

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	setupTrigger(lowChan, highChan, range, options);

	loadAInConfigs(range, lowChan, highChan, queueEnabled());

	daqDev().clearHalt(epAddr);

	daqDev().sendCmd(CMD_AINSCAN_CLEAR_FIFO);

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	setScanConfig(chanCount, samplesPerChan, rate, options);

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

void AiUsb2020::setupTrigger(int lowChan, int highChan, Range range, ScanOption options) const
{
#pragma pack(1)
	struct
	{
		struct
		{
			unsigned char type		: 1;
			unsigned char source	: 1; // analog / digital
			unsigned char mode      : 2;
			unsigned char polarity  : 1; // 0 = low / falling, 1 = high / rising
			unsigned char resv		: 3;
		}
		options;
		unsigned char triggerChannel;
		unsigned short lowThreshold;
		unsigned short highThreshold;
	} cfg;
#pragma pack()

	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTrigConfig();

		memset(&cfg, 0, sizeof(cfg));

		bool analogTrig = false;
		double highThreshold = 0;
		double lowThreshold = 0;

		switch(trigCfg.type)
		{
		// digital triggers
		case TRIG_HIGH:
			cfg.options.type = TRIGGER;
			cfg.options.source = DIGITAL_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = HIGH;
			break;
		case TRIG_LOW:
			cfg.options.type = TRIGGER;
			cfg.options.source = DIGITAL_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = LOW;
			break;
		case TRIG_POS_EDGE:
			cfg.options.type = TRIGGER;
			cfg.options.source = DIGITAL_TRIG;
			cfg.options.mode = EDGE;
			cfg.options.polarity = POS_EDGE;
			break;
		case TRIG_NEG_EDGE:
			cfg.options.type = TRIGGER;
			cfg.options.source = DIGITAL_TRIG;
			cfg.options.mode = EDGE;
			cfg.options.polarity = NEG_EDGE;
			break;
		case GATE_HIGH:
			cfg.options.type = GATE;
			cfg.options.source = DIGITAL_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = HIGH;
			break;
		case GATE_LOW:
			cfg.options.type = GATE;
			cfg.options.source = DIGITAL_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = LOW;
			break;

		// analog triggers
		case TRIG_ABOVE:
			analogTrig = true;
			cfg.options.type = TRIGGER;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = RISING_;
			highThreshold = trigCfg.level;
			break;
		case TRIG_BELOW:
			analogTrig = true;
			cfg.options.type = TRIGGER;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = FALLING_;
			lowThreshold = trigCfg.level;
			break;
		case TRIG_RISING:
			analogTrig = true;
			cfg.options.type = TRIGGER;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = HYSTERESIS;
			cfg.options.polarity = RISING_;
			highThreshold = trigCfg.level;
			lowThreshold = trigCfg.level - trigCfg.variance;
			break;
		case TRIG_FALLING:
			analogTrig = true;
			cfg.options.type = TRIGGER;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = HYSTERESIS;
			cfg.options.polarity = FALLING_;
			lowThreshold = trigCfg.level;
			highThreshold = trigCfg.level + trigCfg.variance;
			break;

		case GATE_ABOVE:
			analogTrig = true;
			cfg.options.type = GATE;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = ABOVE;
			highThreshold = trigCfg.level;
			break;
		case GATE_BELOW:
			analogTrig = true;
			cfg.options.type = GATE;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = LEVEL;
			cfg.options.polarity = BELOW;
			lowThreshold = trigCfg.level;
			break;
		case GATE_IN_WINDOW:
			analogTrig = true;
			cfg.options.type = GATE;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = WINDOW;
			cfg.options.polarity = WINDOW_IN;
			lowThreshold = trigCfg.level - trigCfg.variance;
			highThreshold = trigCfg.level + trigCfg.variance;
			break;
		case GATE_OUT_WINDOW:
			analogTrig = true;
			cfg.options.type = GATE;
			cfg.options.source = ANALOG_TRIG;
			cfg.options.mode = WINDOW;
			cfg.options.polarity = WINDOW_OUT;
			lowThreshold = trigCfg.level - trigCfg.variance;
			highThreshold = trigCfg.level + trigCfg.variance;
			break;
		default:
			break;
		}

		if(analogTrig)
		{
			Range trigChanRange = range;
			int trigChan = trigCfg.trigChan;

			if(queueEnabled())
			{
				for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
				{
					if(trigCfg.trigChan == mAQueue.at(idx).channel)
						trigChanRange = mAQueue.at(idx).range;
				}
			}

			cfg.triggerChannel = trigCfg.trigChan;
			cfg.lowThreshold = getRawThreshold(trigChan, trigChanRange, lowThreshold);
			cfg.highThreshold = getRawThreshold(trigChan, trigChanRange, highThreshold);
		}

		daqDev().sendCmd(CMD_SETTRIG, 0, 0, (unsigned char*) &cfg, sizeof(cfg));
	}
}

unsigned short AiUsb2020::getRawThreshold(int trigChan, Range range, double threshold) const
{
	unsigned short rawThreshold = 0;

	CalCoef calCoef = getCalCoef(trigChan, AI_SINGLE_ENDED, range, NOSCALEDATA);

	double offset = 0;
	double scale = 0;
	mDaqDevice.getEuScaling(range, scale, offset);

	unsigned long long fullScaleCount = 0x1000;
	double lsb = scale / fullScaleCount;

	// convert threshold to count
	double count = threshold;
	count /= lsb;
	count += ((-offset) / scale) *  fullScaleCount;

	// uncalibrate threshold
	double uncalThreshold = count;
	uncalThreshold -= calCoef.offset;
	uncalThreshold /= calCoef.slope;

	if (uncalThreshold > 4095.0f)
		rawThreshold = 0x0fff;
	else if (uncalThreshold < 0.0f)
		rawThreshold = 0;
	else
		rawThreshold = (unsigned short) uncalThreshold;

	return rawThreshold;
}

void AiUsb2020::check_AInScan_Args(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) const
{
	AiDevice::check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	if((options & SO_EXTCLOCK) && (options & SO_PACEROUT))
		throw UlException(ERR_BAD_OPTION);

	if(!(options & SO_EXTCLOCK) && rate < mAiInfo.getMinScanRate())
		throw UlException(ERR_BAD_RATE);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	long long totalCount = (long long) samplesPerChan * chanCount;

	if((options & SO_BURSTIO) && (totalCount < MIN_DRAM_SAMPLE_COUNT || totalCount > MAX_DRAM_SAMPLE_COUNT || (totalCount % BURST_SIZE_DRAM != 0)))
	{
		throw UlException(ERR_BAD_SAMPLE_COUNT);
	}

	if((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTrigConfig();

		// analog triggers
		if((trigCfg.type & (TRIG_RISING | TRIG_FALLING | TRIG_ABOVE | TRIG_BELOW | GATE_ABOVE | GATE_BELOW | GATE_IN_WINDOW | GATE_OUT_WINDOW)))
		{
			if(options & SO_RETRIGGER)
				throw UlException(ERR_BAD_RETRIG_TRIG_TYPE);

			bool validTrigChan = false;

			Range trigChanRange = range;

			if(!queueEnabled())
			{
				for(int chan = lowChan; chan <= highChan; chan++)
				{
					if(trigCfg.trigChan == chan)
					{
						validTrigChan = true;
						break;
					}
				}
			}
			else
			{
				for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
				{
					if(trigCfg.trigChan == mAQueue.at(idx).channel)
					{
						validTrigChan = true;
						trigChanRange = mAQueue.at(idx).range;
						break;
					}
				}
			}

			if(!validTrigChan)
				throw UlException(ERR_BAD_TRIG_CHANNEL);


			// validated trigger threshold

			double offset = 0;
			double scale = 0;
			mDaqDevice.getEuScaling(trigChanRange, scale, offset);

			double minThreshold = offset;
			double maxThreshold = scale + offset;

			if(trigCfg.level > maxThreshold || trigCfg.level < minThreshold)
			{
				throw UlException(ERR_BAD_TRIG_LEVEL);
			}


			if(trigCfg.type & TRIG_RISING)
			{
				if((trigCfg.level - trigCfg.variance) < minThreshold)
				{
					throw UlException(ERR_TRIG_THRESHOLD_OUT_OF_RANGE);
				}
			}
			else if(trigCfg.type & TRIG_FALLING)
			{
				if((trigCfg.level + trigCfg.variance) > maxThreshold)
				{
					throw UlException(ERR_TRIG_THRESHOLD_OUT_OF_RANGE);
				}
			}
			else if((trigCfg.type & GATE_IN_WINDOW) || (trigCfg.type & GATE_OUT_WINDOW))
			{
				if(((trigCfg.level - trigCfg.variance) < minThreshold) || ((trigCfg.level + trigCfg.variance) > maxThreshold))
				{
					throw UlException(ERR_TRIG_THRESHOLD_OUT_OF_RANGE);
				}
			}
		}

	}

}

void AiUsb2020::setScanConfig(int chanCount, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	int epAddr = getScanEndpointAddr();
	mScanConfig.packet_size = (getTransferMode() == SO_BLOCKIO) ? daqDev().getBulkEndpointMaxPacketSize(epAddr) / 2 - 1 : chanCount - 1;

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

unsigned char AiUsb2020::getOptionsCode(ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
	  struct
	  {
		 unsigned char reserved0    : 3;
		 unsigned char exttrigger   : 1;
		 unsigned char reserved1    : 1;
		 unsigned char pacerout     : 1;
		 unsigned char retrigger    : 1;
		 unsigned char use_dram	    : 1;
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
	else if (options & SO_PACEROUT)
	{
		option.pacerout = 1;
	}

	option.use_dram = ((options & SO_BURSTIO) ? 1 : 0);

	return option.code;
}

void AiUsb2020::loadAInConfigs(Range range, int lowChan, int highChan, bool queueEnabled) const
{
#pragma pack(1)
	union TAICONFIG
	{
		struct
		{
			unsigned char chan		 : 1;
			unsigned char range		 : 2;
			unsigned char last_chan  : 1;
			unsigned char enable_cal : 1;
			unsigned char reserved	 : 3;
		};
		unsigned char config;
	};
#pragma pack()

	TAICONFIG aiCfg[2];
	int rangeCode;
	int chan;

	memset(aiCfg, 0, sizeof(aiCfg));

	if(!queueEnabled)
	{
		rangeCode = mapRangeCode(range);

		int idx = 0;

		for(chan = lowChan; chan <= highChan; chan++)
		{
			aiCfg[idx].chan = chan;
			aiCfg[idx].range = rangeCode;

			if(chan  == highChan)
				aiCfg[idx].last_chan = 1;
			else
				aiCfg[idx].last_chan = 0;

			idx++;
		}
	}
	else
	{
		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			chan = mAQueue.at(idx).channel;
			rangeCode = mapRangeCode(mAQueue.at(idx).range);

			aiCfg[idx].chan = chan;
			aiCfg[idx].range = rangeCode;

			if(idx  == mAQueue.size() - 1)
				aiCfg[idx].last_chan = 1;
			else
				aiCfg[idx].last_chan = 0;
		}
	}

	daqDev().sendCmd(CMD_AIN_CONFIG, 0, 0, (unsigned char*)&aiCfg, sizeof(aiCfg));
}

int AiUsb2020::mapRangeCode(Range range) const
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

int AiUsb2020::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	int calCoefIndex;

	switch(range)
	{
	case BIP10VOLTS:
		calCoefIndex = 0;
		break;
	case BIP5VOLTS:
		calCoefIndex = 1;
		break;
	case BIP2VOLTS:
		calCoefIndex = 2;
		break;
	case BIP1VOLTS:
		calCoefIndex = 3;
		break;
	default:
		throw UlException(ERR_BAD_RANGE);
	}

	calCoefIndex += channel * 4;

	return calCoefIndex;
}

void AiUsb2020::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP2VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP1VOLTS);
}

void AiUsb2020::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 2);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);
}


void AiUsb2020::readCalDate()
{
	unsigned char calDateBuf[6];
	int calDateAddr = mAiInfo.getCalDateAddr();

	if(calDateAddr != -1 && getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

		if(bytesReceived == sizeof(calDateBuf))
		{
			if(bytesReceived == sizeof(calDateBuf))
			{
				tm time;
				memset(&time, 0, sizeof(time));

				time.tm_year = calDateBuf[0] + 100;
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
						mFieldCalDate = cal_date_sec;

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
}

} /* namespace ul */
