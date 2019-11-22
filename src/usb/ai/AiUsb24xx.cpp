/*
 * AiUsb24xx.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include <unistd.h>

#include "AiUsb24xx.h"
#include "../../utility/Nist.h"

static const int   SIGN_BITMASK = 1 << 23;
static const int   FULL_SCALE24_BITMASK = (1 << 24) - 1;
static const int   SIGN_EXT_BITMASK = ~FULL_SCALE24_BITMASK;

static const int   TIN_VOLTAGE_FLAG	= 1 << 30;

namespace ul
{
AiUsb24xx::AiUsb24xx(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_SINGLEIO | SO_BLOCKIO);

	int chanCount = 16;
	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416 || daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
		chanCount = 64; //chanCount = 32; including the exp channels

	mActualChanCount = chanCount;

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(chanCount);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, chanCount/2);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, chanCount);
	mAiInfo.setChanTypes(AI_VOLTAGE | AI_TC);
	mAiInfo.setChanTypes(0, (chanCount / 2) - 1, AI_VOLTAGE | AI_TC);
	mAiInfo.setChanTypes((chanCount / 2), chanCount - 1, AI_VOLTAGE);
	mAiInfo.setResolution(24);
	mAiInfo.setMinScanRate(minRate);

	double maxChanDataRate = 3750.0;
	double settlingTime = 0.000640;
	double maxRate = 1.0 / ((1.0 / maxChanDataRate) + settlingTime);
	double periodDbl = (mDaqDevice.getClockFreq() / maxRate);

	unsigned int period = periodDbl;
	double actualrate = mDaqDevice.getClockFreq() / period;

	while(actualrate > maxRate)
	{
		period++;
		actualrate = mDaqDevice.getClockFreq() / period;
	}

	mAiInfo.setMaxScanRate(actualrate);
	mAiInfo.setMaxThroughput(maxRate); // since for 15 channels the throughput is really close to maxRate for one channel

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	memset(&mCjcCorrectionValues, 0, sizeof(mCjcCorrectionValues));

	mFieldCalDateAddr = 0xFE0;

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416 || daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		mAiInfo.setNumCjcChans(8); // including the exp's cjc channels
		mAiInfo.setCalCoefsStartAddr(0xA0);
		mAiInfo.setCalDateAddr(0xFD0); // self cal address
		mAiInfo.setCalCoefCount(10);  // 9 ranges + TC coefs

		const double DEFAULTS[16]={ 1.2004, 0.9439, 0.6703, 0.3711, 1.2506, 1.1418, 1.0145, 0.9315, 0.8663, 1.2399, 1.5692, 1.8611, 0.1736, 0.6306, 1.0432, 1.4027 };

		memcpy(mCjcCorrectionValues, DEFAULTS, sizeof(DEFAULTS));
	}
	else
	{
		mAiInfo.setNumCjcChans(2);
		mAiInfo.setCalCoefsStartAddr(0xB0);
		mAiInfo.setCalDateAddr(0xFD0); // self cal address
		mAiInfo.setCalCoefCount(9);  // 8 ranges + TC coefs

		const double DEFAULTS[8]={ 1.130, 0.920, 0.610, 0.240, 0.370, 0.670, 0.960, 1.00 };

		memcpy(mCjcCorrectionValues, DEFAULTS, sizeof(DEFAULTS));
	}

	mAiInfo.setSampleSize(4);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x81);

	setScanStopCmd(CMD_AINSTOP);

	mScanTempChanSupported = true;

	initCustomScales();

	initChanConfig();

	mHadExp = false;
	mActualCjcCount = 0;
	mScanHasTcChan = false;

	memset(&mCjcTemps, 0, sizeof(mCjcTemps));
	memset(&mChanCjcVal, 0, sizeof(mChanCjcVal));
	memset(&mScanConfig, 0, sizeof(mScanConfig));
	memset(&mLastCjcUpdateTime, 0, sizeof(mLastCjcUpdateTime));
	memset(&mScanChanInfo, 0, sizeof(mScanChanInfo));

	UlLock::initMutex(mCjcsMutex, PTHREAD_MUTEX_RECURSIVE);
}

AiUsb24xx::~AiUsb24xx()
{
	UlLock::destroyMutex(mCjcsMutex);
}

void AiUsb24xx::initialize()
{
	try
	{
		mActualChanCount = mAiInfo.getNumChansByMode(AI_SINGLE_ENDED);
		mActualCjcCount = mAiInfo.getNumCjcChans();

		if(daqDev().getDeviceType() == DaqDeviceId::USB_2416 || daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
		{
			if(!daqDev().hasExp())
			{
				mActualChanCount /= 2;
				mActualCjcCount	/= 2;
			}

			// reinitialize the channel config on 2416 if the status of exp connection changed since last initialization
			// this has to be done since channel arrangement changes when exp is attached or detached.
			if(mHadExp != daqDev().hasExp()) //always false for 2408
			{
				//initCustomScales();
				initChanConfig();
			}
		}

		initChanNums();

		//sendStopCmd(); // Commented out because sending this command before memread it will cause stall on 2408 and reset on 2416

		loadAdcCoefficients();

		mHadExp = daqDev().hasExp();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double AiUsb24xx::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex); // call before check_AIn_Args() so it can detect ALREADY_ACTIVE error

	AInFlag ainFlag = (AInFlag)(flags & ~TIN_VOLTAGE_FLAG);
	check_AIn_Args(channel, inputMode, range, ainFlag);

#pragma pack(1)
	union
	{
		struct
		{
			unsigned char data[3];
			unsigned char flags;
		};
		unsigned int rawVal;
	} input;
#pragma pack()

	TAIN_CFG ain;
	double data = 0.0;
	unsigned int rawVal;

	ain.cfg.channel = mActualChanNum[channel];
	ain.cfg.mode = mapModeCode(channel, inputMode);
	ain.cfg.range = (mChanCfg[channel].chanType == AI_TC) ? mapRangeCode(BIPPT078VOLTS) : mapRangeCode(range);
	ain.cfg.rate = mChanCfg[channel].chanDataRateIdx;

	input.rawVal = 0;

	daqDev().queryCmd(CMD_AIN, ain.raw.wValue, ain.raw.wIndex, (unsigned char*) &input, sizeof(input));

	if(mChanCfg[channel].chanType == AI_TC && mChanCfg[channel].detectOpenTc)
	{
		if((input.flags & 0x80) && !(flags & TIN_VOLTAGE_FLAG))
		{
			data = -9999.0;
			throw UlException(ERR_OPEN_CONNECTION);
		}
	}

	input.flags = 0; // clear MSB

	rawVal = Endian::le_ui32_to_cpu(input.rawVal);

	int i32RawVal = convertI24ToI32(rawVal);

	int calI32 = 0;

	if(!(flags & NOCALIBRATEDATA))
	{
		// raw data must be calibrated before conversion from i32 to u32;

		CalCoef coef = getCalCoef(channel, inputMode, range, flags);
		calI32 = (coef.slope * i32RawVal) + coef.offset;
	}
	else
	{
		calI32 = i32RawVal;
	}

	unsigned int calU32 = convertToU32(calI32);

	if (!(flags & NOSCALEDATA))
	{
		double offset = 0;
		double scale = 0;
		mDaqDevice.getEuScaling(range, scale, offset);

		double lsb = scale / 0x1000000; //pow(2.0, mAiInfo.getResolution());
		data = (calU32 * lsb)  + offset;

		if(mChanCfg[channel].chanType == AI_TC && !(flags & TIN_VOLTAGE_FLAG))
		{
			updateCjcValues();

			unsigned char tcType = mChanCfg[channel].tcType - 1;  // zero based
			double cjcTemp = mChanCjcVal[channel];

			double cjc_volts = NISTCalcVoltage(tcType, cjcTemp);

			double tc_volts = data * 1000;

			double tc_temp = NISTCalcTemp(tcType, tc_volts + cjc_volts);

			if(tc_temp < -273.0)
				 return data = -9999.0;
			else
				data = tc_temp;
		}
	}
	else
	{
		data = calU32;
	}

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

void AiUsb24xx::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	check_TIn_Args(channel, scale, flags);

	double tempVal = 0;

	if(channel & 0x80) // CJC chan
	{
		updateCjcValues();

		int cjc = channel - 0x80;
		tempVal = mCjcTemps[cjc];
	}
	else
	{
		int ainFlags = 0;
		if(scale == TS_VOLTS || scale == TS_NOSCALE)
			ainFlags = TIN_VOLTAGE_FLAG;

		tempVal = aIn(channel, AI_DIFFERENTIAL, BIPPT078VOLTS, (AInFlag) ainFlags);
	}

	if(tempVal != -9999.0)
		*data = convertTempUnit(tempVal, (TempUnit) scale);
	else
		*data = tempVal;
}

double AiUsb24xx::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex); // call before check_AInScan_Args() so it can detect ALREADY_ACTIVE error

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	int epAddr = getScanEndpointAddr();
	bool retry = false;

	setTransferMode(options, rate);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	sendStopCmd();// send stop just in case the last scan was not terminated correctly due to crash

start:

	loadAInScanQueue(inputMode, range, lowChan, highChan);

	daqDev().clearFifo(epAddr);
	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	setScanConfig(lowChan, highChan, samplesPerChan, rate, options);

	if(mScanHasTcChan)
		updateCjcValues();

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_AINSCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		// if for any reason (i.e applications crash) stop command was not sent while the previous scan was running then the next ainscan start command will fail.
		// this behavior is due to an issue with the USB-2416 fw. Here we have to empty the fifo before sending stop command to be able to run scan again. Added 50 ms to allow all pending
		// request empty the fifo before sending the stop command
		if(daqDev().getDeviceType() == DaqDeviceId::USB_2416 ||
		   daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
		{
			usleep(50000);

			// try one more time
			if(!retry)
				retry = true;
			else
				retry = false;
		}

		stopBackground();

		if(retry)
			goto start;

		throw e;
	}

	return actualScanRate();
}

void AiUsb24xx::setTransferMode(ScanOption scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if(!(scanOptions & SO_BURSTIO))
	{
		if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 128.0))
			mTransferMode = SO_SINGLEIO;
	}
}

void AiUsb24xx::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(lowChan, highChan, rate));

	mScanConfig.scan_count = 0;// Note: run scan in continuous mode since max scan count is limited to 65535 //Endian::cpu_to_le_ui32(scanCount);

	//if(options & SO_CONTINUOUS)
	//	mScanConfig.scan_count = 0;

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	int epAddr = getScanEndpointAddr();
	mScanConfig.packet_size = (getTransferMode() == SO_BLOCKIO) ? daqDev().getBulkEndpointMaxPacketSize(epAddr)/4 - 1 : chanCount - 1;
}

double AiUsb24xx::calcMaxRate(int lowChan, int highChan)
{
	int channel = 0;
	const double settlingTime = 0.000640;
	double maxRate = 0;

	double minPeriod = 0;

	if(!queueEnabled())
	{
		for(channel = lowChan; channel <= highChan; channel++)
		{
			minPeriod += (1.0 / mChanCfg[channel].chanDataRate) + settlingTime;
		}
	}
	else
	{
		for(unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			channel = mAQueue.at(idx).channel;

			minPeriod += (1.0 / mChanCfg[channel].chanDataRate) + settlingTime;
		}
	}


	maxRate = 1 / minPeriod;

	return maxRate;
}

unsigned int AiUsb24xx::calcPacerPeriod(int lowChan, int highChan, double rate)
{
	unsigned int period = 0;

	double ratePerChan = rate;

	double maxRate = calcMaxRate(lowChan, highChan);

	if(rate > maxRate)
		ratePerChan = maxRate;

	double clockFreq = mDaqDevice.getClockFreq();
	double periodDbl = (clockFreq / ratePerChan);

	if(periodDbl > UINT_MAX)
		periodDbl = UINT_MAX;

	period = periodDbl;

	double actualrate = clockFreq / period;

	while(actualrate > maxRate)
	{
		period++;
		actualrate = clockFreq / period;
	}

	setActualScanRate(actualrate);

	return period;
}

void AiUsb24xx::loadAInScanQueue(AiInputMode inputMode, Range range, int lowChan, int highChan)
{
#pragma pack(1)
	struct
	{
		unsigned char count;
		TCHAN_CFG element[64];
	}queue;
#pragma pack()

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;
	mScanHasTcChan = false;

	int channel;

	memset(&queue, 0, sizeof(queue));

	queue.count = chanCount;

	unsigned int idx = 0;

	if(!queueEnabled())
	{
		for(channel = lowChan; channel <= highChan; channel++)
		{
			queue.element[idx].channel = mActualChanNum[channel];
			queue.element[idx].mode = mapModeCode(channel, inputMode);
			queue.element[idx].range = (mChanCfg[channel].chanType == AI_TC) ? mapRangeCode(BIPPT078VOLTS) : mapRangeCode(range);
			queue.element[idx].rate = mChanCfg[channel].chanDataRateIdx;


			mScanChanInfo[idx].channel = channel;
			mScanChanInfo[idx].chanType = mChanCfg[channel].chanType;
			mScanChanInfo[idx].range = (mChanCfg[channel].chanType == AI_TC) ? BIPPT078VOLTS : range;
			mScanChanInfo[idx].tcType = mChanCfg[channel].tcType;
			mScanChanInfo[idx].detectOpenTc = mChanCfg[channel].detectOpenTc;

			if(mChanCfg[channel].chanType == AI_TC)
				mScanHasTcChan = true;

			idx++;
		}
	}
	else
	{
		for(idx = 0; idx < mAQueue.size(); idx++)
		{
			channel = mAQueue.at(idx).channel;
			inputMode = mAQueue.at(idx).inputMode;
			range = mAQueue.at(idx).range;

			queue.element[idx].channel = mActualChanNum[channel];
			queue.element[idx].mode = mapModeCode(channel, inputMode);
			queue.element[idx].range = (mChanCfg[channel].chanType == AI_TC) ? mapRangeCode(BIPPT078VOLTS) : mapRangeCode(range);
			queue.element[idx].rate = mChanCfg[channel].chanDataRateIdx;

			mScanChanInfo[idx].channel = channel;
			mScanChanInfo[idx].chanType = mChanCfg[channel].chanType;
			mScanChanInfo[idx].range = (mChanCfg[channel].chanType == AI_TC) ? BIPPT078VOLTS : range;
			mScanChanInfo[idx].tcType = mChanCfg[channel].tcType;
			mScanChanInfo[idx].detectOpenTc = mChanCfg[channel].detectOpenTc;

			if(mChanCfg[channel].chanType == AI_TC)
				mScanHasTcChan = true;
		}
	}

	uint16_t buffLen = (chanCount * sizeof(TCHAN_CFG)) + 1;

	daqDev().sendCmd(CMD_AINSCAN_QUEUE, 0, 0, (unsigned char*)&queue, buffLen);
}



void AiUsb24xx::updateCjcValues()
{
	short data[8];
	unsigned short dataCount = 2;

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416 || daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
		dataCount = 8;

	daqDev().queryCmd(CMD_CJC, 0, 0, (unsigned char*) &data, dataCount * sizeof(short));

	UlLock lock(mCjcsMutex);

	for(int cjc = 0; cjc < mActualCjcCount; cjc++)
	{
		double temp  = data[cjc];
		temp = temp / 256.0;

		mCjcTemps[cjc] = temp;

		int startChan = cjc * 4;
		int endChan = startChan + 4;

		for(int chan = startChan; chan < endChan; chan++)
		{
			mChanCjcVal[chan] = mCjcTemps[cjc] - mCjcCorrectionValues[chan];
		}
	}

	gettimeofday(&mLastCjcUpdateTime, NULL);
}

void AiUsb24xx::copyCjcValues(double cjcValues[32])
{
	UlLock lock(mCjcsMutex);

	memcpy(cjcValues, mChanCjcVal, sizeof(mChanCjcVal));
}

void AiUsb24xx::check_AIn_Args(int channel, AiInputMode inputMode, Range range, AInFlag flags) const
{
	int seChanCount = mActualChanCount;
	int diffChanCount = mActualChanCount / 2;

	if(!mAiInfo.isInputModeSupported(inputMode))
		throw UlException(ERR_BAD_INPUT_MODE);

	if(channel < 0 || channel >= seChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(inputMode == AI_DIFFERENTIAL && channel >= diffChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(!mAiInfo.isRangeSupported(inputMode, range))
		throw UlException(ERR_BAD_RANGE);

	if(mChanCfg[channel].chanType == AI_TC)
	{
		if(inputMode == AI_SINGLE_ENDED)
			throw UlException(ERR_BAD_INPUT_MODE);

		if(range != BIPPT078VOLTS)
			throw UlException(ERR_BAD_RANGE);
	}

	if(~mAiInfo.getAInFlags() & flags)
			throw UlException(ERR_BAD_FLAG);

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

	if((int) mCustomScales.size() < mActualChanCount)
		throw UlException(ERR_INTERNAL);
}

void AiUsb24xx::check_AInScan_Args(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) const
{
	int numOfScanChan = 0;

	int diffChanCount = mActualChanCount / 2;

	if(!mAiInfo.hasPacer())
		throw UlException(ERR_BAD_DEV_TYPE);

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(((options & SO_SINGLEIO) && (options & SO_BLOCKIO)) || ((options & SO_SINGLEIO) && (options & SO_BURSTIO)) || ((options & SO_BLOCKIO) && (options & SO_BURSTIO)))
		throw UlException(ERR_BAD_OPTION);

	if(queueEnabled())
		numOfScanChan = queueLength();
	else
	{
		if(!mAiInfo.isInputModeSupported(inputMode))
			throw UlException(ERR_BAD_INPUT_MODE);

		if(lowChan < 0 || highChan < 0 || lowChan >= mActualChanCount || highChan >= mActualChanCount || lowChan > highChan )
			throw UlException(ERR_BAD_AI_CHAN);

		if(inputMode == AI_DIFFERENTIAL && (lowChan >= diffChanCount || highChan>= diffChanCount))
			throw UlException(ERR_BAD_AI_CHAN);

		numOfScanChan = highChan - lowChan + 1;

		if(!mAiInfo.isRangeSupported(inputMode, range))
			throw UlException(ERR_BAD_RANGE);
	}

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(~mAiInfo.getScanOptions() & options)
		throw UlException(ERR_BAD_OPTION);

	if(~mAiInfo.getAInScanFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	double throughput = rate * numOfScanChan;

	if(!(options & SO_EXTCLOCK))
	{
		double maxRate = 1087.0; // the max rate is 1086.9565217391305 but we set it to 1087 so if user specifies a rate above max rate but close to the actual max rate i.e 1086.96 then it won't rejected
		if(((options & SO_BURSTIO) && (rate > mAiInfo.getMaxBurstRate() || throughput > mAiInfo.getMaxBurstThroughput())) || (!(options & SO_BURSTIO) && (rate > maxRate/*mAiInfo.getMaxScanRate()*/ || throughput > mAiInfo.getMaxThroughput())) )
			throw UlException(ERR_BAD_RATE);
	}

	if(rate <= 0.0)
		throw UlException(ERR_BAD_RATE);

	if(samplesPerChan < mMinScanSampleCount)
		throw UlException(ERR_BAD_SAMPLE_COUNT);

	long long totalCount = (long long) samplesPerChan * numOfScanChan;

	if(options & SO_BURSTIO)
	{
		if(totalCount > (mAiInfo.getFifoSize() / mAiInfo.getSampleSize()))
			throw UlException(ERR_BAD_BURSTIO_COUNT);
		else if (options & SO_CONTINUOUS)
			throw UlException(ERR_BAD_OPTION);
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);


	if((int) mCustomScales.size() < mAiInfo.getNumChans())
		throw UlException(ERR_INTERNAL);
}

