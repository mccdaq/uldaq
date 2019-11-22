/*
 * UsbScanTransferIn.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USBSCANTRANSFERIN_H_
#define USBSCANTRANSFERIN_H_

#include "UsbDaqDevice.h"
#include "../IoDevice.h"
#include "../DaqEventHandler.h"
#include "../utility/ThreadEvent.h"

namespace ul
{

class UL_LOCAL UsbScanTransferIn
{
public:
	UsbScanTransferIn(const UsbDaqDevice& daqDevice);
	virtual ~UsbScanTransferIn();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

	void initilizeTransfers(IoDevice* ioDevice, int endpointAddress, int stageSize);
	void initilizeOnDemandTransfer(IoDevice* ioDevice, int endpointAddress, int stageSize);
	void stopTransfers();

	inline XferState getXferState() const { return mXferState;}
	inline UlError getXferError() const { return mXferError;}

	void waitForXferStateThread();

	double getStageRate() const { return mStageRate;}

private:
	static void LIBUSB_CALL tarnsferCallback(libusb_transfer* transfer);

	void startXferStateThread();
	static void* xferStateThread(void* arg);
	void terminateXferStateThread();

	static bool isDataAvailable(unsigned long long count, unsigned long long current, unsigned long long next);

	void printTransferIndex(libusb_transfer* transfer);

private:
	const UsbDaqDevice&  mUsbDevice;
	IoDevice* mIoDevice;
	double mStageRate;

	pthread_t mXferStateThreadHandle;
	bool mTerminateXferStateThread;
	mutable pthread_mutex_t mXferStateThreadHandleMutex;
	//pthread_mutex_t mXferMutex;
	pthread_mutex_t mStopXferMutex;

	int mNumXferPending;
	XferState	mXferState;
	unsigned int mStageSize;
	bool mResubmit;
	bool mNewSamplesReceived;

	UlError mXferError;

	ThreadEvent mStateThreadInitEvent;
	ThreadEvent mXferEvent;
	ThreadEvent mXferDoneEvent;

	DaqEventHandler* mDaqEventHandler;
	DaqEventType mEnabledDaqEvents;

	unsigned long long mAvailableCount;
	unsigned long long mCurrentEventCount;
	unsigned long long mNextEventCount;

public:
	//static const float STAGE_RATE = 0.010;

	enum {SCAN_PARAM_TCR = 1, SCAN_PARAM_TMR = 2};
	enum {MAX_XFER_COUNT = 32, MAX_STAGE_SIZE = 16384};

	struct
	{
		libusb_transfer* transfer;
		unsigned char buffer[MAX_STAGE_SIZE];
	} mXfer[MAX_XFER_COUNT];

};

} /* namespace ul */

#endif /* USBSCANTRANSFERIN_H_ */
