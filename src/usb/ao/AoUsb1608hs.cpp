/*
 * AoUsb1608hs.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AoUsb1608hs.h"

namespace ul
{
AoUsb1608hs::AoUsb1608hs(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	double minRate = 0.59604644775390625;

	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutScanFlags(AOUTSCAN_FF_NOSCALEDATA | AOUTSCAN_FF_NOCALIBRATEDATA);

	mAoInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_BLOCKIO); // single i/o is not supported

	mAoInfo.hasPacer(true);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(16);
	mAoInfo.setMinScanRate(minRate);
	mAoInfo.setMaxScanRate(76000);
	mAoInfo.setMaxThroughput(76000);
	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefsStartAddr(0);
	mAoInfo.setCalDateAddr(0);
	mAoInfo.setCalCoefCount(0);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(BIP10VOLTS);

	setScanEndpointAddr(0x01);

	setScanStopCmd(CMD_AOUTSTOP);

	memset(&mScanConfig, 0, sizeof(mScanConfig));
	memset(mAOutVals, 0, sizeof(mAOutVals));
}

AoUsb1608hs::~AoUsb1608hs()
{
}

void AoUsb1608hs::initialize()
{
	try
	{
		sendStopCmd();

		readAOutVals();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AoUsb1608hs::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	UlLock lock(mIoDeviceMutex);

	check_AOut_Args(channel, range, flags, dataValue);

	unsigned short calData = calibrateData(channel, range, flags, dataValue);

	unsigned short vals[2];
	memcpy(vals, mAOutVals, sizeof(mAOutVals));

	vals[channel] = calData;

	daqDev().sendCmd(CMD_AOUT, 0, 0, (unsigned char*) vals, sizeof(vals));

	mAOutVals[channel] = vals[channel];
}

double AoUsb1608hs::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutScan_Args(lowChan, highChan, range, samplesPerChan, rate, options, flags, data);

	int epAddr = getScanEndpointAddr();

	//setTransferMode(options, rate);
	mTransferMode = SO_BLOCKIO;

	int chanCount = highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, range, flags);

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AO, chanCount, samplesPerChan, mAoInfo.getSampleSize(), mAoInfo.getResolution(), options, flags, calCoefs, data);

	setScanConfig(lowChan, highChan, samplesPerChan, rate, options);

	daqDev().scanTranserOut()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_AOUTSCAN_START, 0, 0, NULL, 0, 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

void AoUsb1608hs::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	calcPacerParams(rate, mScanConfig.prescale, mScanConfig.preload);

	mScanConfig.preload = Endian::cpu_to_le_ui16(mScanConfig.preload);
	mScanConfig.options = getOptionsCode(lowChan, highChan, options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	daqDev().sendCmd(CMD_AOUTSCAN_CONFIG, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);
}

void AoUsb1608hs::calcPacerParams(double rate, unsigned char& prescale, unsigned short& preload)
{
	long clk = CLK_FREQ;

	double freq= rate;
	double pl;
	double ps = 1;
	int ps_index = 0;

	pl = ((double) clk  / (freq * ps)) - 1;

	while(pl > 65535.0)
	{
		ps *= 2;
		ps_index++;
		pl = ((double) clk  / (freq * ps)) - 1;
	}

	if (ps_index > 8)
	{
		ps = 256;
		ps_index = 8;
		pl = ((double) clk  / (freq * ps)) - 1;
	}
	else
		prescale = ps_index;

	if (pl < 0.0)
		preload = 0;
	else if (pl > 65535.0)
		preload = 65535;
	else
		preload = (unsigned short) pl;

   double actualrate = clk;

   actualrate /= ps;
   actualrate /= preload + 1;

   setActualScanRate(actualrate);
}

unsigned char AoUsb1608hs::getOptionsCode(int lowChan, int highChan, ScanOption options) const
{

	unsigned char optcode = 0;

	unsigned char channelMask = getChannelMask(lowChan, highChan);

	optcode = channelMask;

	return optcode;
}

unsigned char AoUsb1608hs::getChannelMask(int lowChan, int highChan) const
{
	unsigned char chanMask = 0;

	for(int chan = lowChan; chan <= highChan; chan++)
		chanMask |= (unsigned char) (0x01 << chan);

	return chanMask;
}


int AoUsb1608hs::getCalCoefIndex(int channel, Range range) const
{
	int calCoefIndex = channel;

	return calCoefIndex;
}

void AoUsb1608hs::readAOutVals()
{
	UlLock lock(mIoDeviceMutex);

	daqDev().queryCmd(CMD_AOUT, 0, 0, (unsigned char*) mAOutVals, sizeof(mAOutVals));
}

UlError AoUsb1608hs::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned char cmd = daqDev().getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned char status =0;

	try
	{
		daqDev().queryCmd(cmd, 0, 0, (unsigned char*)&status, sizeof(status));

		if((status & daqDev().getScanDoneBitMask()) || !(status & daqDev().getScanRunningBitMask(SD_OUTPUT)))
					*scanDone = true;

		if(status & daqDev().getUnderrunBitMask())
		{
			err = ERR_UNDERRUN;
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

void AoUsb1608hs::setCfg_SenseMode(int channel, AOutSenseMode mode)
{
	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	UlLock lock(mIoDeviceMutex); // acquire the lock so ALREADY_ACTIVE error can be detected correctly on multi-threaded processes

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(channel < 0 || channel >= mAoInfo.getNumChans())
		throw UlException(ERR_BAD_AO_CHAN);

	unsigned char mask = 0;

	daqDev().queryCmd(CMD_AOUT_CONFIG, 0, 0, (unsigned char*) &mask, sizeof(mask));

	std::bitset<8> config = mask;

	if(mode == AOSM_ENABLED)
		config.set(channel);
	else
		config.reset(channel);

	mask = (unsigned char) config.to_ulong();

	daqDev().sendCmd(CMD_AOUT_CONFIG, 0, 0, (unsigned char*) &mask, sizeof(mask));
}

AOutSenseMode AoUsb1608hs::getCfg_SenseMode(int channel) const
{
	AOutSenseMode mode = AOSM_DISABLED;

	if(!daqDev().isConnected())
		throw UlException(ERR_DEV_NOT_CONNECTED);

	UlLock lock(const_cast<AoUsb1608hs*>(this)->mIoDeviceMutex); // acquire the lock so ALREADY_ACTIVE error can be detected correctly on multi-threaded processes

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(channel < 0 || channel >= mAoInfo.getNumChans())
		throw UlException(ERR_BAD_AO_CHAN);

	unsigned char mask = 0;

	daqDev().queryCmd(CMD_AOUT_CONFIG, 0, 0, (unsigned char*) &mask, sizeof(mask));

	std::bitset<8> config = mask;

	if(config[channel])
		 mode = AOSM_ENABLED;

	return mode;
}

} /* namespace ul */
