/*
 * AoDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoDevice.h"
#include <math.h>
#include <algorithm>
#include <bitset>

#include "UlException.h"

namespace ul
{

AoDevice::AoDevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlAoDevice()
{
	mAoConfig = new AoConfig(*this);
	mCalDate = 0;
}

AoDevice::~AoDevice()
{
	if(mAoConfig != NULL)
	{
		delete mAoConfig;
		mAoConfig = NULL;
	}
}

void AoDevice::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void AoDevice::aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[])
{
	check_AOutArray_Args(lowChan, highChan, range, flags, data);

	int i;
	for(int chan = lowChan; chan <= highChan; chan++)
	{
		i = chan - lowChan;
		aOut(chan, range[i], (AOutFlag) flags, data[i]);
	}
}

double AoDevice::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void AoDevice::setTrigger(TriggerType type, int trigChan,  double level, double variance, unsigned int retriggerCount)
{
	if(mAoInfo.supportsTrigger())
	{
		check_AOutSetTrigger_Args(type, trigChan, level, variance, retriggerCount);

		mTrigCfg.type = type;
		mTrigCfg.trigChan = trigChan;
		mTrigCfg.level = level;
		mTrigCfg.variance = variance;
		mTrigCfg.retrigCount = retriggerCount;
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

UlError AoDevice::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}
void  AoDevice::stopBackground()
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

unsigned int AoDevice::calibrateData(int channel, Range range, AOutFlag flags, double data) const
{
	CalCoef calCoef = getCalCoef(channel, range, flags);

	unsigned int calData = 0;

	calData =  (calCoef.slope * data) + calCoef.offset + 0.5;

	unsigned int maxVal = getMaxOutputValue(range, false);

	if(calData > maxVal)
		calData = maxVal;

	return  calData;
}

CalCoef AoDevice::getCalCoef(int channel, Range range, long long flags) const
{
	CalCoef coef;

	// some devices like usb-1208fs-plus don't have DAC cal coefs, if device does not have cal coefs return default cal coef
	if(mAoInfo.getCalCoefCount() <= 0)
		return getDefaultCalCoef(channel, range, flags);

	if(mCalCoefs.empty())
		const_cast<AoDevice*>(this)->loadDacCoefficients();

	if(!mCalCoefs.empty())
	{
		double offset = 0;
		double scale = 0;
		mDaqDevice.getEuScaling(range, scale, offset);

		int calCoefIdx =  getCalCoefIndex(channel, range);

		unsigned long long fullScaleCount = 1ULL << mAoInfo.getResolution();

		double lsb = scale / pow(2.0, mAoInfo.getResolution());

		if (!(flags & NOSCALEDATA))
		{
			if(flags & NOCALIBRATEDATA)
			{
				coef.slope = 1 / lsb;
				coef.offset = ((-offset) / scale) *  fullScaleCount;
			}
			else
			{
				coef.slope = mCalCoefs[calCoefIdx].slope / lsb;
				coef.offset = mCalCoefs[calCoefIdx].offset + mCalCoefs[calCoefIdx].slope * ((-offset) / scale) *  fullScaleCount;
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

CalCoef AoDevice::getDefaultCalCoef(int channel, Range range, long long flags) const
{
	CalCoef coef;

	double offset = 0;
	double scale = 0;
	mDaqDevice.getEuScaling(range, scale, offset);

	unsigned long long fullScaleCount = 1ULL << mAoInfo.getResolution();

	double lsb = scale / pow(2.0, mAoInfo.getResolution());

	if (!(flags & NOSCALEDATA))
	{
		coef.slope = 1 / lsb;
		coef.offset = ((-offset) / scale) *  fullScaleCount;
	}
	else
	{
		coef.slope = 1.0;
		coef.offset = 0.0;
	}

	return coef;
}

std::vector<CalCoef> AoDevice::getScanCalCoefs(int lowChan, int highChan, Range range, long long flags) const
{
	std::vector<CalCoef> calCoefs;

	int chan;

	CalCoef calCoef;


	for (chan = lowChan; chan <= highChan; chan++)
	{
		calCoef = getCalCoef(chan, range, flags);
		calCoefs.push_back(calCoef);
	}

	return calCoefs;
}

double AoDevice::getMaxOutputValue(Range range, bool scaled) const
{
	unsigned long long maxCount = 0;
	double maxOutputVal = 0;

	maxCount = (1ULL << mAoInfo.getResolution()) - 1;

	if(scaled)
		maxOutputVal = toEngUnits(maxCount, range);
	else
		maxOutputVal = maxCount;

	return maxOutputVal;
}

unsigned int AoDevice::fromEngUnits(double engUnits, Range range) const
{
	unsigned int counts = 0;
	double offset = 0;
	double scale = 0;
	mDaqDevice.getEuScaling(range, scale, offset);

	unsigned int maxVal = getMaxOutputValue(range, false);
	offset = -offset;

	if (engUnits <= offset)
		counts = 0;
	else if (engUnits >= (offset + scale * (1 - (1 / maxVal))))
		counts =  (maxVal - 1.0) + 0.5;
	else
		counts = (((engUnits - offset) / scale) * maxVal) + 0.5;


	return counts;
}

double AoDevice::toEngUnits(unsigned int counts, Range range) const
{
	double engUnits = 0;
	double offset = 0;
	double scale = 0;
	mDaqDevice.getEuScaling(range, scale, offset);

	unsigned int maxVal = getMaxOutputValue(range, false);

	if (counts > maxVal)
		engUnits = scale + offset;
	else
		engUnits = ((double)counts / maxVal) * scale + offset;

	return engUnits;
}

void AoDevice::check_AOut_Args(int channel, Range range, AOutFlag flags, double dataValue) const
{
	bool scaled = true;

	if(flags & NOSCALEDATA)
		scaled = false;

	double maxDataValue = getMaxOutputValue(range, scaled);

	if(channel < 0 || channel >= mAoInfo.getNumChans())
		throw UlException(ERR_BAD_AO_CHAN);

	if(!mAoInfo.isRangeSupported(range))
		throw UlException(ERR_BAD_RANGE);

	if (dataValue > maxDataValue)
		throw UlException(ERR_BAD_DA_VAL);


	if(flags & NOSCALEDATA)
	{
		if(dataValue < 0)
			throw UlException(ERR_BAD_DA_VAL);
	}
	else
	{
		double offset = 0;
		double scale = 0;
		mDaqDevice.getEuScaling(range, scale, offset);

		if(dataValue < offset)
			throw UlException(ERR_BAD_DA_VAL);
	}

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void AoDevice::check_AOutArray_Args(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]) const
{
	int numOfScanChan = 0;
	double maxDataValue;
	bool scaled = true;

	if(flags & NOSCALEDATA)
		scaled = false;

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(lowChan < 0 || highChan < 0 || lowChan > mAoInfo.getNumChans() || highChan >= mAoInfo.getNumChans() || lowChan > highChan )
		throw UlException(ERR_BAD_AO_CHAN);

	if(data == NULL || range == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(~mAoInfo.getAOutArrayFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	numOfScanChan = highChan - lowChan + 1;

	for(int i = 0; i < numOfScanChan; i++)
	{
		if(!mAoInfo.isRangeSupported(range[i]))
			throw UlException(ERR_BAD_RANGE);

		maxDataValue = getMaxOutputValue(range[i], scaled);

		if (data[i] > maxDataValue)
			throw UlException(ERR_BAD_DA_VAL);

		if(flags & NOSCALEDATA)
		{
			if(data[0] < 0)
				throw UlException(ERR_BAD_DA_VAL);
		}
		else
		{
			double offset = 0;
			double scale = 0;
			mDaqDevice.getEuScaling(range[i], scale, offset);

			if(data[i] < offset)
				throw UlException(ERR_BAD_DA_VAL);
		}
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void AoDevice::check_AOutScan_Args(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]) const
{
	int numOfScanChan = 0;

	if(!mAoInfo.hasPacer())
		throw UlException(ERR_BAD_DEV_TYPE);

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if((options & SO_SINGLEIO) && (options & SO_BLOCKIO))
			throw UlException(ERR_BAD_OPTION);

	if(lowChan < 0 || highChan < 0 || lowChan > mAoInfo.getNumChans() || highChan >= mAoInfo.getNumChans() || lowChan > highChan )
		throw UlException(ERR_BAD_AO_CHAN);

	if(!mAoInfo.isRangeSupported(range))
		throw UlException(ERR_BAD_RANGE);

	numOfScanChan = highChan - lowChan + 1;

	if(~mAoInfo.getScanOptions() & options)
		throw UlException(ERR_BAD_OPTION);

	if(~mAoInfo.getAOutScanFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	double throughput = rate * numOfScanChan;

	if(!(options & SO_EXTCLOCK))
	{
		if(rate > mAoInfo.getMaxScanRate() || throughput > mAoInfo.getMaxThroughput())
			throw UlException(ERR_BAD_RATE);
	}

	if(rate <= 0.0)
		throw UlException(ERR_BAD_RATE);

	if(samplesPerChan < mMinScanSampleCount)
		throw UlException(ERR_BAD_SAMPLE_COUNT);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void AoDevice::check_AOutSetTrigger_Args(TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const
{
	if(mAoInfo.supportsTrigger())
	{
		if(!(mAoInfo.getTriggerTypes() & trigType) )
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mAoInfo.getScanOptions() & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}


//////////////////////          Configuration functions          /////////////////////////////////

void AoDevice::setCfg_SyncMode(AOutSyncMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
AOutSyncMode AoDevice::getCfg_SyncMode() const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

void AoDevice::setCfg_SenseMode(int channel, AOutSenseMode mode)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

AOutSenseMode AoDevice::getCfg_SenseMode(int channel) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}


} /* namespace ul */
