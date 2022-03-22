/*
 * AiDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiDevice.h"
#include "UlException.h"

#include <math.h>
#include <algorithm>
#include <bitset>


namespace ul
{

AiDevice::AiDevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlAiDevice()
{
	mAiConfig = new AiConfig(*this);
	mCalDate = 0;
	mFieldCalDate = 0;
	mCalModeEnabled = false;
	mScanTempChanSupported = false;
	mScanTempUnit = TU_CELSIUS;
}

AiDevice::~AiDevice()
{
	if(mAiConfig != NULL)
	{
		delete mAiConfig;
		mAiConfig = NULL;
	}
}

double AiDevice::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

double AiDevice::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void AiDevice::aInLoadQueue(AiQueueElement queue[], unsigned int numElements)
{
	check_AInLoadQueue_Args(queue, numElements);

	if(queue == NULL || numElements == 0) // disable loadqueue
		mAQueue.clear();
	else
	{
		mAQueue.clear();

		mAQueue.insert(mAQueue.begin(), &queue[0], &queue[numElements]);
	}
}

void AiDevice::setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	check_AInSetTrigger_Args(type, trigChan, level, variance, retriggerCount);

	mTrigCfg.type = type;
	mTrigCfg.trigChan = trigChan;
	mTrigCfg.level = level;
	mTrigCfg.variance = variance;
	mTrigCfg.retrigCount = retriggerCount;
}

UlError AiDevice::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}
void  AiDevice::stopBackground()
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void AiDevice::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}
void AiDevice::tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[])
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void AiDevice::check_AIn_Args(int channel, AiInputMode inputMode, Range range, AInFlag flags) const
{
	if(!mAiInfo.isInputModeSupported(inputMode))
		throw UlException(ERR_BAD_INPUT_MODE);

	if(channel < 0 || channel >= mAiInfo.getNumChansByMode(inputMode))
		throw UlException(ERR_BAD_AI_CHAN);

	if(!mAiInfo.isRangeSupported(inputMode, range))
			throw UlException(ERR_BAD_RANGE);

	if(~mAiInfo.getAInFlags() & flags)
			throw UlException(ERR_BAD_FLAG);

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

	if((int) mCustomScales.size() < mAiInfo.getNumChans())
		throw UlException(ERR_INTERNAL);
}

void AiDevice::check_AInScan_Args(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) const
{
	int numOfScanChan = 0;

	if(!mAiInfo.hasPacer())
		throw UlException(ERR_BAD_DEV_TYPE);

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(((options & SO_SINGLEIO) && (options & SO_BLOCKIO)) || ((options & SO_SINGLEIO) && (options & SO_BURSTIO)) || ((options & SO_BLOCKIO) && (options & SO_BURSTIO)))
		throw UlException(ERR_BAD_OPTION);

	if(queueEnabled())
		numOfScanChan = queueLength();
	else
	{
		if(!mAiInfo.isInputModeSupported(inputMode))
			throw UlException(ERR_BAD_INPUT_MODE);

		if(lowChan < 0 || highChan < 0 || lowChan >= mAiInfo.getNumChansByMode(inputMode) || highChan >= mAiInfo.getNumChansByMode(inputMode) || lowChan > highChan )
			throw UlException(ERR_BAD_AI_CHAN);

		numOfScanChan = highChan - lowChan + 1;

		if(!mAiInfo.isRangeSupported(inputMode, range))
			throw UlException(ERR_BAD_RANGE);
	}

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(~mAiInfo.getScanOptions() & options)
		throw UlException(ERR_BAD_OPTION);

	if(~mAiInfo.getAInScanFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	double throughput = rate * numOfScanChan;

	if(!(options & SO_EXTCLOCK))
	{
		if(((options & SO_BURSTIO) && (rate > mAiInfo.getMaxBurstRate() || throughput > mAiInfo.getMaxBurstThroughput())) || (!(options & SO_BURSTIO) && (rate > mAiInfo.getMaxScanRate() || throughput > mAiInfo.getMaxThroughput())) )
			throw UlException(ERR_BAD_RATE);
	}

	if(rate <= 0.0)
		throw UlException(ERR_BAD_RATE);

	if(samplesPerChan < mMinScanSampleCount)
		throw UlException(ERR_BAD_SAMPLE_COUNT);

	long long totalCount = (long long) samplesPerChan * numOfScanChan;

	if(options & SO_BURSTIO)
	{
		if(totalCount > (mAiInfo.getFifoSize() / mAiInfo.getSampleSize()))
			throw UlException(ERR_BAD_BURSTIO_COUNT);
		else if (options & SO_CONTINUOUS)
			throw UlException(ERR_BAD_OPTION);
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);


	if((int) mCustomScales.size() < mAiInfo.getNumChans())
		throw UlException(ERR_INTERNAL);
}

void AiDevice::check_AInLoadQueue_Args(const AiQueueElement queue[], unsigned int numElements) const
{
	if(queue != NULL)
	{
		if(mAiInfo.getQueueTypes())
		{
			for(unsigned int i = 0; i < numElements; i++)
			{
				if(!mAiInfo.isInputModeSupported(queue[i].inputMode))
					throw UlException(ERR_BAD_INPUT_MODE);

				if(numElements > (unsigned int) mAiInfo.getMaxQueueLength(queue[i].inputMode))
					throw UlException(ERR_BAD_QUEUE_SIZE);

				if(queue[i].channel < 0 || queue[i].channel >= mAiInfo.getNumChansByMode(queue[i].inputMode))
					throw UlException(ERR_BAD_AI_CHAN);

				if(!mAiInfo.isRangeSupported(queue[i].inputMode, queue[i].range))
					throw UlException(ERR_BAD_RANGE);
			}

			if(!isValidChanQueue(queue, numElements))
				throw UlException(ERR_BAD_AI_CHAN_QUEUE);

			if(!isValidGainQueue(queue, numElements))
				throw UlException(ERR_BAD_AI_GAIN_QUEUE);

			if(!isValidModeQueue(queue, numElements))
				throw UlException(ERR_BAD_AI_MODE_QUEUE);
		}
		else
			throw UlException(ERR_BAD_DEV_TYPE);
	}
}

void AiDevice::check_AInSetTrigger_Args(TriggerType trigType, int trigChan, double level, double variance, unsigned int retriggerCount) const
{
	if(mAiInfo.supportsTrigger())
	{
		if(!(mAiInfo.getTriggerTypes() & trigType))
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mAiInfo.getScanOptions() & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

bool AiDevice::isValidChanQueue(const AiQueueElement queue[], unsigned int numElements) const
{
	bool valid = true;

	AiQueueType queueTypes = mAiInfo.getQueueTypes();

	if(!(queueTypes & CHAN_QUEUE)) // if channel queue is not supported then make sure channels are consecutive
	{
		for(unsigned int i = 1; i < numElements; i++)
		{
			if(queue[i].channel != (queue[i - 1].channel + 1))
			{
				valid = false;
				return valid;
			}
		}
	}
	else
	{
		AiChanQueueLimitation chanQueueLimitations = mAiInfo.getChanQueueLimitations();

		if(chanQueueLimitations & UNIQUE_CHAN)
		{
			for(unsigned int i = 1; i < numElements; i++)
			{
				for(unsigned int j = 0; j < i; j++)
				{
					if(queue[i].channel == queue[j].channel)
					{
						valid = false;
						return valid;
					}
				}
			}
		}

		if(chanQueueLimitations & ASCENDING_CHAN)
		{
			for(unsigned int i = 1; i < numElements; i++)
			{
				if(queue[i].channel <= queue[i - 1].channel)
				{
					valid = false;
					return valid;
				}
			}
		}
	}

	return valid;
}

bool AiDevice::isValidGainQueue(const AiQueueElement queue[], unsigned int numElements) const
{
	bool valid = true;

	AiQueueType queueTypes = mAiInfo.getQueueTypes();

	if(!(queueTypes & GAIN_QUEUE)) // if gain queue is not supported then make sure all gains are the same
	{
		for(unsigned int i = 1; i < numElements; i++)
		{
			if(queue[i].range != queue[i - 1].range )
			{
				valid = false;
				return valid;
			}
		}
	}

	return valid;
}

bool AiDevice::isValidModeQueue(const AiQueueElement queue[], unsigned int numElements) const
{
	bool valid = true;

	AiQueueType queueTypes = mAiInfo.getQueueTypes();

	if(!(queueTypes & MODE_QUEUE)) // if gain queue is not supported then make sure all gains are the same
	{
		for(unsigned int i = 1; i < numElements; i++)
		{
			if(queue[i].inputMode != queue[i - 1].inputMode )
			{
				valid = false;
				return valid;
			}
		}
	}

	return valid;
}

double AiDevice::calibrateData(int channel, AiInputMode inputMode, Range range, unsigned int count, long long flags) const
{
	double calData = 0;

	CalCoef calCoef = getCalCoef(channel, inputMode, range, flags);

	calData = (calCoef.slope * count) + calCoef.offset;

	if(flags & NOSCALEDATA)
	{
		unsigned long long maxVal = (1ULL << mAiInfo.getResolution()) - 1;

		if(calData > maxVal)
			calData = maxVal;
		else if(calData < 0)
			calData = 0;
	}

	return  calData;
}

CalCoef AiDevice::getCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const
{
	CalCoef coef;

	if(mCalCoefs.empty())
		const_cast<AiDevice*>(this)->loadAdcCoefficients();

	if(!mCalCoefs.empty())
	{
		double offset = 0;
		double scale = 0;
		mDaqDevice.getEuScaling(range, scale, offset);

		int calCoefIdx =  getCalCoefIndex(channel, inputMode, range);

		double lsb = scale / pow(2.0, mAiInfo.getResolution());

		if (!(flags & NOSCALEDATA))
		{
			if(flags & NOCALIBRATEDATA)
			{
				coef.slope = lsb;
				coef.offset = offset;
			}
			else
			{
				coef.slope = mCalCoefs[calCoefIdx].slope * lsb;
				coef.offset = mCalCoefs[calCoefIdx].offset * lsb + offset;
			}
		}
		else
		{
			if(flags & NOCALIBRATEDATA)
			{
				coef.slope = 1.0;
				coef.offset = 0.0;
			}
			else
			{
				coef.slope = mCalCoefs[calCoefIdx].slope;
				coef.offset = mCalCoefs[calCoefIdx].offset;
			}
		}
	}
	else
		throw UlException(ERR_DEAD_DEV);

	return coef;
}

std::vector<CalCoef> AiDevice::getScanCalCoefs(int lowChan, int highChan, AiInputMode inputMode, Range range, long long flags) const
{
	std::vector<CalCoef> calCoefs;

	int chan;

	CalCoef calCoef;

	if (!queueEnabled())
	{
		for (chan = lowChan; chan <= highChan; chan++)
		{
			calCoef = getCalCoef(chan, inputMode, range, flags);
			calCoefs.push_back(calCoef);
		}
	}
	else
	{
		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
		{
			calCoef = getCalCoef(mAQueue[idx].channel, mAQueue[idx].inputMode, mAQueue[idx].range, flags);
			calCoefs.push_back(calCoef);
		}
	}

	return calCoefs;
}

std::vector<CustomScale> AiDevice::getCustomScales(int lowChan, int highChan) const
{
	std::vector<CustomScale> customScales;

	int chan;

	if (!queueEnabled())
	{
		for (chan = lowChan; chan <= highChan; chan++)
			customScales.push_back(mCustomScales[chan]);
	}
	else
	{
		for (unsigned int idx = 0; idx < mAQueue.size(); idx++)
			customScales.push_back(mCustomScales[mAQueue[idx].channel]);
	}

	return customScales;
}

bool AiDevice::queueEnabled() const
{
	return (!mAQueue.empty());
}
int AiDevice::queueLength() const
{
	return mAQueue.size();
}

double AiDevice::convertTempUnit(double tempC, TempUnit unit)
{
	double temp = tempC;
	switch(unit)
	{
	case TU_FAHRENHEIT:
		temp  = tempC * 1.8 + 32.0;
		break;
	case TU_KELVIN:
		temp  = tempC + 273.15;
		break;
	default:
	break;
	}

	return temp;
}

void AiDevice::initCustomScales()
{
	CustomScale coef;
	for(int i = 0; i < mAiInfo.getNumChans(); i++)
	{
		coef.slope = 1.0;
		coef.offset = 0;

		mCustomScales.push_back(coef);
	}
}

/*void AiDevice::initTempUnits()
{
	for(int i = 0; i < mAiInfo.getNumChans(); i++)
	{
		mScanChanTempUnit.push_back(TU_CELSIUS);
	}
}*/

