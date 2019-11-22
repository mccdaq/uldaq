/*
 * AoUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb1808.h"
#include "./../daqo/DaqOUsb1808.h"
#include "./../../DaqODevice.h"

namespace ul
{
AoUsb1808::AoUsb1808(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutScanFlags(AOUTSCAN_FF_NOSCALEDATA | AOUTSCAN_FF_NOCALIBRATEDATA);

	mAoInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mAoInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mAoInfo.hasPacer(true);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(16);
	mAoInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1808X)
	{
		mAoInfo.setMaxScanRate(500000);
		mAoInfo.setMaxThroughput(numChans * 500000);
	}
	else
	{
		mAoInfo.setMaxScanRate(125000);
		mAoInfo.setMaxThroughput(numChans * 125000);
	}

	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefsStartAddr(0x7100);
	mAoInfo.setCalDateAddr(0x7110);
	mAoInfo.setCalCoefCount(numChans);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(BIP10VOLTS);
}

AoUsb1808::~AoUsb1808()
{
}

void AoUsb1808::initialize()
{
	try
	{
		//sendStopCmd(); no need for this, daqo subsystem sends the stop command

		loadDacCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}


void AoUsb1808::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	UlLock lock(mIoDeviceMutex);

	check_AOut_Args(channel, range, flags, dataValue);

	unsigned short calData = calibrateData(channel, range, flags, dataValue);

	daqDev().sendCmd(CMD_AOUT, calData, channel, NULL, 0);
}

double AoUsb1808::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutScan_Args(lowChan, highChan, range, samplesPerChan, rate, options, flags, data);

	double actualRate = 0;

	DaqOUsb1808* daqODev = dynamic_cast<DaqOUsb1808*>(mDaqDevice.daqODevice());

	if(daqODev)
	{
		int numChans = highChan - lowChan + 1;

		DaqOutChanDescriptor* chanDescriptors = new DaqOutChanDescriptor[numChans];

		for(int i = 0; i < numChans; i++)
		{
			chanDescriptors[i].type = DAQO_ANALOG;
			chanDescriptors[i].channel = lowChan + i;
			chanDescriptors[i].range = range;
		}

		actualRate =  daqODev->daqOutScan(FT_AO, chanDescriptors, numChans, samplesPerChan, rate, options, (DaqOutScanFlag) flags, data);

		delete [] chanDescriptors;
	}

	return actualRate;
}

int AoUsb1808::getCalCoefIndex(int channel, Range range) const
{
	int calCoefIndex = channel;

	return calCoefIndex;
}

CalCoef AoUsb1808::getChanCalCoef(int channel, long long flags) const
{
	Range range = BIP10VOLTS;
	return getCalCoef(channel, range, flags);
}

UlError AoUsb1808::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return mDaqDevice.daqODevice()->getStatus(FT_AO, status, xferStatus);
}

UlError AoUsb1808::waitUntilDone(double timeout)
{
	return mDaqDevice.daqODevice()->waitUntilDone(FT_AO, timeout);
}


void AoUsb1808::stopBackground()
{
	mDaqDevice.daqODevice()->stopBackground(FT_AO);
}

ScanStatus AoUsb1808::getScanState() const
{
	return mDaqDevice.daqODevice()->getScanState();
}



} /* namespace ul */
