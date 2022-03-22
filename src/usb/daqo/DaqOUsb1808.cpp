/*
 * DaqOUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqOUsb1808.h"
#include "./../ao/AoUsb1808.h"


namespace ul
{
DaqOUsb1808::DaqOUsb1808(const UsbDaqDevice& daqDevice) : DaqOUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mDaqOInfo.setDaqOutScanFlags(DAQOUTSCAN_FF_NOSCALEDATA | DAQOUTSCAN_FF_NOCALIBRATEDATA);
	mDaqOInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mDaqOInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mDaqOInfo.setChannelTypes(DAQO_ANALOG| DAQO_DIGITAL);

	mDaqOInfo.setMaxBurstRate(0);
	mDaqOInfo.setMaxBurstThroughput(0);
	mDaqOInfo.setFifoSize(FIFO_SIZE);

	mDaqOInfo.setMaxQueueLength(3);

	mDaqOInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1808X)
	{
		mDaqOInfo.setMaxScanRate(500000);
		mDaqOInfo.setMaxThroughput(mDaqOInfo.getMaxQueueLength() * 500000);
	}
	else
	{
		mDaqOInfo.setMaxScanRate(125000);
		mDaqOInfo.setMaxThroughput(mDaqOInfo.getMaxQueueLength() * 125000);
	}

	setScanEndpointAddr(0x02);

	setScanStopCmd(CMD_OUT_SCAN_STOP);

	mPatternTrig = false;
	mRetrigCount = 0;

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

DaqOUsb1808::~DaqOUsb1808()
{

}

double DaqOUsb1808::daqOutScan(FunctionType functionType, DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, void* data)
{
	check_DaqOutScan_Args(chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	AoUsb1808* aoDev = dynamic_cast<AoUsb1808*>(mDaqDevice.aoDevice());

	if(aoDev)
	{
		int sampleSize = 2;
		int aoBitness = aoDev->getAoInfo().getResolution();
		int chanCount = numChans;
		int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan, sampleSize);

		std::vector<CalCoef> calCoefs = getScanCalCoefs(chanDescriptors, numChans, flags);

		daqDev().setupTrigger(functionType, options);

		loadScanConfigs(chanDescriptors, numChans);

		daqDev().sendCmd(CMD_SCAN_CLEARFIFO);

		setChanDescriptors(chanDescriptors, chanCount);

		setScanInfo(functionType, chanCount, samplesPerChan, sampleSize, aoBitness, options, flags, calCoefs, data);

		setScanConfig(functionType, chanCount, samplesPerChan, rate, options, flags);

		daqDev().scanTranserOut()->initilizeTransfers(this, epAddr, stageSize);

		try
		{
			daqDev().sendCmd(CMD_OUT_SCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

			setScanState(SS_RUNNING);
		}
		catch(UlException& e)
		{
			stopBackground();
			throw e;
		}
	}

	return actualScanRate();

}

void DaqOUsb1808::setScanConfig(FunctionType functionType, int chanCount, unsigned int scanCount, double rate, ScanOption options, DaqOutScanFlag flags)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(functionType, options, flags);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	TriggerConfig trigCfg = daqDev().getTriggerConfig(functionType);

	if(options & SO_RETRIGGER)
	{
		if(trigCfg.retrigCount == 0)
			mScanConfig.retrig_count = scanCount;
		else
		{
			if(!(options & SO_CONTINUOUS))
				mScanConfig.retrig_count = trigCfg.retrigCount > scanCount ? scanCount : trigCfg.retrigCount;
			else
			{
				// different behavior from the UL for windows in continuous mode
				mScanConfig.retrig_count = trigCfg.retrigCount;
			}
		}
	}
}

unsigned char DaqOUsb1808::getOptionsCode(FunctionType functionType, ScanOption options, DaqOutScanFlag flags) const
{
#pragma pack(1)
	union TOptionsCode
	{
		struct
		{
			unsigned char ext_trigger		 : 1;
			unsigned char pattern_trigger    : 1;
			unsigned char retrigger			 : 1;
			unsigned char reserved0			 : 1;
			unsigned char reserved1			 : 1;
			unsigned char reserved2			 : 1;
			unsigned char reserved3			 : 1;
			unsigned char reserved4			 : 1;
		};
		unsigned char code;
	} option;
#pragma pack()

	TriggerConfig trigCfg = daqDev().getTriggerConfig(functionType);

	option.code = 0;

	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		option.ext_trigger = 1;

		if (options & SO_RETRIGGER)
			option.retrigger = 1;

		if(trigCfg.type & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW))
		{
			option.ext_trigger = 0;
			option.pattern_trigger = 1;
		}
	}

	return option.code;
}

void DaqOUsb1808::loadScanConfigs(DaqOutChanDescriptor chanDescriptors[], int numChans) const
{
   if(numChans <= 3)
   {
		unsigned char scanQueue[3];

		memset(scanQueue, 0, sizeof(scanQueue));

		int idx = 0;

		for(idx = 0; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQO_ANALOG)
			{
				scanQueue[idx] = chanDescriptors[idx].channel;
			}
			else if (chanDescriptors[idx].type == DAQO_DIGITAL)
				scanQueue[idx] = 2;
		}

		daqDev().sendCmd(CMD_OUT_SCAN_CONFIG, 0, idx - 1, (unsigned char*) &scanQueue, sizeof(scanQueue));
	}
}


std::vector<CalCoef> DaqOUsb1808::getScanCalCoefs(DaqOutChanDescriptor chanDescriptors[], int numChans, DaqOutScanFlag flags) const
{
	std::vector<CalCoef> calCoefs;

	CalCoef calCoef;

	const AoUsb1808* aoDev = dynamic_cast<AoUsb1808*>(mDaqDevice.aoDevice());

	if(aoDev)
	{
		for(int idx = 0; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQO_ANALOG)
			{
				calCoef = aoDev->getChanCalCoef(chanDescriptors[idx].channel, flags);
			}
			else
			{
				calCoef.slope = 1;
				calCoef.offset = 0;
			}

			calCoefs.push_back(calCoef);
		}
	}

	return calCoefs;
}

} /* namespace ul */