void AiUsb24xx::check_AInLoadQueue_Args(const AiQueueElement queue[], unsigned int numElements) const
{
	if(queue != NULL)
	{
		if(mAiInfo.getQueueTypes())
		{
			int diffChanCount = mActualChanCount / 2;

			for(unsigned int i = 0; i < numElements; i++)
			{
				if(!mAiInfo.isInputModeSupported(queue[i].inputMode))
					throw UlException(ERR_BAD_INPUT_MODE);

				if(numElements > (unsigned int) mAiInfo.getMaxQueueLength(queue[i].inputMode))
					throw UlException(ERR_BAD_QUEUE_SIZE);

				if(queue[i].channel < 0 || queue[i].channel >= mActualChanCount)
					throw UlException(ERR_BAD_AI_CHAN);

				if(queue[i].inputMode == AI_DIFFERENTIAL && queue[i].channel >= diffChanCount)
					throw UlException(ERR_BAD_AI_CHAN);

				if(!mAiInfo.isRangeSupported(queue[i].inputMode, queue[i].range))
					throw UlException(ERR_BAD_RANGE);
			}

			if(!isValidChanQueue(queue, numElements))
				throw UlException(ERR_BAD_AI_CHAN_QUEUE);

			if(!isValidGainQueue(queue, numElements))
				throw UlException(ERR_BAD_AI_GAIN_QUEUE);

			if(!isValidModeQueue(queue, numElements))
				throw UlException(ERR_BAD_AI_MODE_QUEUE);
		}
		else
			throw UlException(ERR_BAD_DEV_TYPE);
	}
}

