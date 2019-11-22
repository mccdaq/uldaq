/*
 * AiUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsb1808.h"
#include "../daqi/DaqIUsb1808.h"

namespace ul
{

AiUsb1808::AiUsb1808(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
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

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1808X)
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

	initCustomScales();

	memset(mAdcConfig, 0, sizeof(mAdcConfig));

	for(int chan = 0; chan < mAiInfo.getNumChans(); chan++)
	{
		mAdcConfig[chan].mode = GROUND_MODE;
		mAdcConfig[chan].range = mapRangeCode(BIP10VOLTS);
	}
}

AiUsb1808::~AiUsb1808()
{

}

void AiUsb1808::initialize()
{
	try
	{
		//sendStopCmd(); no need for this, daqi subsystem sends the stop command

		loadAdcCoefficients();

		writeAInConfigs();

		//do one ain because if this is the first time after the FPGA is loaded the first ain sample is garbage.
		//aIn(0, AI_DIFFERENTIAL, BIP10VOLTS, 0);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AiUsb1808::disconnect()
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

double AiUsb1808::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	unsigned int rawVal;
	unsigned int rawVals[8];

	TADCCONFIG config;
	config.mask = 0;
	config.mode = (inputMode == AI_SINGLE_ENDED) ? SE_MODE : DIFF_MODE;
	config.range = mapRangeCode(range);

	if(config.mask != mAdcConfig[channel].mask)
		loadAInConfig(channel, inputMode, range);

	daqDev().queryCmd(CMD_AIN, channel, 0, (unsigned char*) &rawVals, sizeof(rawVals));

	rawVal = Endian::le_ui32_to_cpu(rawVals[channel]);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	return data;
}

double AiUsb1808::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsb1808* daqIDev = dynamic_cast<DaqIUsb1808*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		int numChans = (queueEnabled() ? queueLength() : highChan - lowChan + 1);

		DaqInChanDescriptor* chanDescriptors = new DaqInChanDescriptor[numChans];

		for(int i = 0; i < numChans; i++)
		{
			if(queueEnabled())
			{
				chanDescriptors[i].channel = mAQueue[i].channel;
				chanDescriptors[i].type = (mAQueue[i].inputMode == AI_DIFFERENTIAL) ? DAQI_ANALOG_DIFF : DAQI_ANALOG_SE;
				chanDescriptors[i].range = mAQueue[i].range;
			}
			else
			{
				chanDescriptors[i].channel = lowChan + i;
				chanDescriptors[i].type = (inputMode == AI_DIFFERENTIAL) ? DAQI_ANALOG_DIFF : DAQI_ANALOG_SE;
				chanDescriptors[i].range = range;
			}
		}

		actualRate =  daqIDev->daqInScan(FT_AI, chanDescriptors, numChans, samplesPerChan, rate, options, (DaqInScanFlag) flags, data);

		delete [] chanDescriptors;
	}

	return actualRate;
}

void AiUsb1808::writeAInConfigs() const
{
	daqDev().sendCmd(CMD_ADC_SETUP, 0, 0, (unsigned char*) mAdcConfig, sizeof(mAdcConfig));
}

void AiUsb1808::resetAInConfigs() const
{
	for(int chan = 0; chan < mAiInfo.getNumChans(); chan++)
		mAdcConfig[chan].mode = GROUND_MODE;

	writeAInConfigs();
}


void AiUsb1808::loadAInConfig(int chan, AiInputMode inputMode, Range range) const
{
	if (chan < mAiInfo.getNumChans())
	{
		if(inputMode == AI_SINGLE_ENDED)
			mAdcConfig[chan].mode = SE_MODE;
		else if(inputMode == AI_DIFFERENTIAL)
			mAdcConfig[chan].mode = DIFF_MODE;

		mAdcConfig[chan].range = mapRangeCode(range);

		writeAInConfigs();
	}
}

void AiUsb1808::loadAInConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const
{
	int chan;

	for (chan = 0; chan < mAiInfo.getNumChans(); chan++)
		mAdcConfig[chan].mode = GROUND_MODE;

	for(int idx = 0; idx < numChans; ++idx)
	{
		chan = chanDescriptors[idx].channel;

		if(chanDescriptors[idx].type == DAQI_ANALOG_SE)
			mAdcConfig[chan].mode = SE_MODE;
		else if(chanDescriptors[idx].type == DAQI_ANALOG_DIFF)
			mAdcConfig[chan].mode = DIFF_MODE;

		mAdcConfig[chan].range = mapRangeCode(chanDescriptors[idx].range);
	}

	writeAInConfigs();
}

CalCoef AiUsb1808::getChanCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const
{
	return getCalCoef(channel, inputMode, range, flags);
}

CustomScale AiUsb1808::getChanCustomScale(int channel) const
{
	return mCustomScales[channel];
}

int AiUsb1808::mapRangeCode(Range range) const
{
	int rangeCode;

	switch(range)
	{
	case BIP10VOLTS:
		rangeCode = 0x00;
		break;

	case BIP5VOLTS:
		rangeCode = 0x01;
		break;

	case UNI10VOLTS:
		rangeCode = 0x02;
		break;

	case UNI5VOLTS:
		rangeCode = 0x03;
		break;

	default:
		throw UlException(ERR_BAD_RANGE);
	}

	return rangeCode;
}

int AiUsb1808::getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const
{
	int calCoefIndex;

	int rangeCode = mapRangeCode(range);

	calCoefIndex = (channel * 4) + rangeCode;

	return calCoefIndex;
}

UlError AiUsb1808::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return mDaqDevice.daqIDevice()->getStatus(FT_AI, status, xferStatus);
}

UlError AiUsb1808::waitUntilDone(double timeout)
{
	return mDaqDevice.daqIDevice()->waitUntilDone(FT_AI, timeout);
}


void AiUsb1808::stopBackground()
{
	mDaqDevice.daqIDevice()->stopBackground(FT_AI);
}

ScanStatus AiUsb1808::getScanState() const
{
	return mDaqDevice.daqIDevice()->getScanState();
}




void AiUsb1808::addSupportedRanges()
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

void AiUsb1808::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 8);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 8);

	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE | MODE_QUEUE);

	// Add queue limitations
	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN);
}

} /* namespace ul */
