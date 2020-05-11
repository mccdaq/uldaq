/*
 * AiE1608.cpp
 *
*     Author: Measurement Computing Corporation
 */

#include "AiE1608.h"
#include <limits.h>

namespace ul
{
AiE1608::AiE1608(const NetDaqDevice& daqDevice) : AiNetBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA | AIN_FF_NOCALIBRATEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA | AINSCAN_FF_NOCALIBRATEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_BLOCKIO );
	mAiInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAiInfo.hasPacer(true);
	mAiInfo.setNumChans(8);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 4);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 8);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 7, AI_VOLTAGE);
	mAiInfo.setResolution(16);
	mAiInfo.setMinScanRate(minRate);

	mAiInfo.setMaxScanRate(250000);
	mAiInfo.setMaxThroughput(250000);

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalCoefsStartAddr(0);
	mAiInfo.setCalDateAddr(0x50);
	mAiInfo.setCalCoefCount(8);
	mAiInfo.setSampleSize(2);

	addSupportedRanges();
	addQueueInfo();

	initCustomScales();

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

AiE1608::~AiE1608()
{
}

void AiE1608::initialize()
{
	try
	{
//		sendStopCmd();

		loadAdcCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double AiE1608::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned short rawVal = 0;

	unsigned char rangeCode = mapRangeCode(range);
	unsigned char chanCode = getChanCode(channel, inputMode);
	unsigned char params[2]={chanCode, rangeCode};

	daqDev().queryCmd(CMD_AIN, params, sizeof(params), (unsigned char*) &rawVal, sizeof(rawVal));

	rawVal = Endian::le_ui16_to_cpu(rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiE1608::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales = getCustomScales(lowChan, highChan);

	loadAInConfigs(inputMode, range, lowChan, highChan, queueEnabled());

	setScanInfo(FT_AI, chanCount, samplesPerChan, mAiInfo.getSampleSize(), mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	setScanConfig(chanCount, samplesPerChan, rate, options);

	int timeout = 1000 +  (1000 / (chanCount * actualScanRate()));

	daqDev().scanTranserIn()->initilizeTransfer(this, mAiInfo.getSampleSize(), timeout);


	try
	{
		daqDev().queryCmd(CMD_AINSCAN_START, (unsigned char*) &mScanConfig, sizeof(mScanConfig));

		// don't remove. Added to eliminates 200 ms of ack. This forces the ACK to be sent immediately for the start command since scan won't start
		// until host ACKs the start response from the host
		daqDev().flushCmdSocket();

		daqDev().scanTranserIn()->start();

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

UlError AiE1608::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	UlError err = ERR_NO_ERROR;

	if(status && xferStatus)
	{
		ScanStatus scanStatus = getScanState();

		getXferStatus(xferStatus);

		if(scanStatus != SS_RUNNING)
			err = daqDev().scanTranserIn()->getXferError();

		*status = scanStatus;
	}
	else
		err = ERR_BAD_ARG;

	return err;
}

void AiE1608::sendStopCmd()
{
	//Note: This is implemented differently from Windows and Android
	// the stop command always forces  close data socket from the daq device

	unsigned char	closeSocket = 1;

	daqDev().queryCmd(CMD_AINSTOP, &closeSocket, sizeof(closeSocket));

	//daqDev().queryCmd(CMD_AINSTOP);
}


UlError AiE1608::terminateScan()
{
	UlError err = ERR_NO_ERROR;

	try
	{
		sendStopCmd();
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

void AiE1608::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserIn()->terminate();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

void AiE1608::setScanConfig(int chanCount, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(chanCount, rate, options));
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);
	mScanConfig.options = getOptionsCode(options);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;
}

unsigned char AiE1608::getOptionsCode(ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
	  struct
	  {
		 unsigned char reserved0    : 2;
		 unsigned char trigmode     : 3;
		 unsigned char reserved1	: 3;
	  };
	  unsigned char code;
	} option;
#pragma pack()

	option.code = 0;

	if (options & SO_EXTTRIGGER)
	{
		switch (mTrigCfg.type)
		{
			case TRIG_POS_EDGE:
				option.trigmode = TRIG_EDGE_RISING;
				break;
			case TRIG_NEG_EDGE:
				option.trigmode = TRIG_EDGE_FALLING;
				break;
			case TRIG_HIGH:
				option.trigmode = TRIG_LEVEL_HIGH;
				break;
			case TRIG_LOW:
				option.trigmode = TRIG_LEVEL_LOW;
				break;
			default:
				throw UlException(ERR_BAD_TRIG_TYPE);
		}
	}

	return option.code;
}

void AiE1608::loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const
{
#pragma pack(1)
	struct
	{
		unsigned char size;
		struct
		{
			unsigned char chan;
			unsigned char range;
		}element[8];
	}queue;
#pragma pack()

	unsigned int idx = 0;

	memset(&queue, 0, sizeof(queue));

	if(!queueEnabled)
	{
		int rangeCode = mapRangeCode(range);

		for(int chan = lowChan; chan <= highChan; chan++)
		{
			queue.element[idx].chan = getChanCode(chan, inputMode);
			queue.element[idx].range = rangeCode;

			idx++;
		}
	}
	else
	{
		for (idx = 0; idx < mAQueue.size(); idx++)
		{
			queue.element[idx].chan = getChanCode(mAQueue.at(idx).channel, mAQueue.at(idx).inputMode);
			queue.element[idx].range = mapRangeCode(mAQueue.at(idx).range);
		}
	}

	queue.size = idx;

	unsigned short bufLen = (queue.size * 2) + 1;

	daqDev().queryCmd(CMD_AIQUEUE_W, (unsigned char*) &queue, bufLen);
}


unsigned int AiE1608::calcPacerPeriod(int chanCount, double rate, ScanOption options)
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

int AiE1608::mapRangeCode(Range range) const
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

int AiE1608::getChanCode(int chan, AiInputMode inputMode) const
{
	int chanCode = chan;

	if(inputMode == AI_DIFFERENTIAL)
		chanCode += 8;

	return chanCode;
}

int AiE1608::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
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

	if(inputMode == AI_SINGLE_ENDED)
		calCoefIndex += 4;

	return calCoefIndex;
}

void AiE1608::addSupportedRanges()
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

void AiE1608::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 8);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);
}

} /* namespace ul */
