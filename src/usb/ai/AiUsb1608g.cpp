/*
 * AiUsb1608g.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsb1608g.h"

/*#define CAL_REF_COUNT 		3
#define MIN_SLOPE 			0.9f
#define MAX_SLOPE 			1.2f
#define MIN_OFFSET			-10000
#define MAX_OFFSET			10000
#define VREFS_ADDR			0x7040*/

namespace ul
{

AiUsb1608g::AiUsb1608g(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_BURSTMODE | SO_RETRIGGER);
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(16);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 8);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 16);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 15, AI_VOLTAGE);
	mAiInfo.setResolution(16);
	mAiInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1608G || daqDev().getDeviceType() == DaqDeviceId::USB_1608G_2)
	{
		mAiInfo.setMaxScanRate(250000);
		mAiInfo.setMaxThroughput(250000);
	}
	else
	{
		mAiInfo.setMaxScanRate(500000);
		mAiInfo.setMaxThroughput(500000);
	}

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalCoefsStartAddr(0x7000);
	mAiInfo.setCalDateAddr(0x7098);
	mAiInfo.setCalCoefCount(4);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x86);

	setScanStopCmd(CMD_AINSTOP);

	initCustomScales();

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

AiUsb1608g::~AiUsb1608g()
{
}

void AiUsb1608g::initialize()
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

double AiUsb1608g::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	loadAInConfigs(inputMode, range, channel, channel, false);

	daqDev().queryCmd(CMD_AIN, channel, 0, (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb1608g::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
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

void AiUsb1608g::setScanConfig(int chanCount, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

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

unsigned char AiUsb1608g::getOptionsCode(ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
	  struct
	  {
		 unsigned char burstio      : 1;
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

	option.burstio = ((options & SO_BURSTMODE) ? 1 : 0);

	return option.code;
}

void AiUsb1608g::loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const
{
#pragma pack(1)
	union TAICONFIG
	{
		struct
		{
			unsigned char chan		 : 3;
			unsigned char range		 : 2;
			unsigned char mode		 : 2;
			unsigned char last_chan  : 1;
		};
		unsigned char config;
	};
#pragma pack()

	TAICONFIG aiCfg[16];
	int rangeCode;
	int chan;

	memset(aiCfg, 0, sizeof(aiCfg));

	if(!queueEnabled)
	{
		rangeCode = mapRangeCode(range);

		int idx = 0;

		for(chan = lowChan; chan <= highChan; chan++)
		{
			aiCfg[idx].chan = chan % 8;
			aiCfg[idx].mode = getModeCode(chan, inputMode);
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

			aiCfg[idx].chan = chan % 8;
			aiCfg[idx].mode = getModeCode(chan, mAQueue.at(idx).inputMode);
			aiCfg[idx].range = rangeCode;

			if(idx  == mAQueue.size() - 1)
				aiCfg[idx].last_chan = 1;
			else
				aiCfg[idx].last_chan = 0;
		}
	}

	daqDev().sendCmd(CMD_AIN_CONFIG, 0, 0, (unsigned char*)&aiCfg, sizeof(aiCfg));
}

int AiUsb1608g::mapRangeCode(Range range) const
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

int AiUsb1608g::getModeCode(int chan, AiInputMode inputMode) const
{
	int mode;

	if(calModeEnabled())
	{
		mode = 3;
	}
	else
	{
		if(inputMode == AI_SINGLE_ENDED)
		{
			if(chan < 8)
				mode = 1;
			else
				mode = 2;
		}
		else
			mode = 0;
	}

	return mode;
}

int AiUsb1608g::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
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

	return calCoefIndex;
}

void AiUsb1608g::addSupportedRanges()
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

void AiUsb1608g::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 16);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 16);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);
}




void AiUsb1608g::readCalDate()
{
	unsigned short calDateBuf[6];
	int calDateAddr = mAiInfo.getCalDateAddr();

	if(calDateAddr != -1 && getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

		if(bytesReceived == sizeof(calDateBuf))
		{
			tm time;
			memset(&time, 0, sizeof(time));

			// the date is stored on the device in big-endian format by the test department
			time.tm_year =  mEndian.be_ui16_to_cpu(calDateBuf[0]) - 1900;
			time.tm_mon = mEndian.be_ui16_to_cpu(calDateBuf[1]) - 1;
			time.tm_mday = mEndian.be_ui16_to_cpu(calDateBuf[2]);
			time.tm_hour = mEndian.be_ui16_to_cpu(calDateBuf[3]);
			time.tm_min = mEndian.be_ui16_to_cpu(calDateBuf[4]);
			time.tm_sec = mEndian.be_ui16_to_cpu(calDateBuf[5]);
			time.tm_isdst = -1;

			time_t cal_date_sec = mktime(&time); // seconds since unix epoch

			if(cal_date_sec > 0) // mktime returns  -1 if cal date is invalid
				mCalDate = cal_date_sec;

			// convert seconds to string
			/*
			struct tm *timeinfo;
			timeinfo = localtime(&cal_date_sec);
			char b[100];
			strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
			std::cout << b << std::endl;*/
		}
	}
}