void AiUsb24xx::check_TIn_Args(int channel, TempScale scale, TInFlag flags) const
{
	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

	int diffChanCount = mActualChanCount / 2;

	if(channel < 0 || channel >= diffChanCount)
	{
		bool cjcChan = false;

		if(channel > 0)
		{
			for(int i = 0; i < mActualCjcCount; i++)
			{
				if(channel == (0x80 + i))
				{
					cjcChan = true;
					break;
				}
			}
		}

		if(!cjcChan)
			throw UlException(ERR_BAD_AI_CHAN);
	}
	else if(mChanCfg[channel].chanType != AI_TC)
	{
		throw UlException(ERR_BAD_AI_CHAN_TYPE);
	}

	if(~mAiInfo.getTInFlags() & flags)
		throw UlException(ERR_BAD_FLAG);
}

int AiUsb24xx::convertI24ToI32(int i24)
{
	unsigned int i32 = 0;

	i24 = i24 & 0x00FFFFFF;

	if (i24 & SIGN_BITMASK)
		i32 = i24 | SIGN_EXT_BITMASK;
	else
		i32 = i24 & FULL_SCALE24_BITMASK;

	return i32;
}

unsigned int AiUsb24xx::convertToU32(int i32)
{
	unsigned int ui32 = 0;


	memcpy(&ui32, &i32, sizeof(ui32));

	if (i32 < 0 )
	{
		ui32 &= FULL_SCALE24_BITMASK;
		if (ui32 < SIGN_BITMASK)
			ui32 = 0;
		else
			ui32 -= SIGN_BITMASK;
	}
	else
	{
		ui32 += SIGN_BITMASK;
		if (ui32 > FULL_SCALE24_BITMASK)
			ui32 = FULL_SCALE24_BITMASK;
	}

	return ui32;
}

