/*
 * IoDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */
#include <limits.h>

#include "IoDevice.h"
#include "UlException.h"

namespace ul
{

IoDevice::IoDevice(const DaqDevice& daqDevice): mDaqDevice(daqDevice), mMinScanSampleCount(1), mEndian(Endian::Instance()), mScanState(SS_IDLE), mActualScanRate(0.0)
{
	memset(&mScanInfo, 0, sizeof(mScanInfo));

	memset(&mTrigCfg, 0, sizeof(mTrigCfg));
	mTrigCfg.type = TRIG_POS_EDGE;

	mScanErrorFlag = false; //only used by DT devices

	UlLock::initMutex(mIoDeviceMutex, PTHREAD_MUTEX_RECURSIVE);

	UlLock::initMutex(mProcessScanDataMutex, PTHREAD_MUTEX_RECURSIVE);
}

IoDevice::~IoDevice()
{
	UlLock::destroyMutex(mIoDeviceMutex);
	UlLock::destroyMutex(mProcessScanDataMutex);
}

void IoDevice::disconnect()
{
	try
	{
		if(getScanState() == SS_RUNNING)
			stopBackground();
	}
	catch(UlException& e)
	{
		//UL_LOG("Ul exception occurred: " << e.what());
	}
}

void IoDevice::setScanState(ScanStatus state)
{
	mScanState = state;
}

ScanStatus IoDevice::getScanState() const
{
	return mScanState;
}

void IoDevice::setActualScanRate(double rate)
{
	mActualScanRate = rate;
}

double IoDevice::actualScanRate() const
{
	return mActualScanRate;
}

void IoDevice::setScanInfo(FunctionType functionType, int chanCount, int samplesPerChanCount, int sampleSize, unsigned int analogResolution, ScanOption options, long long flags, std::vector<CalCoef> calCoefs, std::vector<CustomScale> customScales, void* dataBuffer)
{
	if(mScanState == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	ScanDataBufferType dataBufferType = DATA_DBL;

	if(functionType == FT_DI || functionType == FT_DO || functionType == FT_CTR)
		dataBufferType = DATA_UINT64;

	mScanInfo.functionType = functionType;
	mScanInfo.chanCount = chanCount;
	mScanInfo.samplesPerChanCount = samplesPerChanCount;
	mScanInfo.sampleSize = sampleSize;
	//mScanInfo.sampleBitness = sampleBitness;
	mScanInfo.flags = flags;
	std::copy(calCoefs.begin(), calCoefs.end(), mScanInfo.calCoefs);
	std::copy(customScales.begin(), customScales.end(), mScanInfo.customScales);
	mScanInfo.recycle = options & SO_CONTINUOUS ? true : false;
	mScanInfo.dataBuffer = dataBuffer;
	mScanInfo.dataBufferType = dataBufferType;
	mScanInfo.fullScale =  (1ULL << analogResolution) - 1;
	mScanInfo.dataBufferSize = mScanInfo.chanCount * mScanInfo.samplesPerChanCount;
	mScanInfo.stoppingScan = false;

	mScanDoneWaitEvent.reset();

	UlLock lock(mProcessScanDataMutex);
	mScanInfo.currentCalCoefIdx = 0;
	mScanInfo.currentDataBufferIdx = 0;
	mScanInfo.totalSampleTransferred = 0;
	mScanInfo.allSamplesTransferred = false;
}
void IoDevice::setScanInfo(FunctionType functionType, int chanCount, int samplesPerChanCount, int sampleSize, unsigned int analogResolution, ScanOption options, long long flags, std::vector<CalCoef> calCoefs, void* dataBuffer)
{
	std::vector<CustomScale> customScales;

	setScanInfo(functionType, chanCount, samplesPerChanCount, sampleSize, analogResolution, options, flags, calCoefs, customScales, dataBuffer);
}

void IoDevice::getXferStatus(TransferStatus* xferStatus) const
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransfered is not updated atomically

	if(mScanInfo.totalSampleTransferred == 0)  // if scan never ran since ul is loaded
	{
		xferStatus->currentIndex = -1;
		xferStatus->currentTotalCount = 0;
		xferStatus->currentScanCount = 0;
	}
	else
	{
		//unsigned long long count = mScanInfo.totalSampleTransferred; // return current sample count
		//unsigned long long idx = -1;

		if(mScanInfo.chanCount > 0 && mScanInfo.totalSampleTransferred >= mScanInfo.chanCount)
		{
			unsigned long long idx = mScanInfo.totalSampleTransferred;
			idx -= (idx % mScanInfo.chanCount);
			idx -= mScanInfo.chanCount;
			idx = idx % mScanInfo.dataBufferSize;

			xferStatus->currentIndex = idx;
			xferStatus->currentTotalCount = mScanInfo.totalSampleTransferred;
			xferStatus->currentScanCount = mScanInfo.totalSampleTransferred / mScanInfo.chanCount;
		}
		else
		{
			xferStatus->currentIndex = -1;
			xferStatus->currentTotalCount = mScanInfo.totalSampleTransferred;
			xferStatus->currentScanCount = 0;
		}
	}
}
/*
void IoDevice::getXferStatus(unsigned long long* currentScanCount, unsigned long long* currentTotalCount, long long* currentIndex) const
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransfered is not updated atomically

	if(mScanInfo.totalSampleTransferred == 0)  // if scan never ran since ul is loaded
	{
		*currentIndex = -1;
		*currentTotalCount = 0;
		*currentScanCount = 0;
	}
	else
	{
		unsigned long long count = mScanInfo.totalSampleTransferred; // return current sample count
		unsigned long long idx = -1;

		if(mScanInfo.chanCount > 0 && count >= mScanInfo.chanCount)
		{
			idx = count;

			idx -= (idx % mScanInfo.chanCount);

			idx -= mScanInfo.chanCount;

			idx = idx % mScanInfo.dataBufferSize;
		}

		*currentIndex = idx;
		*currentTotalCount = count;
		*currentScanCount = count / mScanInfo.chanCount;
	}
}*/

unsigned int IoDevice::calcPacerPeriod(double rate, ScanOption options)
{
	unsigned int period = 0;
	if(!(options & SO_EXTCLOCK))
	{
		double clockFreq = mDaqDevice.getClockFreq();
		double periodDbl = clockFreq / rate;

		if (periodDbl > 0)
			--periodDbl;

		if(periodDbl > UINT_MAX)
			periodDbl = UINT_MAX;

		period = periodDbl;

		double actualrate = clockFreq / (1ULL + period);

		setActualScanRate(actualrate);
	}
	else
	{
		period = 0;
		setActualScanRate(rate);
	}

	return period;
}

UlError IoDevice::wait(WaitType waitType, long long waitParam, double timeout)
{
	UlError err = ERR_NO_ERROR;

	if(waitType == WAIT_UNTIL_DONE)
	{
		err = waitUntilDone( timeout);
	}

	return err;
}

UlError IoDevice::waitUntilDone(double timeout)
{
	UlError err = ERR_NO_ERROR;

	if(mScanState == SS_RUNNING)
	{
		unsigned long long timeout_us = timeout * 1000000;
		int ret = 0;

		if(timeout > 0)
		{
			ret = mScanDoneWaitEvent.wait_for_signal(timeout_us);
		}
		else if(timeout == -1)
		{
			mScanDoneWaitEvent.wait_for_signal();
		}

		if(ret == ETIMEDOUT)
			err = ERR_TIMEDOUT;
	}

	return err;
}

/*unsigned int IoDevice::readScanDataDbl(double* readArray, unsigned int samplesPerChanCount, int fillMode, double timeout)
{
	unsigned int samplesPerChanRead = 0;


	return samplesPerChanRead;
}*/

} /* namespace ul */