void AiDevice::check_TIn_Args(int channel, TempScale scale, TInFlag flags) const
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
	{
		bool cjcChan = false;

		if(channel > 0)
		{
			for(int i = 0; i < mAiInfo.getNumCjcChans(); i++)
			{
				if(channel == (0x80 + i))
				{
					cjcChan = true;
					break;
				}
			}
		}

		if(!cjcChan)
			throw UlException(ERR_BAD_AI_CHAN);
	}

	if(~mAiInfo.getTInFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void AiDevice::check_TInArray_Args(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]) const
{
	if(lowChan < 0 || highChan < 0 || lowChan >= mAiInfo.getNumChans() || highChan >= mAiInfo.getNumChans() || lowChan > highChan )
		throw UlException(ERR_BAD_AI_CHAN);

	if(~mAiInfo.getTInArrayFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}


//////////////////////          Configuration functions          /////////////////////////////////

void AiDevice::setCfg_ChanType(int channel, AiChanType chanType)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
AiChanType AiDevice::getCfg_ChanType(int channel) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_ChanTcType(int channel, TcType tcType)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
TcType AiDevice::getCfg_ChanTcType(int channel) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_ScanTempUnit(TempUnit unit)
{
	if(!mScanTempChanSupported)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if( unit < TU_CELSIUS || unit > TU_KELVIN)
		throw UlException(ERR_BAD_UNIT);

	mScanTempUnit = unit;

	//for(unsigned int i = 0; i < mScanChanTempUnit.size(); i++)
	//	mScanChanTempUnit[i] = unit;
}

TempUnit AiDevice::getCfg_ScanTempUnit() const
{
	if(!mScanTempChanSupported)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	return mScanTempUnit;
}

/*
void AiDevice::setCfg_ScanChanTempUnit(int channel, TempUnit unit)
{
	if(!mScanTempChanSupported)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if(channel < 0 || channel >= (int) mScanChanTempUnit.size())
		throw UlException(ERR_BAD_AI_CHAN);

	if( unit < TU_CELSIUS || unit > TU_KELVIN)
		throw UlException(ERR_BAD_UNIT);

	mScanChanTempUnit[channel] = unit;
}
TempUnit AiDevice::getCfg_ScanChanTempUnit(int channel) const
{
	if(!mScanTempChanSupported)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if(channel < 0 || channel >= (int) mScanChanTempUnit.size())
			throw UlException(ERR_BAD_AI_CHAN);

	return mScanChanTempUnit[channel];
}*/

void AiDevice::setCfg_AutoZeroMode(AutoZeroMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
AutoZeroMode AiDevice::getCfg_AutoZeroMode() const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_AdcTimingMode(AdcTimingMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
AdcTimingMode AiDevice::getCfg_AdcTimingMode()
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_ChanIepeMode(int channel, IepeMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

IepeMode AiDevice::getCfg_ChanIepeMode(int channel)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_ChanCouplingMode(int channel, CouplingMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
CouplingMode AiDevice::getCfg_ChanCouplingMode(int channel)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_ChanSensorSensitivity(int channel, double sensitivity)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

double AiDevice::getCfg_ChanSensorSensitivity(int channel)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_ChanSlope(int channel, double slope)
{
	if(channel < 0 || channel >= (int) mCustomScales.size())
		throw UlException(ERR_BAD_AI_CHAN);

	mCustomScales[channel].slope = slope;
}

double AiDevice::getCfg_ChanSlope(int channel)
{
	if(channel < 0 || channel >= (int) mCustomScales.size())
		throw UlException(ERR_BAD_AI_CHAN);

	return mCustomScales[channel].slope;

}
void AiDevice::setCfg_ChanOffset(int channel, double offset)
{
	if(channel < 0 || channel >= (int) mCustomScales.size())
		throw UlException(ERR_BAD_AI_CHAN);

	mCustomScales[channel].offset = offset;
}

double AiDevice::getCfg_ChanOffset(int channel)
{
	if(channel < 0 || channel >= (int) mCustomScales.size())
		throw UlException(ERR_BAD_AI_CHAN);

	return mCustomScales[channel].offset;
}

unsigned long long AiDevice::getCfg_CalDate(int calTableIndex)
{
	mDaqDevice.checkConnection();

	unsigned long long calDate = 0;

	if(calTableIndex == 0)
	{
		calDate = mCalDate;
	}
	else if(calTableIndex == 1)
	{
		calDate = mFieldCalDate;
	}

	return calDate;
}

void AiDevice::getCfg_CalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen)
{
	mDaqDevice.checkConnection();

	time_t calDateSec = getCfg_CalDate(calTableIndex);

	// convert seconds to string
	struct tm *timeinfo;
	timeinfo = localtime(&calDateSec);
	char calDateStr[128];
	strftime(calDateStr, 128, "%c", timeinfo);

	unsigned int len = strlen(calDateStr) + 1;

	if(len <= *maxStrLen)
	{
		memcpy(calDate, calDateStr, len);
		*maxStrLen = len;
	}
	else
	{
		*maxStrLen = len;

		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}

SensorConnectionType AiDevice::getCfg_SensorConnectionType(int channel) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::getCfg_ChanCoefsStr(int channel, char* coefsStr, unsigned int* len) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void  AiDevice::setCfg_ChanDataRate(int channel, double rate)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
double  AiDevice::getCfg_ChanDataRate(int channel) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void  AiDevice::setCfg_ChanOpenTcDetectionMode(int channel, OtdMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
OtdMode  AiDevice::getCfg_ChanOpenTcDetectionMode(int channel) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void  AiDevice::setCfg_OpenTcDetectionMode(int dev, OtdMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
OtdMode  AiDevice::getCfg_OpenTcDetectionMode(int dev) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_CalTableType(int dev, AiCalTableType calTableType)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
AiCalTableType AiDevice::getCfg_CalTableType(int dev) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::setCfg_RejectFreqType(int dev, AiRejectFreqType rejectFreqType)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
AiRejectFreqType AiDevice::getCfg_RejectFreqType(int dev) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

unsigned long long AiDevice::getCfg_ExpCalDate(int calTableIndex)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AiDevice::getCfg_ExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

} /* namespace ul */