int AiUsb24xx::mapRangeCode(Range range) const
{
	int rangeCode;

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
	case BIP2PT5VOLTS:
		rangeCode = 3;
		break;
	case BIP1PT25VOLTS:
		rangeCode = 4;
		break;
	case BIPPT625VOLTS:
		rangeCode = 5;
		break;
	case BIPPT312VOLTS:
		rangeCode = 6;
		break;
	case BIPPT156VOLTS:
		rangeCode = 7;
		break;
	case BIPPT078VOLTS:
		rangeCode = 8;
		break;

	default:
		throw UlException(ERR_BAD_RANGE);
	}

	return rangeCode;
}

int AiUsb24xx::mapModeCode(int channel, AiInputMode mode) const
{
	int modeCode;

	if(mChanCfg[channel].chanType == AI_TC)
	{
		if(mChanCfg[channel].detectOpenTc)
			modeCode = MODE_TC;
		else
			modeCode =	MODE_TC_NO_OTD;
	}
	else
	{
		if(mode == AI_DIFFERENTIAL)
			modeCode = MODE_DIFF;
		else
		{
			int diffChanCount = mActualChanCount / 2;

			if(channel < diffChanCount)
				modeCode = MODE_SE_HI;
			else
				modeCode = MODE_SE_LO;
		}
	}

	return modeCode;
}

