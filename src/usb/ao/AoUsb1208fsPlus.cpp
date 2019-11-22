/*
 * AoUsb1208fsPlus.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb1208fsPlus.h"

namespace ul
{

AoUsb1208fs_Plus::AoUsb1208fs_Plus(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutScanFlags(AOUTSCAN_FF_NOSCALEDATA | AOUTSCAN_FF_NOCALIBRATEDATA);

	mAoInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_SINGLEIO |SO_BLOCKIO);
	mAoInfo.setTriggerTypes(TRIG_NONE);

	mAoInfo.hasPacer(true);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(12);
	mAoInfo.setMinScanRate(minRate);
	mAoInfo.setMaxScanRate(50000);
	mAoInfo.setMaxThroughput(50000 * numChans);
	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefsStartAddr(0);
	mAoInfo.setCalDateAddr(0);
	mAoInfo.setCalCoefCount(0);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(UNI5VOLTS);

	setScanEndpointAddr(0x02);

	setScanStopCmd(CMD_AOUTSTOP);

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

AoUsb1208fs_Plus::~AoUsb1208fs_Plus()
{
}

void AoUsb1208fs_Plus::initialize()
{
	try
	{
		sendStopCmd();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}


void AoUsb1208fs_Plus::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	UlLock lock(mIoDeviceMutex);

	check_AOut_Args(channel, range, flags, dataValue);

	unsigned short calData = calibrateData(channel, range, flags, dataValue);

	daqDev().sendCmd(CMD_AOUT, calData, channel, NULL, 0);
}

double AoUsb1208fs_Plus::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutScan_Args(lowChan, highChan, range, samplesPerChan, rate, options, flags, data);

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	int chanCount = highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, range, flags);

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AO, chanCount, samplesPerChan, mAoInfo.getSampleSize(), mAoInfo.getResolution(), options, flags, calCoefs, data);

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

void AoUsb1208fs_Plus::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(lowChan, highChan, options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;
}

unsigned char AoUsb1208fs_Plus::getOptionsCode(int lowChan, int highChan, ScanOption options) const
{

	unsigned char optcode = 0;

	unsigned char channelMask = getChannelMask(lowChan, highChan);

	optcode = channelMask;

	return optcode;
}

unsigned char AoUsb1208fs_Plus::getChannelMask(int lowChan, int highChan) const
{
	unsigned char chanMask = 0;

	for(int chan = lowChan; chan <= highChan; chan++)
		chanMask |= (unsigned char) (0x01 << chan);

	return chanMask;
}


int AoUsb1208fs_Plus::getCalCoefIndex(int channel, Range range) const
{
	int calCoefIndex = channel;

	return calCoefIndex;
}


} /* namespace ul */
