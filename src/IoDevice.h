/*
 * IoDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef IODEVICE_H_
#define IODEVICE_H_

#include <vector>

#include "DaqDevice.h"
#include "./utility/Endian.h"
#include "./utility/UlLock.h"
#include "./utility/ThreadEvent.h"

namespace ul
{

class UL_LOCAL IoDevice
{
public:
	IoDevice(const DaqDevice& daqDevice);
	virtual ~IoDevice();

	virtual void initialize() {};
	virtual void reconfigure() {};
	virtual void disconnect();

	virtual void setScanState(ScanStatus state);
	virtual ScanStatus getScanState() const;

	void setActualScanRate(double rate);
	double actualScanRate() const;

	virtual void stopBackground() {};
	virtual UlError terminateScan() {return ERR_BAD_DEV_TYPE;}
	virtual UlError checkScanState(bool* scanDone = NULL) const { return ERR_BAD_DEV_TYPE;}

	virtual void updateScanParam(int param)  {};
	virtual void processScanData(void* transfer) {};
	virtual unsigned int processScanData(void* transfer, unsigned int stageSize) { return 0;}

	//void getXferStatus(unsigned long long* currentScanCount, unsigned long long* currentTotalCount, long long* currentIndex) const;
	void getXferStatus(TransferStatus* xferStatus) const;
	inline bool allScanSamplesTransferred() const { return mScanInfo.allSamplesTransferred; }
	inline bool recycleMode() const { return mScanInfo.recycle; }
	inline unsigned int scanChanCount() const { return mScanInfo.chanCount; }
	inline unsigned long long totalScanSamplesTransferred() const { return mScanInfo.totalSampleTransferred; }

	TriggerConfig getTrigConfig() const { return mTrigCfg;}

	virtual UlError wait(WaitType waitType, long long waitParam, double timeout);
	virtual UlError waitUntilDone(double timeout);
	void signalScanDoneWaitEvent() { mScanDoneWaitEvent.signal();}

	bool scanErrorOccurred() { return mScanErrorFlag; }
	void setscanErrorFlag() { mScanErrorFlag = true;}
	void resetScanErrorFlag() { mScanErrorFlag = false; }

protected:
	void setScanInfo(FunctionType functionType, int chanCount, int samplesPerChanCount, int sampleSize, unsigned int analogResolution, ScanOption options, long long flags, std::vector<CalCoef> calCoefs, std::vector<CustomScale> customScales, void* dataBuffer);
	void setScanInfo(FunctionType functionType, int chanCount, int samplesPerChanCount, int sampleSize, unsigned int analogResolution, ScanOption options, long long flags, std::vector<CalCoef> calCoefs, void* dataBuffer);
	unsigned int calcPacerPeriod(double rate, ScanOption options);

protected:
	const DaqDevice& mDaqDevice;
	pthread_mutex_t mIoDeviceMutex;
	int mMinScanSampleCount;

	mutable pthread_mutex_t mProcessScanDataMutex;

	enum {MAX_CHAN_COUNT = 128};

	struct
	{
		FunctionType functionType;
		unsigned int chanCount;
		unsigned int samplesPerChanCount;
		unsigned int sampleSize;
		long long flags;
		bool recycle;
		unsigned long long fullScale;  // analog channels only
		CalCoef calCoefs[MAX_CHAN_COUNT];
		CustomScale customScales[MAX_CHAN_COUNT];
		unsigned long long dataBufferSize;
		void* dataBuffer;
		ScanDataBufferType	dataBufferType;
		unsigned int currentCalCoefIdx;
		unsigned long long currentDataBufferIdx;
		unsigned long long totalSampleTransferred;
		bool allSamplesTransferred;
		bool stoppingScan;
	} mScanInfo;

	TriggerConfig mTrigCfg;

public:
	Endian& mEndian;

private:
	ScanStatus mScanState;
	double mActualScanRate;
	ThreadEvent mScanDoneWaitEvent;
	bool mScanErrorFlag; // added for DT devices since scan error is obtained through a poll method
};

} /* namespace ul */

#endif /* IODEVICE_H_ */