void AiUsb24xx::loadAdcCoefficients()
{
#pragma pack(1)
	typedef struct
	{
		unsigned char slope[8];
		unsigned char offset[8];
	}  coef;
#pragma pack()

	CalCoef calCoef;

	if(getScanState() == SS_IDLE)
	{
		mCalCoefs.clear();

		int calCoefCount = mAiInfo.getCalCoefCount();
		int calBlockSize = calCoefCount * sizeof(coef);
		int address = mAiInfo.getCalCoefsStartAddr();

		coef* buffer = new coef[calCoefCount];

		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, address, (unsigned char*)buffer, calBlockSize);

		if(bytesReceived == calBlockSize)
		{
			for(int i = 0; i < calCoefCount; i++)
			{
				calCoef.slope = mEndian.le_ptr_to_cpu_f64(buffer[i].slope);
				calCoef.offset = mEndian.le_ptr_to_cpu_f64(buffer[i].offset);

				mCalCoefs.push_back(calCoef);
			}
		}

		delete [] buffer;

		readCalDate();
	}
}

int AiUsb24xx::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	int calCoefIndex = mapRangeCode(range);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2408 || daqDev().getDeviceType() == DaqDeviceId::USB_2408_2AO)
		calCoefIndex--;

	return calCoefIndex;
}

