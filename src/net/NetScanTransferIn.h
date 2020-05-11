/*
 * NetScanTransferIn.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_NETSCANTRANSFERIN_H_
#define NET_NETSCANTRANSFERIN_H_

#include "NetDaqDevice.h"
#include "../IoDevice.h"
#include "../DaqEventHandler.h"
#include "../utility/ThreadEvent.h"

namespace ul
{

class UL_LOCAL NetScanTransferIn
{
public:
	NetScanTransferIn(const NetDaqDevice& daqDevice);
	virtual ~NetScanTransferIn();

	const NetDaqDevice& daqDev() const {return mNetDevice;}

	void initilizeTransfer(IoDevice* ioDevice, int sampleSize, int timeout);
	void start();

	void terminate();

	inline UlError getXferError() const { return mXferError;}

private:
	void startXferThread();
	static void* xferThread(void* arg);

	static bool isDataAvailable(unsigned long long count, unsigned long long current, unsigned long long next);

private:
	const NetDaqDevice&  mNetDevice;
	IoDevice* mIoDevice;

	pthread_t mXferThreadHandle;
	bool mTerminateXferThread;
	mutable pthread_mutex_t mXferThreadHandleMutex;

	UlError 	mXferError;
	XferState	mXferState;

	ThreadEvent mXferThreadInitEvent;
	ThreadEvent mXferEvent;

	DaqEventHandler* mDaqEventHandler;
	DaqEventType mEnabledDaqEvents;

	unsigned long long mAvailableCount;
	unsigned long long mCurrentEventCount;
	unsigned long long mNextEventCount;

	int mSampleSize;
};

} /* namespace ul */

#endif /* NET_NETSCANTRANSFERIN_H_ */
