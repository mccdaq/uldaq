/*
 * AoUsb1208hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb1208hs.h"
#include "../UsbScanTransferOut.h"

namespace ul
{

AoUsb1208hs::AoUsb1208hs(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutScanFlags(AOUTSCAN_FF_NOSCALEDATA | AOUTSCAN_FF_NOCALIBRATEDATA);

	mAoInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mAoInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mAoInfo.hasPacer(true);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(12);
	mAoInfo.setMinScanRate(minRate);
	mAoInfo.setMaxScanRate(1000000);
	mAoInfo.setMaxThroughput(1000000 * numChans);
	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefsStartAddr(0x4080);
	mAoInfo.setCalDateAddr(0x7098);
	mAoInfo.setCalCoefCount(numChans);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(BIP10VOLTS);

	setScanEndpointAddr(0x02);

	setScanStopCmd(CMD_AOUTSTOP);

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

AoUsb1208hs::~AoUsb1208hs()
{
}

void AoUsb1208hs::initialize()
{
	try
	{
		sendStopCmd();

		loadDacCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}


void AoUsb1208hs::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	UlLock lock(mIoDeviceMutex);

	check_AOut_Args(channel, range, flags, dataValue);

	unsigned short calData = calibrateData(channel, range, flags, dataValue);

	daqDev().sendCmd(CMD_AOUT, calData, channel, NULL, 0);
}

double AoUsb1208hs::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutScan_Args(lowChan, highChan, range, samplesPerChan, rate, options, flags, data);

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	int chanCount = highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, range, flags);

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AO, chanCount, samplesPerChan, mAoInfo.getSampleSize(), mAoInfo.getResolution(), options, flags, calCoefs, data);

	daqDev().setupTrigger(FT_AO, options);

	setScanConfig(lowChan, highChan, samplesPerChan, rate, options);

	daqDev().sendCmd(CMD_AOUTSCAN_CLEAR_FIFO);

	daqDev().scanTranserOut()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_AOUTSCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

		return actualScanRate();
}

void AoUsb1208hs::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(lowChan, highChan, options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

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

unsigned char AoUsb1208hs::getOptionsCode(int lowChan, int highChan, ScanOption options) const
{

	unsigned char optcode = 0;

	unsigned char channelMask = getChannelMask(lowChan, highChan);

	optcode = channelMask;

	if (options & SO_RETRIGGER)
	{
		optcode |= 1 << 4; 		// enable trigger
		optcode |= 1 << 5;      // enable retrigger
	}
	else if(options & SO_EXTTRIGGER)
	{
		optcode |= 1 << 4;      // enable trigger
	}

	return optcode;
}

unsigned char AoUsb1208hs::getChannelMask(int lowChan, int highChan) const
{
	unsigned char chanMask = 0;

	for(int chan = lowChan; chan <= highChan; chan++)
		chanMask |= (unsigned char) (0x01 << chan);

	return chanMask;
}


int AoUsb1208hs::getCalCoefIndex(int channel, Range range) const
{
	int calCoefIndex = channel;

	return calCoefIndex;
}


} /* namespace ul */