CalCoef AiUsb24xx::getCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const
{
	CalCoef coef;

	if(mCalCoefs.empty())
		const_cast<AiUsb24xx*>(this)->loadAdcCoefficients();

	if(!mCalCoefs.empty())
	{
		int calCoefIdx =  getCalCoefIndex(channel, inputMode, range);

		if(mChanCfg[channel].chanType == AI_TC)
			calCoefIdx = mAiInfo.getCalCoefCount() - 1; // last cal coef is the TC coefficient

		coef = mCalCoefs[calCoefIdx];
	}
	else
		throw UlException(ERR_DEAD_DEV);

	return coef;
}


UlError AiUsb24xx::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned char cmd = CMD_AINSCAN_STATUS;

#pragma pack(1)
	struct
	{
		unsigned short depth;
		unsigned char status;
	}scan;
#pragma pack()


	try
	{
		daqDev().queryCmd(cmd, 0, 0, (unsigned char*)&scan, sizeof(scan));

		if(scan.status & 0x02)
		{
			err = ERR_OVERRUN;
		}
		else if(scan.status & 0x04)
		{
			err = ERR_PACER_OVERRUN;
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

void AiUsb24xx::initChanConfig()
{
	UlLock lock(mIoDeviceMutex);

	for(int i = 0; i < 64; i++)
	{
		mChanCfg[i].chanType = AI_VOLTAGE;
		mChanCfg[i].chanDataRateIdx = CHR_3750;
		mChanCfg[i].chanDataRate = 3750;
		mChanCfg[i].tcType = TC_J;
		mChanCfg[i].detectOpenTc = false;
	}
}

void AiUsb24xx::initChanNums()
{
	int seChanCount = mActualChanCount;
	int diffChanCount = mActualChanCount / 2;

	for(int i = 0; i < 64; i++)
		mActualChanNum[i] = -1;

	for(int i = 0; i < seChanCount; i++)
	{
		if(i >= diffChanCount)
			mActualChanNum[i] = i % diffChanCount;
		else
			mActualChanNum[i] = i;
	}
}

double AiUsb24xx::getChanDataRate(int chanRateIndex)
{
	double chanRate = 0;

	switch(chanRateIndex)
	{
	case CHR_30000:
		chanRate = 30000.0;
		break;
	case CHR_15000:
		chanRate = 15000.0;
		break;
	case CHR_7500:
		chanRate = 7500.0;
		break;
	case CHR_2000:
		chanRate = 2000.0;
		break;
	case CHR_1000:
		chanRate = 1000.0;
		break;
	case CHR_500:
		chanRate = 500.0;
		break;
	case CHR_100:
		chanRate = 100.0;
		break;
	case CHR_60:
		chanRate = 60.0;
		break;
	case CHR_30:
		chanRate = 30.0;
		break;
	case CHR_25:
		chanRate = 25.0;
		break;
	case CHR_15:
		chanRate = 15.0;
		break;
	case CHR_10:
		chanRate = 10.0;
		break;
	case CHR_5:
		chanRate = 5.0;
		break;
	case CHR_2PT5:
		chanRate = 2.5;
		break;
	}

	return chanRate;
}

void AiUsb24xx::readCalDate()
{
	unsigned char calDateBuf[6];
	int calDateAddr = mAiInfo.getCalDateAddr();

	if(calDateAddr != -1 && getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_RESERVED0, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

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
					mCalDate = cal_date_sec;

				// convert seconds to string

				/*struct tm *timeinfo;
				timeinfo = localtime(&cal_date_sec);
				char b[100];
				strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
				std::cout << b << std::endl;*/
			}
		}

		/// read field cal date
		calDateAddr = mFieldCalDateAddr;

		bytesReceived = daqDev().memRead(MT_EEPROM, MR_RESERVED0, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

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

void AiUsb24xx::processScanData32(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	double data;
	unsigned int rawVal;

	double* dataBuffer = (double*) mScanInfo.dataBuffer;

	int i32RawVal;
	int calI32;
	unsigned int calU32;
	double offset = 0;
	double scale = 0;
	Range range;

	double cjcValues[32] = {0};

	if(mScanHasTcChan)
		copyCjcValues(cjcValues);

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui32_to_cpu(buffer[numOfSampleCopied]);

		if(mScanChanInfo[mScanInfo.currentCalCoefIdx].chanType == AI_TC &&
		   mScanChanInfo[mScanInfo.currentCalCoefIdx].detectOpenTc && (rawVal & 0x80000000))
		{
			dataBuffer[mScanInfo.currentDataBufferIdx] = -9999.0;
		}
		else
		{
			rawVal = rawVal & 0x00FFFFFF;
			i32RawVal = convertI24ToI32(rawVal);

			if(!(mScanInfo.flags & NOCALIBRATEDATA))
			{
				// raw data must be calibrated before conversion from i32 to u32;
				calI32 = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * i32RawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
			}
			else
			{
				calI32 = i32RawVal;
			}

			calU32 = convertToU32(calI32);

			if (!(mScanInfo.flags & NOSCALEDATA))
			{

				range = mScanChanInfo[mScanInfo.currentCalCoefIdx].range;
				mDaqDevice.getEuScaling(range, scale, offset);

				double lsb = scale / 0x1000000; //pow(2.0, mAiInfo.getResolution());
				data = (calU32 * lsb)  + offset;

				if(mScanChanInfo[mScanInfo.currentCalCoefIdx].chanType == AI_TC)
				{
					unsigned char tcType = mScanChanInfo[mScanInfo.currentCalCoefIdx].tcType - 1;  // zero based
					double cjcTemp = cjcValues[mScanChanInfo[mScanInfo.currentCalCoefIdx].channel];

					double cjc_volts = NISTCalcVoltage(tcType, cjcTemp);

					double tc_volts = data * 1000;

					double tc_temp = NISTCalcTemp(tcType, tc_volts + cjc_volts);

					if(tc_temp < -273.0)
						data = -9999.9;
					else
						data = convertTempUnit(tc_temp, mScanTempUnit);
				}
			}
			else
			{
				data = calU32;
			}

			dataBuffer[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;
		}

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}
}

void AiUsb24xx::updateScanParam(int param)
{
	if(mScanHasTcChan)
	{
		timeval now;
		gettimeofday(&now, NULL);
		double t1 = (now.tv_sec * 1000000.0 + now.tv_usec) / 1000.0;

		double t0 = (mLastCjcUpdateTime.tv_sec * 1000000.0 + mLastCjcUpdateTime.tv_usec) / 1000.0;

		double duration = t1 - t0;

		if(duration > 10.0)
		{
			updateCjcValues();
		}
	}
}

void AiUsb24xx::initCustomScales()
{
	mCustomScales.clear();

	CustomScale coef;
	for(int i = 0; i < mActualChanCount; i++)
	{
		coef.slope = 1.0;
		coef.offset = 0;

		mCustomScales.push_back(coef);
	}
}

void AiUsb24xx::addSupportedRanges()
{
	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416 || daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		mAiInfo.addRange(AI_SINGLE_ENDED, BIP20VOLTS);
		mAiInfo.addRange(AI_DIFFERENTIAL, BIP20VOLTS);
	}

	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP2PT5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP1PT25VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIPPT625VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIPPT312VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIPPT156VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIPPT078VOLTS);

	mAiInfo.addRange(AI_DIFFERENTIAL, BIP10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP2PT5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP1PT25VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT625VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT312VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT156VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT078VOLTS);

}

void AiUsb24xx::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 64);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 64);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE | MODE_QUEUE);
}

