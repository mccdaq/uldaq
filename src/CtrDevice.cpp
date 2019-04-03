/*
 * CtrDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrDevice.h"
#include <algorithm>
#include <bitset>

#include "UlException.h"

namespace ul
{

CtrDevice::CtrDevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlCtrDevice()
{
	mCtrConfig = new CtrConfig(*this);
}

CtrDevice::~CtrDevice()
{
	if(mCtrConfig != NULL)
	{
		delete mCtrConfig;
		mCtrConfig = NULL;
	}
}

unsigned long long CtrDevice::cIn(int ctrNum)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void CtrDevice::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void CtrDevice::cClear(int ctrNum)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

unsigned long long CtrDevice::cRead(int ctrNum, CounterRegisterType regType)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

double CtrDevice::cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[])
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void CtrDevice::cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
							CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
							CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void CtrDevice::setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	check_CtrSetTrigger_Args(type, trigChan, level, variance, retriggerCount);

	mTrigCfg.type = type;
	mTrigCfg.trigChan = trigChan;
	mTrigCfg.level = level;
	mTrigCfg.variance = variance;
	mTrigCfg.retrigCount = retriggerCount;
}

UlError CtrDevice::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}
void  CtrDevice::stopBackground()
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void CtrDevice::initScanCountersState()
{
	mScanCtrActive.clear();

	for(int ctrNum = 0; ctrNum < mCtrInfo.getNumCtrs(); ctrNum++)
		mScanCtrActive.push_back(false);
}

void CtrDevice::setScanCounterActive(int ctrNum)
{
	if(ctrNum < mCtrInfo.getNumCtrs())
		mScanCtrActive[ctrNum] = true;
}

void CtrDevice::setScanCountersInactive()
{
	for(unsigned int ctrNum = 0; ctrNum < mScanCtrActive.size(); ctrNum++)
		mScanCtrActive[ctrNum] = false;
}

bool CtrDevice::isScanCounterActive(int ctrNum) const
{
	bool active = true;

	if(ctrNum < mCtrInfo.getNumCtrs())
		active = mScanCtrActive[ctrNum];

	return active;
}

void CtrDevice::check_CIn_Args(int ctrNum) const
{
	if(ctrNum < 0 || ctrNum >= mCtrInfo.getNumCtrs())
		throw UlException(ERR_BAD_CTR);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void CtrDevice::check_CLoad_Args(int ctrNum, CounterRegisterType regType, unsigned long long loadValue) const
{
	if(ctrNum < 0 || ctrNum >= mCtrInfo.getNumCtrs())
			throw UlException(ERR_BAD_CTR);

	unsigned long long maxCtrVal = (1ULL << mCtrInfo.getResolution()) - 1;

	if(loadValue > maxCtrVal)
		throw UlException(ERR_BAD_CTR_VAL);

	std::bitset<32> typeBitSet(regType);

	if(typeBitSet.count() > 1)
		throw UlException(ERR_BAD_CTR_REG);

	if(!(mCtrInfo.getRegisterTypes() & regType) || regType == CRT_COUNT)
		throw UlException(ERR_BAD_CTR_REG);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void CtrDevice::check_CClear_Args(int ctrNum) const
{
	if(ctrNum < 0 || ctrNum >= mCtrInfo.getNumCtrs())
		throw UlException(ERR_BAD_CTR);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void CtrDevice::check_CRead_Args(int ctrNum, CounterRegisterType regType) const
{
	if(ctrNum < 0 || ctrNum >= mCtrInfo.getNumCtrs())
		throw UlException(ERR_BAD_CTR);

	std::bitset<32> typeBitSet(regType);

	if(typeBitSet.count() > 1)
		throw UlException(ERR_BAD_CTR_REG);

	if(!(mCtrInfo.getRegisterTypes() & regType) || regType == CRT_LOAD)
		throw UlException(ERR_BAD_CTR_REG);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

}

void CtrDevice::check_CInScan_Args(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]) const
{
	int numOfScanCtr = highCtrNum - lowCtrNum + 1;

	if(!mCtrInfo.hasPacer())
		throw UlException(ERR_BAD_DEV_TYPE);

	if(getScanState() == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(((options & SO_SINGLEIO) && (options & SO_BLOCKIO)) || ((options & SO_SINGLEIO) && (options & SO_BURSTIO)) || ((options & SO_BLOCKIO) && (options & SO_BURSTIO)))
		throw UlException(ERR_BAD_OPTION);

	if(lowCtrNum < 0 || highCtrNum < 0 || lowCtrNum > mCtrInfo.getNumCtrs() || highCtrNum > mCtrInfo.getNumCtrs() || lowCtrNum > highCtrNum )
		throw UlException(ERR_BAD_CTR);

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(~mCtrInfo.getScanOptions() & options)
		throw UlException(ERR_BAD_OPTION);

	if(~mCtrInfo.getCInScanFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	if(((flags & CINSCAN_FF_CTR16_BIT) && (flags & CINSCAN_FF_CTR32_BIT)) ||
	   ((flags & CINSCAN_FF_CTR16_BIT) && (flags & CINSCAN_FF_CTR48_BIT)) ||
	   ((flags & CINSCAN_FF_CTR16_BIT) && (flags & CINSCAN_FF_CTR64_BIT)) ||
	   ((flags & CINSCAN_FF_CTR32_BIT) && (flags & CINSCAN_FF_CTR48_BIT)) ||
	   ((flags & CINSCAN_FF_CTR32_BIT) && (flags & CINSCAN_FF_CTR64_BIT)))
		throw UlException(ERR_BAD_FLAG);

	int sampleSize = 2;

	if(flags & CINSCAN_FF_CTR32_BIT)
		sampleSize = 4;
	else if ((flags & CINSCAN_FF_CTR48_BIT) || (flags & CINSCAN_FF_CTR64_BIT))
		sampleSize = 8;


	double throughput = rate * numOfScanCtr * (sampleSize / 2);

	if(!(options & SO_EXTCLOCK))
	{
		if(((options & SO_BURSTIO) && (rate > mCtrInfo.getMaxBurstRate() || throughput > mCtrInfo.getMaxBurstThroughput())) || (!(options & SO_BURSTIO) && (rate > mCtrInfo.getMaxScanRate() || throughput > mCtrInfo.getMaxThroughput())) )
			throw UlException(ERR_BAD_RATE);
	}

	if(rate <= 0.0)
		throw UlException(ERR_BAD_RATE);

	if(samplesPerCounter < mMinScanSampleCount)
		throw UlException(ERR_BAD_SAMPLE_COUNT);

	long totalCount = samplesPerCounter * numOfScanCtr;

	if(options & SO_BURSTIO)
	{
		int sampleSize = mCtrInfo.getResolution() / 8;

		if(flags & CINSCAN_FF_CTR16_BIT)
			sampleSize = 2;
		else if(flags & CINSCAN_FF_CTR32_BIT)
			sampleSize = 4;
		else if(flags & CINSCAN_FF_CTR64_BIT)
			sampleSize = 8;

		if(totalCount > (mCtrInfo.getFifoSize() / sampleSize))
			throw UlException(ERR_BAD_BURSTIO_COUNT);
		else if (options & SO_CONTINUOUS)
			throw UlException(ERR_BAD_OPTION);
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void CtrDevice::check_CConfigScan_Args(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
										CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
										CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag) const
{
	if(ctrNum < 0 || ctrNum >= mCtrInfo.getNumCtrs())
		throw UlException(ERR_BAD_CTR);

	if(getScanState() == SS_RUNNING && isScanCounterActive(ctrNum))
	{
		throw UlException(ERR_ALREADY_ACTIVE);
	}

	if(!(mCtrInfo.getCtrMeasurementTypes(ctrNum) & measureType))
		throw UlException(ERR_BAD_CTR_MEASURE_TYPE);

	if((measureMode != CMM_DEFAULT) && (~mCtrInfo.getCtrMeasurementModes(measureType) & measureMode))
		throw UlException(ERR_BAD_CTR_MEASURE_MODE);

	if((measureMode != CMM_DEFAULT) && !(mCtrInfo.getCtrMeasurementModes(measureType) & measureMode))
		throw UlException(ERR_BAD_CTR_MEASURE_MODE);

	if(edgeDetection < CED_RISING_EDGE || edgeDetection > CED_FALLING_EDGE)
		throw UlException(ERR_BAD_EDGE_DETECTION);

	if(debounceMode < CDM_NONE || debounceMode > CDM_TRIGGER_BEFORE_STABLE)
		throw UlException(ERR_BAD_DEBOUNCE_MODE);

	if(debounceMode != CDM_NONE)
	{
		if (debounceTime == CDT_DEBOUNCE_0ns)
			throw UlException(ERR_BAD_DEBOUNCE_TIME);

		std::vector<CounterDebounceTime> debounceTimes = mCtrInfo.getDebounceTimes();

		if(std::find(debounceTimes.begin(), debounceTimes.end(), debounceTime) == debounceTimes.end())
			throw UlException(ERR_BAD_DEBOUNCE_TIME);
	}

	if(measureType == CMT_PERIOD || measureType == CMT_PULSE_WIDTH || measureType == CMT_TIMING)
	{
		std::vector<CounterTickSize> tickSizes = mCtrInfo.getTickSizes();

		if(std::find(tickSizes.begin(), tickSizes.end(), tickSize) == tickSizes.end())
		{
			throw UlException(ERR_BAD_TICK_SIZE);
		}
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void CtrDevice::check_CtrSetTrigger_Args(TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const
{
	if(mCtrInfo.supportsTrigger())
	{
		if(!(mCtrInfo.getTriggerTypes() & trigType))
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mCtrInfo.getScanOptions() & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

//////////////////////          Configuration functions          /////////////////////////////////

void CtrDevice::setCfg_CtrReg(int ctrNum, long long regVal)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}
long long CtrDevice::getCfg_CtrReg(int ctrNum) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}


} /* namespace ul */