///////////////////////  calibration ////////////////////////////

/*void AiUsb1608g::reloadCalCoefs() const
{
	const_cast<AiUsb1608g*>(this)->loadAdcCoefficients();
}

void AiUsb1608g::calibrate() const
{
	if(daqDev().isScanRunning())
		throw UlException(ERR_ALREADY_ACTIVE);

	try
	{
		calAdc(BIP10VOLTS);
		calAdc(BIP5VOLTS);
		calAdc(BIP2VOLTS);
		calAdc(BIP1VOLTS);

		storeCalDate();
	}
	catch(UlException& e)
	{
		const_cast<AiUsb1608g*>(this)->enableCalMode(false);

		throw UlException(e.getError());
	}
}

void AiUsb1608g::calAdc(Range range) const
{
	UlError err = ERR_NO_ERROR;
	const int sampleCount = 1000;
	double dataBuffer[sampleCount];
	double rate = 250000;
	AInScanFlag flags = AINSCAN_FF_NOSCALEDATA;
	ScanOption options = SO_DEFAULTIO;
	unsigned int calIdx[CAL_REF_COUNT];
	double s_desired[CAL_REF_COUNT];
	double s_avgerage[CAL_REF_COUNT];

	double slope=1.0, offset=0.0;

	const_cast<AiUsb1608g*>(this)->enableCalMode(true);

	getCalOutputIndexs(range, calIdx);
	getVRefs(range, s_desired);

	for(int i = 0; i < CAL_REF_COUNT; i++)
	{
		daqDev().setCalOutput(calIdx[i]);

		usleep(500000);

		check_AInScan_Args(0, 0, AI_SINGLE_ENDED, range, sampleCount, rate, options, flags, dataBuffer);

		const_cast<AiUsb1608g*>(this)->aInScan(0, 0, AI_SINGLE_ENDED, range, sampleCount, rate, options, flags, dataBuffer);

		while (getScanState() == SS_RUNNING)
		{
			usleep(10000);
		}

		const_cast<AiUsb1608g*>(this)->stopBackground();

		err = daqDev().scanTranserIn()->getXferError();

		if(err != ERR_NO_ERROR)
			throw UlException(err);

		s_avgerage[i] = 0;
		for(int j = 0; j < sampleCount; j++)
			s_avgerage[i] += dataBuffer[j];

		s_avgerage[i] /= sampleCount;
	}


	double s_desired_sum = 0;
	double s_avgerage_sum = 0;
	double s_avgerage_sqr = 0;

	for(int i = 0; i < CAL_REF_COUNT; i++)
	{
		s_desired_sum += s_desired[i];
		s_avgerage_sum += s_avgerage[i];
		s_avgerage_sqr +=  s_avgerage[i] * s_avgerage[i];

		slope += s_avgerage[i] * s_desired[i];
	}

	slope = CAL_REF_COUNT * slope - s_avgerage_sum * s_desired_sum;
	slope /= (CAL_REF_COUNT * s_avgerage_sqr - s_avgerage_sum * s_avgerage_sum);
	offset = (s_desired_sum - slope * s_avgerage_sum) / CAL_REF_COUNT;

	if(slope < MIN_SLOPE)
		err = ERR_MIN_SLOPE_VAL_REACHED;
	else if (slope > MAX_SLOPE)
		err = ERR_MAX_SLOPE_VAL_REACHED;

	if (offset < MIN_OFFSET)
		err = ERR_MIN_OFFSET_VAL_REACHED;
	else if (offset > MAX_OFFSET)
		err = ERR_MAX_OFFSET_VAL_REACHED;

	if(err != ERR_NO_ERROR)
		throw UlException(err);

	storeCalCoefs(range, (float)slope, (float)offset);
	reloadCalCoefs();

	const_cast<AiUsb1608g*>(this)->enableCalMode(false);

}

void AiUsb1608g::getCalOutputIndexs(Range range, unsigned int* Idx) const
{
	Idx[0] = 0;
	switch(range)
	{
		case BIP10VOLTS:
			Idx[1] = 7;
			Idx[2] = 8;
			break;
		case BIP5VOLTS:
			Idx[1] = 5;
			Idx[2] = 6;
			break;
		case BIP2VOLTS:
			Idx[1] = 3;
			Idx[2] = 4;
			break;
		case BIP1VOLTS:
			Idx[1] = 1;
			Idx[2] = 2;
			break;
		default:
			break;
	}
}

void AiUsb1608g::getVRefs(Range range, double* refs) const
{
	float vrefs[CAL_REF_COUNT];
	unsigned int vrefs_addr = VREFS_ADDR;
	int blockSize = sizeof(float);
	unsigned char vref[4];
	int __attribute__((unused)) bytesReceived;

	// read 0V volt ref
	bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, vrefs_addr, vref, blockSize);
	vrefs[0] = mEndian.le_ptr_to_cpu_f32(vref);

	vrefs_addr += getVRefOffset(range) * sizeof(float);

	// read +/- full scale vrefs
	bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, vrefs_addr, (unsigned char*)&vref, blockSize);

	vrefs[1] = mEndian.le_ptr_to_cpu_f32(vref);

	vrefs_addr += getVRefOffset(range) * sizeof(float);

	bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, vrefs_addr, (unsigned char*)&vref, blockSize);

	vrefs[2] = mEndian.le_ptr_to_cpu_f32(vref);

	 // convert vrefs to count
	double offset, scale;
	daqDev().getEuScaling(range, offset, scale);
	double lsb = scale / pow(2.0, mAiInfo.getResolution());

	long midpoint = 1 << (mAiInfo.getResolution() - 1);

	for(int i = 0; i < CAL_REF_COUNT; i++)
		refs[i] = vrefs[i] / lsb + midpoint;

}

unsigned int AiUsb1608g::getVRefOffset(Range range) const
{
	int offset = 0;
	switch(range)
	{
		case BIP10VOLTS:
			offset = 1;
			break;
		case BIP5VOLTS:
			offset = 3;
			break;
		case BIP2VOLTS:
			offset = 5;
			break;
		case BIP1VOLTS:
			offset = 7;
			break;
		default:
			break;
	}

	return offset;
}

void AiUsb1608g::storeCalCoefs(Range range, float slope, float offset) const
{
#pragma pack(1)
	typedef struct
	{
		unsigned char slope[4];
		unsigned char offset[4];
	} TCoefs;
#pragma pack()

	TCoefs calCoef;

	mEndian.cpu_f32_to_le_ptr(slope, calCoef.slope);
	mEndian.cpu_f32_to_le_ptr(offset, calCoef.offset);

	unsigned int calCoefsAddr;

	int bytesWritten = 0;
	int memOffset = 0;

	switch(range)
	{
		case BIP10VOLTS:
			memOffset = 0;
			break;
		case BIP5VOLTS:
			memOffset = 1;
			break;
		case BIP2VOLTS:
			memOffset = 2;
			break;
		case BIP1VOLTS:
			memOffset = 3;
			break;
		default:
			break;
	}

	unsigned short memUnlockCode = daqDev().getMemUnlockCode();
	int unlockAddr = daqDev().getMemUnlockAddr();

	// unlock the memory
	daqDev().memWrite(MT_EEPROM, MR_CAL, unlockAddr, (unsigned char*) &memUnlockCode, sizeof(memUnlockCode));

	calCoefsAddr = mAiInfo.getCalCoefsStartAddr() + memOffset * sizeof(TCoefs);
	daqDev().memWrite(MT_EEPROM, MR_CAL, calCoefsAddr, (unsigned char*) &calCoef, sizeof(calCoef));

	//lock the memory
	unsigned short val = 0;
	bytesWritten = daqDev().memWrite(MT_EEPROM, MR_CAL, unlockAddr, (unsigned char*) &val, sizeof(val));

	if(bytesWritten != sizeof(TCoefs))
		throw UlException(ERR_DEAD_DEV);
}

void AiUsb1608g::storeCalDate() const
{
	// unlock the memory
	unsigned short memUnlockCode = daqDev().getMemUnlockCode();
	int unlockAddr = daqDev().getMemUnlockAddr();

		// unlock the memory
	daqDev().memWrite(MT_EEPROM, MR_CAL, unlockAddr, (unsigned char*) &memUnlockCode, sizeof(memUnlockCode));

	time_t  now = time(0);
	struct tm  now_struct;

	now_struct = *localtime(&now);

	// the date is stored on the device in big-endian format by the test department
	unsigned short calDateBuf[6];
	calDateBuf[0] = mEndian.cpu_to_be_ui16(now_struct.tm_year + 1900);
	calDateBuf[1] = mEndian.cpu_to_be_ui16(now_struct.tm_mon + 1);
	calDateBuf[2] = mEndian.cpu_to_be_ui16(now_struct.tm_mday);
	calDateBuf[3] = mEndian.cpu_to_be_ui16(now_struct.tm_hour);
	calDateBuf[4] = mEndian.cpu_to_be_ui16(now_struct.tm_min);
	calDateBuf[5] = mEndian.cpu_to_be_ui16(now_struct.tm_sec);

	unsigned int calDateAddr = mAiInfo.getCalDateAddr();

	daqDev().memWrite(MT_EEPROM, MR_CAL, calDateAddr, (unsigned char*) &calDateBuf, sizeof(calDateBuf));

	//lock the memory
	unsigned short val = 0;
	daqDev().memWrite(MT_EEPROM, MR_CAL, unlockAddr, (unsigned char*) &val, sizeof(val));
}*/

} /* namespace ul */
