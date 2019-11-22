/*
 * DaqIDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqIDevice.h"
#include "AiInfo.h"
#include "DioInfo.h"
#include "CtrInfo.h"
#include "DioDevice.h"

#include <math.h>
#include <algorithm>
#include "UlException.h"

namespace ul
{

DaqIDevice::DaqIDevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlDaqIDevice()
{
	for(int i = 0; i < 4; i++)
	{
		mLastStatus[i].error = ERR_NO_ERROR;
		mLastStatus[i].scanCount = 0;
		mLastStatus[i].totalCount = 0;
		mLastStatus[i].index = -1;
	}
}

DaqIDevice::~DaqIDevice()
{

}

double DaqIDevice::daqInScan(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, double data[])
{
	return daqInScan(FT_DAQI, chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);
}

double DaqIDevice::daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

UlError DaqIDevice::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void  DaqIDevice::stopBackground()
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

UlError DaqIDevice::getStatus(FunctionType functionType, ScanStatus* status, TransferStatus* xferStatus)
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

UlError DaqIDevice::waitUntilDone(FunctionType functionType, double timeout)
{
	UlError err = ERR_NO_ERROR;

	if(mScanInfo.functionType == functionType)
	{
		err =  IoDevice::waitUntilDone(timeout);
	}

	return err;
}

void DaqIDevice::stopBackground(FunctionType functionType)
{
	if(mScanInfo.functionType == functionType || mScanInfo.functionType == 0)
	{
		stopBackground();
	}
}

void DaqIDevice::setTrigger(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount)
{
	if(mDaqIInfo.supportsTrigger())
	{
		check_DaqInSetTrigger_Args(type, trigChanDesc, level, variance, retriggerCount);

		mTrigCfg.type = type;
		mTrigCfg.trigChan = trigChanDesc.channel;
		mTrigCfg.level = level;
		mTrigCfg.variance = variance;
		mTrigCfg.retrigCount = retriggerCount;
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqIDevice::storeLastStatus()
{
	int index = -1;
	ScanStatus status;
	TransferStatus xferStatus;

	UlError error = getStatus(&status, &xferStatus);

	switch(mScanInfo.functionType)
	{
	case FT_DAQI:
		index = 0;
		break;
	case FT_AI:
		index = 1;
		break;
	case FT_DI:
		index = 2;
		break;
	case FT_CTR:
		index = 3;
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

UlError DaqIDevice::getLastStatus(FunctionType functionType, TransferStatus* xferStatus)
{
	UlError error = ERR_NO_ERROR;
	int index = -1;

	switch(functionType)
	{
	case FT_DAQI:
		index = 0;
		break;
	case FT_AI:
		index = 1;
		break;
	case FT_DI:
		index = 2;
		break;
	case FT_CTR:
		index = 3;
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



void DaqIDevice::check_DaqInScan_Args(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data) const
{
	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(chanDescriptors != NULL)
	{
		bool invalidRate = false;

		if(numChans > mDaqIInfo.getMaxQueueLength())
			throw UlException(ERR_BAD_NUM_CHANS);

		for(unsigned int i = 0; i < (unsigned int) numChans; i++)
		{
			if(mDaqIInfo.getChannelTypes() & chanDescriptors[i].type)
			{
				std::bitset<32> typeBitSet(chanDescriptors[i].type);

				if(typeBitSet.count() > 1)
					throw UlException(ERR_BAD_DAQI_CHAN_TYPE);

				if(chanDescriptors[i].type == DAQI_ANALOG_DIFF || chanDescriptors[i].type == DAQI_ANALOG_SE)
				{
					const AiInfo& aiInfo = (const AiInfo&) mDaqDevice.getAiDevice().getAiInfo();

					AiInputMode inputMode = chanDescriptors[i].type == DAQI_ANALOG_DIFF ? AI_DIFFERENTIAL : AI_SINGLE_ENDED;

					if(chanDescriptors[i].channel >= aiInfo.getNumChansByMode(inputMode))
						throw UlException(ERR_BAD_AI_CHAN);

					if(!aiInfo.isInputModeSupported(inputMode))
						throw UlException(ERR_BAD_INPUT_MODE);

					if(!aiInfo.isRangeSupported(inputMode, chanDescriptors[i].range))
						throw UlException(ERR_BAD_RANGE);

					if(rate > aiInfo.getMaxScanRate())
						invalidRate = true;
				}
				else if(chanDescriptors[i].type == DAQI_DIGITAL)
				{
					const DioInfo& dioInfo = (const DioInfo&) mDaqDevice.getDioDevice().getDioInfo();

					if(dioInfo.isPortSupported((DigitalPortType) chanDescriptors[i].channel) == false)
						throw UlException(ERR_BAD_PORT_TYPE);

					if(rate > dioInfo.getMaxScanRate(DD_INPUT))
						invalidRate = true;
				}
				else if(chanDescriptors[i].type == DAQI_CTR16 || chanDescriptors[i].type == DAQI_CTR32 || chanDescriptors[i].type == DAQI_CTR48)
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

		if(~mDaqIInfo.getDaqInScanFlags() & flags)
			throw UlException(ERR_BAD_FLAG);

		if((!(options & SO_EXTCLOCK) && invalidRate) || (rate <= 0.0))
			throw UlException(ERR_BAD_RATE);

		if(samplesPerChan < mMinScanSampleCount)
			throw UlException(ERR_BAD_SAMPLE_COUNT);

		if(!mDaqDevice.isConnected())
			throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
	}
}

void DaqIDevice::check_DaqInSetTrigger_Args(TriggerType trigType, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) const
{
	if(mDaqIInfo.supportsTrigger())
	{
		if(!(mDaqIInfo.getTriggerTypes() & trigType))
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mDaqIInfo.getScanOptions() & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);

		if(!(trigType & (TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_HIGH | TRIG_LOW)))
		{
			if(trigChanDesc.type == DAQI_CTR16 || trigChanDesc.type == DAQI_CTR32 || trigChanDesc.type == DAQI_CTR48)
			{
				throw UlException(ERR_BAD_DAQI_CHAN_TYPE);
			}
			else if(trigChanDesc.type == DAQI_ANALOG_DIFF || trigChanDesc.type == DAQI_ANALOG_SE)
			{
				if(!(trigType & (TRIG_RISING | TRIG_FALLING | TRIG_ABOVE |TRIG_BELOW)))
					throw UlException(ERR_BAD_DAQI_CHAN_TYPE);
			}
			else if(trigChanDesc.type == DAQI_DIGITAL)
			{
				DioDevice* dioDev = mDaqDevice.dioDevice();

				if(dioDev)
				{
					if(!(trigType & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE |TRIG_PATTERN_BELOW)))
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