//////////////////////          Configuration functions          /////////////////////////////////

void AiUsb24xx::setCfg_ChanType(int channel, AiChanType chanType)
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	int seChanCount = mActualChanCount;
	int diffChanCount = mActualChanCount / 2;

	if(channel < 0 || channel >= seChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(!(mAiInfo.getChanTypes() & chanType))
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	if(chanType == AI_TC && channel >= diffChanCount)
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	UlLock lock(mIoDeviceMutex); // acquire the lock so ALREADY_ACTIVE error can be detected correctly on multi-threaded processes

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	mChanCfg[channel].chanType = chanType;
}

AiChanType AiUsb24xx::getCfg_ChanType(int channel) const
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	return mChanCfg[channel].chanType;
}

void AiUsb24xx::setCfg_ChanTcType(int channel, TcType tcType)
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	int diffChanCount = mActualChanCount / 2;

	if(channel < 0 || channel >= diffChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(tcType < TC_J || tcType > TC_N)
		throw UlException(ERR_BAD_TC_TYPE);

	if(mChanCfg[channel].chanType != AI_TC)
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	UlLock lock(mIoDeviceMutex); // acquire the lock so ALREADY_ACTIVE error can be detected correctly on multi-threaded processes

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	mChanCfg[channel].tcType = tcType;

}
TcType AiUsb24xx::getCfg_ChanTcType(int channel) const
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	int diffChanCount = mActualChanCount / 2;

	if(channel < 0 || channel >= diffChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(mChanCfg[channel].chanType != AI_TC)
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	return mChanCfg[channel].tcType;
}

