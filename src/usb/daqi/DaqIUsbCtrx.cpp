/*
 * DaqIUsbCtrx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqIUsbCtrx.h"
#include "../../DioDevice.h"
#include "../../CtrDevice.h"

namespace ul
{

DaqIUsbCtrx::DaqIUsbCtrx(const UsbDaqDevice& daqDevice) : DaqIUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mDaqIInfo.setDaqInScanFlags(DAQINSCAN_FF_NOCLEAR);
	mDaqIInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_EXTCLOCK | SO_SINGLEIO | SO_BLOCKIO | SO_RETRIGGER);
	mDaqIInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mDaqIInfo.setChannelTypes(DAQI_DIGITAL | DAQI_CTR16 | DAQI_CTR32 | DAQI_CTR48);

	mInternalChanTypes = mDaqIInfo.getChannelTypes() | DAQI_CTR64_INTERNAL;

	mDaqIInfo.setMaxBurstRate(0);
	mDaqIInfo.setMaxBurstThroughput(0);
	mDaqIInfo.setFifoSize(FIFO_SIZE);

	mDaqIInfo.setMaxQueueLength(9);

	mDaqIInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_CTR04)
	{
		mDaqIInfo.setMaxQueueLength(5);
	}
	else
	{
		mDaqIInfo.setMaxQueueLength(9);
	}

	mDaqIInfo.setMaxScanRate(4000000);
	mDaqIInfo.setMaxThroughput(4000000);

	setScanEndpointAddr(0x86);

	setScanStopCmd(CMD_SCAN_STOP);
}

DaqIUsbCtrx::~DaqIUsbCtrx()
{

}

double DaqIUsbCtrx::daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data)
{
	UlLock lock(mIoDeviceMutex);

	check_DaqInScan_Args(chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);

	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	int sampleSize = calcMaxSampleSize(chanDescriptors, numChans);
	int analogResolution = 0;
	int chanCount = numChans;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan, sampleSize);

	std::vector<CalCoef> calCoefs;
	std::vector<CustomScale> customScales;

	if(functionType == FT_DAQI)
	{
		CalCoef calCoef = {1, 0};
		CustomScale customScale = {1, 0};

		for(int idx = 0; idx < numChans; idx++)
		{
			calCoefs.push_back(calCoef);
			customScales.push_back(customScale);
		}
	}

	daqDev().setupTrigger(functionType, options);

	loadScanConfigs(chanDescriptors, numChans);

	daqDev().clearHalt(epAddr);

	daqDev().sendCmd(CMD_SCAN_CLEARFIFO);

	setScanInfo(functionType, chanCount, samplesPerChan, sampleSize, analogResolution, options, flags, calCoefs, customScales, data);

	setScanConfig(functionType, chanCount, samplesPerChan, sampleSize, rate, options, flags);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_SCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

int DaqIUsbCtrx::calcMaxSampleSize(DaqInChanDescriptor chanDescriptors[], int numChans) const
{
	int maxSampleSize = 2;
	int sampleSize = 2;

	for(int i = 0; i < numChans; i++)
	{
		if(chanDescriptors[i].type == DAQI_CTR32)
			sampleSize = 4;
		else if(chanDescriptors[i].type == DAQI_CTR48 || chanDescriptors[i].type == (DaqInChanType) DAQI_CTR64_INTERNAL)
			sampleSize = 8;

		if(maxSampleSize < sampleSize)
			maxSampleSize = sampleSize;
	}

	return maxSampleSize;
}


void DaqIUsbCtrx::setScanConfig(FunctionType functionType, int chanCount, unsigned int scanCount, int sampleSize, double rate, ScanOption options, DaqInScanFlag flags)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(functionType, options, flags);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	int epAddr = getScanEndpointAddr();
	mScanConfig.packet_size = (getTransferMode() == SO_BLOCKIO) ? daqDev().getBulkEndpointMaxPacketSize(epAddr)/2 - 1 : (chanCount * (sampleSize / 2)) - 1;

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

unsigned char DaqIUsbCtrx::getOptionsCode(FunctionType functionType, ScanOption options, DaqInScanFlag flags) const
{
#pragma pack(1)
	union TOptionsCode
	{
		struct
		{
			unsigned char noclear			 : 1;
			unsigned char reserved0			 : 1;
			unsigned char reserved1			 : 1;
			unsigned char ext_trigger		 : 1;
			unsigned char reserved2			 : 1;
			unsigned char reserved3			 : 1;
			unsigned char retrigger			 : 1;
			unsigned char reserved4			 : 1;
		};
		unsigned char code;
	} option;
#pragma pack()

	option.code = 0;

	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		option.ext_trigger = 1;

		if (options & SO_RETRIGGER)
			option.retrigger = 1;
	}

	if((flags & DAQINSCAN_FF_NOCLEAR))
	{
		option.noclear = 1;
	}

	return option.code;
}


void DaqIUsbCtrx::loadScanConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const
{
#pragma pack(1)
	union
	{
		struct
		{
			unsigned char chanNum		 : 3;
			unsigned char bankNum		 : 2;
			unsigned char isDigitalPort	 : 1;
			unsigned char fillZeros		 : 1;
			unsigned char reserved		 : 1;
		};
		unsigned char code;
	} scanQueue[33];
#pragma pack()

	memset(scanQueue, 0, sizeof(scanQueue));

	int sampleSize = calcMaxSampleSize(chanDescriptors, numChans);
	int sampleBankCount = sampleSize / 2;

	int idx = 0;
	int fillZeroBankCount = 0;

	int chan = 0;
	int chanBankCount = 0;
	int isDigitalPort = 0;
	int sqIdx = 0;

	for(idx = 0; idx < numChans; idx++)
	{
		chanBankCount = 0;
		isDigitalPort = 0;

		if (chanDescriptors[idx].type == DAQI_DIGITAL)
		{
			chan = 0;
			chanBankCount = 1;
			isDigitalPort = 1;
		}
		else if (chanDescriptors[idx].type == DAQI_CTR16)
		{
			chan = chanDescriptors[idx].channel;
			chanBankCount = 1;

			mDaqDevice.ctrDevice()->setScanCounterActive(chanDescriptors[idx].channel);
		}
		else if (chanDescriptors[idx].type == DAQI_CTR32)
		{
			chan = chanDescriptors[idx].channel;
			chanBankCount = 2;

			mDaqDevice.ctrDevice()->setScanCounterActive(chanDescriptors[idx].channel);
		}
		else if (chanDescriptors[idx].type == DAQI_CTR48)
		{
			chan = chanDescriptors[idx].channel;
			chanBankCount = 3;

			mDaqDevice.ctrDevice()->setScanCounterActive(chanDescriptors[idx].channel);
		}
		else if (chanDescriptors[idx].type == (DaqInChanType) DAQI_CTR64_INTERNAL)
		{
			chan = chanDescriptors[idx].channel;
			chanBankCount = 4;

			mDaqDevice.ctrDevice()->setScanCounterActive(chanDescriptors[idx].channel);
		}


		for(int bank = 0; bank < chanBankCount; bank++)
		{
			scanQueue[sqIdx].chanNum = chan;
			scanQueue[sqIdx].bankNum = bank;
			scanQueue[sqIdx].fillZeros = 0;
			scanQueue[sqIdx].isDigitalPort = isDigitalPort;

			sqIdx++;
		}

		fillZeroBankCount = sampleBankCount - chanBankCount;

		for(int i = 0; i < fillZeroBankCount; i++)
		{
			scanQueue[sqIdx].fillZeros = 1; // this bit is ignored in the FPGA, the following two lines imitate fill zero in FPGA

			scanQueue[sqIdx].isDigitalPort = 1;
			scanQueue[sqIdx].chanNum = 1;
			sqIdx++;
		}
	}


	daqDev().sendCmd(CMD_SCAN_CONFIG, 0, sqIdx - 1, (unsigned char*) &scanQueue, sizeof(scanQueue));
}

void DaqIUsbCtrx::check_DaqInScan_Args(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data) const
{
	if(chanDescriptors != NULL)
	{
		bool invalidRate = false;

		if(numChans > mDaqIInfo.getMaxQueueLength())
			throw UlException(ERR_BAD_NUM_CHANS);

		for(unsigned int i = 0; i < (unsigned int) numChans; i++)
		{
			if(mInternalChanTypes & chanDescriptors[i].type)
			{
				std::bitset<32> typeBitSet(chanDescriptors[i].type);

				if(typeBitSet.count() > 1)
					throw UlException(ERR_BAD_DAQI_CHAN_TYPE);

				if(chanDescriptors[i].type == DAQI_DIGITAL)
				{
					const DioInfo& dioInfo = (const DioInfo&) mDaqDevice.getDioDevice().getDioInfo();

					if(dioInfo.isPortSupported((DigitalPortType) chanDescriptors[i].channel) == false)
						throw UlException(ERR_BAD_PORT_TYPE);

					if(rate > dioInfo.getMaxScanRate(DD_INPUT))
						invalidRate = true;
				}
				else if(chanDescriptors[i].type == DAQI_CTR16 || chanDescriptors[i].type == DAQI_CTR32 || chanDescriptors[i].type == DAQI_CTR48 || chanDescriptors[i].type == (DaqInChanType)DAQI_CTR64_INTERNAL)
				{
					const CtrInfo& ctrInfo = (const CtrInfo&) mDaqDevice.getCtrDevice().getCtrInfo();

					if(chanDescriptors[i].channel >= ctrInfo.getNumCtrs())
						throw UlException(ERR_BAD_CTR);

					if(rate > ctrInfo.getMaxScanRate())
						invalidRate = true;
				}
			}
			else
			{
				throw UlException(ERR_BAD_DAQI_CHAN_TYPE);
			}
		}

		if(data == NULL)
			throw UlException(ERR_BAD_BUFFER);

		if(~mDaqIInfo.getScanOptions() & options)
			throw UlException(ERR_BAD_OPTION);


		int sampleSize = calcMaxSampleSize(chanDescriptors, numChans);

		double throughput = rate * numChans * (sampleSize / 2);

		if(throughput > mDaqIInfo.getMaxThroughput())
			invalidRate = true;

		if((!(options & SO_EXTCLOCK) && invalidRate) || (rate <= 0.0))
			throw UlException(ERR_BAD_RATE);

		if(samplesPerChan < mMinScanSampleCount)
			throw UlException(ERR_BAD_SAMPLE_COUNT);

		if(!mDaqDevice.isConnected())
			throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
	}
}
} /* namespace ul */
