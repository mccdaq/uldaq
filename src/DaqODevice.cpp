/*
 * DaqODevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqODevice.h"
#include "AoInfo.h"
#include "DioInfo.h"
#include "DioDevice.h"

#include <math.h>
#include <algorithm>
#include <bitset>

#include "UlException.h"

namespace ul
{
DaqODevice::DaqODevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlDaqODevice()
{
	for(int i = 0; i < 3; i++)
	{
		mLastStatus[i].error = ERR_NO_ERROR;
		mLastStatus[i].scanCount = 0;
		mLastStatus[i].totalCount = 0;
		mLastStatus[i].index = -1;
	}
}

DaqODevice::~DaqODevice()
{

}


double DaqODevice::daqOutScan(DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, double data[])
{
	return daqOutScan(FT_DAQO, chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);
}

double DaqODevice::daqOutScan(FunctionType functionType, DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, void* data)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

UlError DaqODevice::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void  DaqODevice::stopBackground()
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

UlError DaqODevice::getStatus(FunctionType functionType, ScanStatus* status, TransferStatus* xferStatus)
{
	if(mScanInfo.functionType == functionType)
	{
		return getStatus(status, xferStatus);
	}
	else
	{
		*status = SS_IDLE;
		return getLastStatus(functionType, xferStatus);
	}
}

UlError DaqODevice::waitUntilDone(FunctionType functionType, double timeout)
{
	UlError err = ERR_NO_ERROR;

	if(mScanInfo.functionType == functionType)
	{
		err =  IoDevice::waitUntilDone(timeout);
	}

	return err;
}

void DaqODevice::stopBackground(FunctionType functionType)
{
	if(mScanInfo.functionType == functionType || mScanInfo.functionType == 0)
	{
		stopBackground();
	}
}

void DaqODevice::setTrigger(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount)
{
	if(mDaqOInfo.supportsTrigger())
	{
		check_DaqOutSetTrigger_Args(type, trigChanDesc, level, variance, retriggerCount);

		mTrigCfg.type = type;
		mTrigCfg.trigChan = trigChanDesc.channel;
		mTrigCfg.level = level;
		mTrigCfg.variance = variance;
		mTrigCfg.retrigCount = retriggerCount;
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqODevice::storeLastStatus()
{
	int index = -1;
	ScanStatus status;
	TransferStatus xferStatus;

	UlError error = getStatus(&status, &xferStatus);

	switch(mScanInfo.functionType)
	{
	case FT_DAQO:
		index = 0;
		break;
	case FT_AO:
		index = 1;
		break;
	case FT_DO:
		index = 2;
		break;
	default:
		break;
	}

	if(index != -1)
	{
		mLastStatus[index].error = error;
		mLastStatus[index].scanCount = xferStatus.currentScanCount;
		mLastStatus[index].totalCount = xferStatus.currentTotalCount;
		mLastStatus[index].index = xferStatus.currentIndex;
	}
}

UlError DaqODevice::getLastStatus(FunctionType functionType, TransferStatus* xferStatus)
{
	UlError error = ERR_NO_ERROR;
	int index = -1;

	switch(functionType)
	{
	case FT_DAQO:
		index = 0;
		break;
	case FT_AO:
		index = 1;
		break;
	case FT_DO:
		index = 2;
		break;
	default:
		break;
	}

	if(index != -1)
	{
		error = mLastStatus[index].error;
		xferStatus->currentScanCount = mLastStatus[index].scanCount;
		xferStatus->currentTotalCount = mLastStatus[index].totalCount;
		xferStatus->currentIndex = mLastStatus[index].index;
	}

	return error;
}



void DaqODevice::check_DaqOutScan_Args(DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, void* data) const
{
	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(chanDescriptors != NULL)
	{
		bool invalidRate = false;

		if(numChans > mDaqOInfo.getMaxQueueLength())
			throw UlException(ERR_BAD_NUM_CHANS);

		for(unsigned int i = 0; i < (unsigned int) numChans; i++)
		{
			if(mDaqOInfo.getChannelTypes() & chanDescriptors[i].type)
			{
				std::bitset<32> typeBitSet(chanDescriptors[i].type);

				if(typeBitSet.count() > 1)
					throw UlException(ERR_BAD_DAQO_CHAN_TYPE);

				if(chanDescriptors[i].type == DAQO_ANALOG)
				{
					const AoInfo& aoInfo = (const AoInfo&) mDaqDevice.getAoDevice().getAoInfo();

					if(!aoInfo.isRangeSupported(chanDescriptors[i].range))
						throw UlException(ERR_BAD_RANGE);

					if(rate > aoInfo.getMaxScanRate())
						invalidRate = true;
				}
				else if(chanDescriptors[i].type == DAQO_DIGITAL)
				{
					const DioInfo& dioInfo = (const DioInfo&) mDaqDevice.getDioDevice().getDioInfo();

					if(dioInfo.isPortSupported((DigitalPortType) chanDescriptors[i].channel) == false)
						throw UlException(ERR_BAD_PORT_TYPE);

					if(rate > dioInfo.getMaxScanRate(DD_OUTPUT))
						invalidRate = true;
				}
			}
			else
			{
				throw UlException(ERR_BAD_DAQO_CHAN_TYPE);
			}
		}

		if(data == NULL)
			throw UlException(ERR_BAD_BUFFER);

		if(~mDaqOInfo.getScanOptions() & options)
			throw UlException(ERR_BAD_OPTION);

		if(~mDaqOInfo.getDaqOutScanFlags() & flags)
			throw UlException(ERR_BAD_FLAG);

		if((!(options & SO_EXTCLOCK) && invalidRate) || (rate <= 0.0))
			throw UlException(ERR_BAD_RATE);

		if(samplesPerChan < mMinScanSampleCount)
			throw UlException(ERR_BAD_SAMPLE_COUNT);

		if(!mDaqDevice.isConnected())
			throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
	}
}

void DaqODevice::check_DaqOutSetTrigger_Args(TriggerType trigType, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) const
{
	if(mDaqOInfo.supportsTrigger())
	{
		if(!(mDaqOInfo.getTriggerTypes() & trigType))
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mDaqOInfo.getScanOptions() & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);

		if(!(trigType & (TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_HIGH | TRIG_LOW)))
		{
			if(trigChanDesc.type == DAQI_CTR16 || trigChanDesc.type == DAQI_CTR32 || trigChanDesc.type == DAQI_CTR48)
			{
				throw UlException(ERR_BAD_DAQI_CHAN_TYPE);
			}
			else if(trigChanDesc.type == DAQI_ANALOG_DIFF || trigChanDesc.type == DAQI_ANALOG_SE)
			{
				throw UlException(ERR_BAD_DAQI_CHAN_TYPE);
			}
			else if(trigChanDesc.type == DAQI_DIGITAL)
			{
				DioDevice* dioDev = mDaqDevice.dioDevice();

				if(dioDev)
				{
					if(!(trigType & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW)))
						throw UlException(ERR_BAD_DAQI_CHAN_TYPE);

					bool validPort = false;

					const UlDioInfo& dioInfo = dioDev->getDioInfo();
					unsigned int portNum;

					for(portNum = 0; portNum < dioInfo.getNumPorts(); portNum++)
					{
						if(trigChanDesc.channel == dioInfo.getPortType(portNum))
						{
							validPort = true;
							break;
						}
					}

					if(!validPort)
						throw UlException(ERR_BAD_PORT_TYPE);

					unsigned long long pattern = ((unsigned long long) level) & ((unsigned long long) variance);
					unsigned long long maxPortVal = (1ULL << dioInfo.getNumBits(portNum)) - 1;

					if(pattern > maxPortVal)
						throw UlException(ERR_BAD_PORT_VAL);
				}
				else
					throw UlException(ERR_BAD_DAQI_CHAN_TYPE);
			}
		}
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

} /* namespace ul */
