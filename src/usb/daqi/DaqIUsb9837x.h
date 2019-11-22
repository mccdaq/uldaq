/*
 * DaqIUsb9837x.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_DAQI_DAQIUSB9837X_H_
#define USB_DAQI_DAQIUSB9837X_H_

#include "DaqIUsbBase.h"
#include "../Usb9837x.h"

namespace ul
{

class UL_LOCAL DaqIUsb9837x: public DaqIUsbBase
{
public:
	DaqIUsb9837x(const UsbDaqDevice& daqDevice);
	virtual ~DaqIUsb9837x();

	const Usb9837x& dtDev() const {return (const Usb9837x&) daqDev();}

	virtual void initialize();

	virtual double daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data);

	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone = NULL) const;

	void overrunOccured() { mOverrunOccurred = true;}

	void resetClockFreq() { mPreviousClockFreq = -1;}

	int syncMode() { return mPreviousSyncMode; }

	void resetSyncMode();


protected:
	void check_DaqInScan_Args_(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data) const;
	virtual void check_DaqInSetTrigger_Args(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) const;
	//void loadScanConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const;
	std::vector<CalCoef> getScanCalCoefs(DaqInChanDescriptor chanDescriptors[], int numChans, DaqInScanFlag flags) const;
	std::vector<CustomScale> getCustomScales(DaqInChanDescriptor chanDescriptors[], int numChans) const;

	virtual void sendStopCmd();



private:

	void configureScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, double rate, ScanOption options);
	void configureClock(DaqInChanDescriptor chanDescriptors[], int numChans, double rate, ScanOption options);
	void configureCGL(DaqInChanDescriptor chanDescriptors[], int numChans);
	void configureFifoPacketSize(int epAddr, double rate, int chanCount, int sampleCount, ScanOption options) const;

	unsigned short getTrigCode(FunctionType functionType, ScanOption options);

	virtual void processScanData32_dbl(libusb_transfer* transfer);
	//virtual void processScanData32_uint64(libusb_transfer* transfer);

public:
	enum { SYNC_MODE_NONE = 0, SYNC_MODE_MASTER = 1, SYNC_MODE_SLAVE = 2};

private:
	enum { FIFO_SIZE = 2 * 1024 * 4 };  // 2K sample FIFO according to DT9837A HW specification, page 4

	enum { INTERNAL = 0, EXTERNAL = 1, DISABLED = 2 };
	//enum { SYNC_MODE_NONE = 0, SYNC_MODE_MASTER = 1, SYNC_MODE_SLAVE = 2};

	bool mVariablAdFifoSize;

	int mPreviousSyncMode;
	double mPreviousClockFreq;

	bool mOverrunOccurred;

	unsigned int mFirstNoneAdcChanIdx;
	unsigned int mGrpDelayTotalSamples;
	unsigned int mGrpDelaySamplesProcessed;

	bool mHasDacChan;
	unsigned int mDacChanIdx;

	struct
	{
		double buf_dbl[8 * Usb9837xDefs::GRP_DELAY_SIZE_IN_SAMPLES]; // this swap buffer is used to retain none A/D samples, 9837B has four none ADC channels, A and C versions have 3
		unsigned int buf_uint32[8 * Usb9837xDefs::GRP_DELAY_SIZE_IN_SAMPLES];
		unsigned int size;
		unsigned int idx;
	}mSwapBuffer;

};

} /* namespace ul */

#endif /* USB_DAQI_DAQIUSB9837X_H_ */
