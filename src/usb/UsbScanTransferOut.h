/*
 * UsbScanTransferOut.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_USBSCANTRANSFEROUT_H_
#define USB_USBSCANTRANSFEROUT_H_

#include "UsbDaqDevice.h"
#include "../IoDevice.h"
#include "../DaqEventHandler.h"
#include "../utility/ThreadEvent.h"

namespace ul
{

class UL_LOCAL UsbScanTransferOut
{
public:
	UsbScanTransferOut(const UsbDaqDevice& daqDevice);
	virtual ~UsbScanTransferOut();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

	void initilizeTransfers(IoDevice* ioDevice, int endpointAddress, int stageSize);
	void initilizeOnDemandTransfer(IoDevice* ioDevice, int endpointAddress, int stageSize);
	void stopTransfers(bool delay = true);

	inline XferState getXferState() const { return mXferState;}
	inline UlError getXferError() const { return mXferError;}

	void waitForXferStateThread();

	double getStageRate() const { return mStageRate;}

private:
	static void LIBUSB_CALL tarnsferCallback(libusb_transfer* transfer);

	void startXferStateThread();
	static void* xferStateThread(void* arg);
	void terminateXferStateThread();

	void printTransferIndex(libusb_transfer* transfer);

private:
	const UsbDaqDevice&  mUsbDevice;
	IoDevice* mIoDevice;
	double mStageRate;

	pthread_t mXferStateThreadHandle;
	bool mTerminateXferStateThread;
	mutable pthread_mutex_t mXferStateThreadHandleMutex;
	pthread_mutex_t mXferMutex;
	pthread_mutex_t mStopXferMutex;

	int mNumXferPending;
	XferState	mXferState;
	unsigned int mStageSize;
	bool mResubmit;
	bool mNewSamplesSent;

	UlError mXferError;

	ThreadEvent mStateThreadInitEvent;
	ThreadEvent mXferEvent;
	ThreadEvent mXferDoneEvent;

	DaqEventHandler* mDaqEventHandler;
	DaqEventType mEnabledDaqEvents;

public:

	enum {SCAN_PARAM_TCR = 1, SCAN_PARAM_TMR = 2};
	enum {MAX_XFER_COUNT = 32, MAX_STAGE_SIZE = 16384};

	struct
	{
		libusb_transfer* transfer;
		unsigned char buffer[MAX_STAGE_SIZE];
	} mXfer[MAX_XFER_COUNT];
};

} /* namespace ul */

#endif /* USB_USBSCANTRANSFEROUT_H_ */
