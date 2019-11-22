/*
 * DaqIUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqIUsb1808.h"
#include "../ai/AiUsb1808.h"
#include "../../DioDevice.h"
#include "../../CtrDevice.h"

namespace ul
{

DaqIUsb1808::DaqIUsb1808(const UsbDaqDevice& daqDevice) : DaqIUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mDaqIInfo.setDaqInScanFlags(DAQINSCAN_FF_NOSCALEDATA | DAQINSCAN_FF_NOCALIBRATEDATA | DAQINSCAN_FF_NOCLEAR);
	mDaqIInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mDaqIInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mDaqIInfo.setChannelTypes(DAQI_ANALOG_DIFF | DAQI_ANALOG_SE | DAQI_DIGITAL | DAQI_CTR32);


	mDaqIInfo.setMaxBurstRate(0);
	mDaqIInfo.setMaxBurstThroughput(0);
	mDaqIInfo.setFifoSize(FIFO_SIZE);

	mDaqIInfo.setMaxQueueLength(13);
	mDaqIInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1808X)
	{
		mDaqIInfo.setMaxScanRate(200000);
		mDaqIInfo.setMaxThroughput(mDaqIInfo.getMaxQueueLength() * 200000);
	}
	else
	{
		mDaqIInfo.setMaxScanRate(50000);
		mDaqIInfo.setMaxThroughput(mDaqIInfo.getMaxQueueLength() * 50000);
	}

	setScanEndpointAddr(0x86);

	setScanStopCmd(CMD_IN_SCAN_STOP);

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

DaqIUsb1808::~DaqIUsb1808()
{

}

double DaqIUsb1808::daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data)
{
	UlLock lock(mIoDeviceMutex);

	check_DaqInScan_Args(chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	AiUsb1808* aiDev = dynamic_cast<AiUsb1808*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		int sampleSize = 4;
		int aiResolution = aiDev->getAiInfo().getResolution();
		int chanCount = numChans;
		int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan, sampleSize);

		std::vector<CalCoef> calCoefs = getScanCalCoefs(chanDescriptors, numChans, flags);
		std::vector<CustomScale> customScales = getCustomScales(chanDescriptors, numChans);

		daqDev().setupTrigger(functionType, options);

		loadScanConfigs(chanDescriptors, numChans);

		daqDev().clearHalt(epAddr);

		daqDev().sendCmd(CMD_SCAN_CLEARFIFO);

		setScanInfo(functionType, chanCount, samplesPerChan, sampleSize, aiResolution, options, flags, calCoefs, customScales, data);

		setScanConfig(functionType, chanCount, samplesPerChan, rate, options, flags);

		daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

		try
		{
			daqDev().sendCmd(CMD_IN_SCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

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


void DaqIUsb1808::setScanConfig(FunctionType functionType, int chanCount, unsigned int scanCount, double rate, ScanOption options, DaqInScanFlag flags)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(functionType, options, flags);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	int epAddr = getScanEndpointAddr();
	mScanConfig.packet_size = (getTransferMode() == SO_BLOCKIO) ? daqDev().getBulkEndpointMaxPacketSize(epAddr)/2 - 1 : chanCount - 1;

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

unsigned char DaqIUsb1808::getOptionsCode(FunctionType functionType, ScanOption options, DaqInScanFlag flags) const
{
#pragma pack(1)
	union TOptionsCode
	{
		struct
		{
			unsigned char ext_trigger		 : 1;
			unsigned char pattern_trigger    : 1;
			unsigned char retrigger			 : 1;
			unsigned char noclear			 : 1;
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

	if((flags & DAQINSCAN_FF_NOCLEAR))
	{
		option.noclear = 1;
	}

	return option.code;
}


void DaqIUsb1808::loadScanConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const
{
	unsigned char scanQueue[13];

	memset(scanQueue, 0, sizeof(scanQueue));

	DaqInChanDescriptor aichanDescs[8];
	unsigned int aiChanCount = 0;

	int idx = 0;

	for(idx = 0; idx < numChans; idx++)
	{
		if(chanDescriptors[idx].type == DAQI_ANALOG_SE || chanDescriptors[idx].type == DAQI_ANALOG_DIFF)
		{
			scanQueue[idx] = chanDescriptors[idx].channel;

			memcpy(&aichanDescs[aiChanCount], &chanDescriptors[idx], sizeof(DaqInChanDescriptor));

			aiChanCount++;
		}
		else if (chanDescriptors[idx].type == DAQI_DIGITAL)
			scanQueue[idx] = 8;
		else if (chanDescriptors[idx].type == DAQI_CTR32)
		{
			scanQueue[idx] = 9 + chanDescriptors[idx].channel;

			mDaqDevice.ctrDevice()->setScanCounterActive(chanDescriptors[idx].channel);
		}
	}

	if(aiChanCount > 0)
	{
		const AiUsb1808* aiDev = dynamic_cast<AiUsb1808*>(mDaqDevice.aiDevice());

		if(aiDev)
			aiDev->loadAInConfigs(aichanDescs, aiChanCount);
	}

	daqDev().sendCmd(CMD_IN_SCAN_CONFIG, 0, idx - 1, (unsigned char*) &scanQueue, sizeof(scanQueue));
}




std::vector<CalCoef> DaqIUsb1808::getScanCalCoefs(DaqInChanDescriptor chanDescriptors[], int numChans, DaqInScanFlag flags) const
{
	std::vector<CalCoef> calCoefs;

	CalCoef calCoef;
	AiInputMode inputMode;

	const AiUsb1808* aiDev = dynamic_cast<AiUsb1808*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		for(int idx = 0; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQI_ANALOG_SE || chanDescriptors[idx].type == DAQI_ANALOG_DIFF)
			{
				inputMode = chanDescriptors[idx].type == DAQI_ANALOG_SE ? AI_SINGLE_ENDED : AI_DIFFERENTIAL;

				calCoef = aiDev->getChanCalCoef(chanDescriptors[idx].channel, inputMode, chanDescriptors[idx].range, flags);
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

std::vector<CustomScale> DaqIUsb1808::getCustomScales(DaqInChanDescriptor chanDescriptors[], int numChans) const
{
	std::vector<CustomScale> customScales;

	CustomScale customScale;

	const AiUsb1808* aiDev = dynamic_cast<AiUsb1808*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		for(int idx = 0; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQI_ANALOG_SE || chanDescriptors[idx].type == DAQI_ANALOG_DIFF)
			{
				customScale = aiDev->getChanCustomScale(chanDescriptors[idx].channel);
			}
			else
			{
				customScale.slope = 1;
				customScale.offset = 0;
			}

			customScales.push_back(customScale);
		}
	}

	return customScales;
}


} /* namespace ul */