void  AiUsb24xx::setCfg_ChanDataRate(int channel, double rate)
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	UlLock lock(mIoDeviceMutex); // acquire the lock so ALREADY_ACTIVE error can be detected correctly on multi-threaded processes

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(rate >= 3750.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_3750;
		mChanCfg[channel].chanDataRate = 3750.0;
	}
	else if(rate >= 2000.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_2000;
		mChanCfg[channel].chanDataRate = 2000.0;
	}
	else if(rate >= 1000.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_1000;
		mChanCfg[channel].chanDataRate = 1000.0;
	}
	else if(rate >= 500.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_500;
		mChanCfg[channel].chanDataRate = 500.0;
	}
	else if(rate >= 100.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_100;
		mChanCfg[channel].chanDataRate = 100.0;
	}
	else if(rate >= 60.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_60;
		mChanCfg[channel].chanDataRate = 60.0;
	}
	else if(rate >= 50.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_50;
		mChanCfg[channel].chanDataRate = 50.0;
	}
	else if(rate >= 30.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_30;
		mChanCfg[channel].chanDataRate = 30.0;
	}
	else if(rate >= 25.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_25;
		mChanCfg[channel].chanDataRate = 25.0;
	}
	else if(rate >= 15.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_15;
		mChanCfg[channel].chanDataRate = 15.0;
	}
	else if(rate >= 10.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_10;
		mChanCfg[channel].chanDataRate = 10.0;
	}
	else if(rate >= 5.0)
	{
		mChanCfg[channel].chanDataRateIdx = CHR_5;
		mChanCfg[channel].chanDataRate = 5.0;
	}
	else
	{
		mChanCfg[channel].chanDataRateIdx = CHR_2PT5;
		mChanCfg[channel].chanDataRate = 2.5;
	}
}
double  AiUsb24xx::getCfg_ChanDataRate(int channel) const
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	return mChanCfg[channel].chanDataRate;
}

void  AiUsb24xx::setCfg_ChanOpenTcDetectionMode(int channel, OtdMode mode)
{
	int diffChanCount = mActualChanCount / 2;

	if(channel < 0 || channel >= diffChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(mChanCfg[channel].chanType != AI_TC)
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	UlLock lock(mIoDeviceMutex); // acquire the lock so ALREADY_ACTIVE error can be detected correctly on multi-threaded processes

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(mode == OTD_ENABLED)
		mChanCfg[channel].detectOpenTc = true;
	else
		mChanCfg[channel].detectOpenTc = false;
}
OtdMode  AiUsb24xx::getCfg_ChanOpenTcDetectionMode(int channel) const
{
	int diffChanCount = mActualChanCount / 2;

	if(channel < 0 || channel >= diffChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(mChanCfg[channel].chanType != AI_TC)
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	OtdMode mode = OTD_DISABLED;

	if(mChanCfg[channel].detectOpenTc)
		mode = OTD_ENABLED;

	return mode;
}

} /* namespace ul */
