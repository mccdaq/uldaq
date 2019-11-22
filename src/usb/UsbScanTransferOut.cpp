/*
 * UsbScanTransferOut.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#include "UsbScanTransferOut.h"

#define STAGE_RATE 		0.010

namespace ul
{

UsbScanTransferOut::UsbScanTransferOut(const UsbDaqDevice& daqDevice) : mUsbDevice(daqDevice), mXferState(TS_IDLE)
{
	mIoDevice = NULL;
	mDaqEventHandler = daqDev().eventHandler();

	mStageRate = STAGE_RATE;

	mXferStateThreadHandle = 0;
	mTerminateXferStateThread = false;

	mNumXferPending = 0;
	mStageSize = 0;
	mResubmit = true;
	mNewSamplesSent = false;

	mXferError = ERR_NO_ERROR;

	UlLock::initMutex(mXferMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mXferStateThreadHandleMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mStopXferMutex, PTHREAD_MUTEX_RECURSIVE);

	memset(&mXfer, 0, sizeof(mXfer));

	mEnabledDaqEvents = (DaqEventType) 0;
}

UsbScanTransferOut::~UsbScanTransferOut()
{
	UlLock::destroyMutex(mXferMutex);
	UlLock::destroyMutex(mXferStateThreadHandleMutex);
	UlLock::destroyMutex(mStopXferMutex);
}

void UsbScanTransferOut::initilizeTransfers(IoDevice* ioDevice, int endpointAddress, int stageSize)
{
	UlError err = ERR_NO_ERROR;

	mIoDevice = ioDevice;

	mXferError = ERR_NO_ERROR;
	mStageSize = stageSize;
	mXferState = TS_RUNNING;
	mResubmit = true;
	mNewSamplesSent = false;
	memset(&mXfer, 0, sizeof(mXfer));

	if(mStageSize > MAX_STAGE_SIZE)
		mStageSize = MAX_STAGE_SIZE;

	unsigned int actualStageSize = 0;

	// Just in case thread is not terminated
	terminateXferStateThread();

	int numOfXfers;
	numOfXfers = MAX_XFER_COUNT;

	mXferEvent.reset();
	mXferDoneEvent.reset();

	mEnabledDaqEvents = mDaqEventHandler->getEnabledEventTypes();
	mDaqEventHandler->resetOutputEvents(mEnabledDaqEvents);

	UlLock lock(mXferMutex);

	for(int i = 0; i < numOfXfers; i++)
	{
		mXfer[i].transfer = mUsbDevice.allocTransfer();
		mXfer[i].transfer->buffer = mXfer[i].buffer;

		actualStageSize = mIoDevice->processScanData(mXfer[i].transfer, mStageSize);

		err = mUsbDevice.asyncBulkTransfer(mXfer[i].transfer, endpointAddress, mXfer[i].buffer, actualStageSize, tarnsferCallback, this,  0);

		if(err)
		{
			if(mNumXferPending)
				stopTransfers(false);

			throw(UlException(err));
		}

		mNumXferPending++;

		if(mIoDevice->allScanSamplesTransferred() || !mResubmit)
			break;
	}

	startXferStateThread();
}

void LIBUSB_CALL UsbScanTransferOut::tarnsferCallback(libusb_transfer* transfer)
{
	UsbScanTransferOut* This = (UsbScanTransferOut*)transfer->user_data;

	//This->printTransferIndex(transfer);  // uncomment for debugging

	UlLock lock(This->mXferMutex);

	unsigned int actualStageSize = 0;

	if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
	{
		if(!This->mIoDevice->scanErrorOccurred()) // only DT devices set this to true
		{
			//check if processScanData() has set allScanSamplesTransferred to true, if that's the case then no need to resubmit
			//the request. Also we should not set mNewSamplesSent to true to prevent sending the tmr command
			if(!This->mIoDevice->allScanSamplesTransferred() && This->mResubmit)
			{
				actualStageSize = This->mIoDevice->processScanData(transfer, This->mStageSize);

				transfer->length = actualStageSize;

				libusb_submit_transfer(transfer);

				This->mNewSamplesSent = true;
			}
			else
				This->mNumXferPending--;
		}
		else
			This->mNumXferPending--;
	}
	else
		This->mNumXferPending--;

	if(This->mNumXferPending == 0)
	{
		//This->mTerminateXferStateThread = true;

		// check the status of the last request to see if the device is unplugged
		// do not read transfer->status because the transfer objects are freed in stopTransfers()
		// when mNumXferPending is set to zero

		if(transfer->status == LIBUSB_TRANSFER_ERROR || transfer->status  == LIBUSB_TRANSFER_NO_DEVICE)
		{
			This->mXferError = ERR_DEAD_DEV;
		}

		// Note: Do not access the transfer object beyond here because as soon as mXferState is set to TS_IDLE
		// the stopTransfers() function calls libusb_free_transfer on all transfer and transfer object here in not valid anymore

		This->mXferState = TS_IDLE;

		This->mXferDoneEvent.signal();

	}

	// DT devices will continue receiving data even when scan error occurs, we manually prevent
	// mXferEvent to send signal to the status thread so the wait times out and the status thread performs status check
	if(!This->mIoDevice->scanErrorOccurred()) // only DT devices set this true
		This->mXferEvent.signal();
}

void UsbScanTransferOut::stopTransfers(bool delay)
{
	FnLog log("UsbScanTransferOut::stopTransfers");

	mResubmit = false;

	if(delay)
		usleep(1000);

	UlLock lock(mStopXferMutex);

	for(int i = 0; i < MAX_XFER_COUNT; i++)
	{
		if(mXfer[i].transfer)
			libusb_cancel_transfer(mXfer[i].transfer);
	}

	if(mXferState == TS_RUNNING)
	{
		mXferDoneEvent.wait_for_signal(1000000); // wait up to 1 second
	}


	if(mNumXferPending > 0)
		std::cout << "##### error still xfer pending. mNumXferPending ="   << mNumXferPending << std::endl;


	for(int i = 0; i < MAX_XFER_COUNT; i++)
	{
		if(mXfer[i].transfer)
		{
			libusb_free_transfer(mXfer[i].transfer);
			mXfer[i].transfer = NULL;
		}
	}
}

void UsbScanTransferOut::startXferStateThread()
{
	FnLog log("UsbScanTransferOut::startXferStateThread");

	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(!status)
	{
		mTerminateXferStateThread = false;
		mStateThreadInitEvent.reset();

		status = pthread_create(&mXferStateThreadHandle, &attr, &xferStateThread, this);

#ifndef __APPLE__
		pthread_setname_np(mXferStateThreadHandle, "xfer_out_state_td");
#endif

		if(status)
			UL_LOG("#### Unable to start the event handler thread");
		else
		{
			mStateThreadInitEvent.wait_for_signal(100);
		}

		status = pthread_attr_destroy(&attr);
	}
	else
		UL_LOG("#### Unable to initialize attributes for the event handler thread");
}

void* UsbScanTransferOut::xferStateThread(void *arg)
{
	UsbScanTransferOut* This = (UsbScanTransferOut*) arg;
	//int count = 0;
	unsigned long long timeout = 250000;  // first timeout
	bool scanDone = false;

	int niceVal = 0;  // make sure this thread does not get a high priority if the parent thread is running with high priority
	setpriority(PRIO_PROCESS, 0, niceVal);

	This->mStateThreadInitEvent.signal();

	while (!This->mTerminateXferStateThread)
	{
		if(This->mXferEvent.wait_for_signal(timeout) == ETIMEDOUT) // wait up to 100 ms (initial timeout is 250 ms)
		{
			timeout = 100000;

			UL_LOG("#### retrieving status");

			This->mXferError = This->mIoDevice->checkScanState(&scanDone);

			if(This->mXferError || scanDone)
			{
				This->mIoDevice->terminateScan();

				This->mTerminateXferStateThread = true;

				if((This->mEnabledDaqEvents & DE_ON_OUTPUT_SCAN_ERROR) && This->mXferError)
				{
					This->mDaqEventHandler->setCurrentEventAndData(DE_ON_OUTPUT_SCAN_ERROR, This->mXferError);
				}
				else if((This->mEnabledDaqEvents & DE_ON_END_OF_OUTPUT_SCAN) && scanDone)
				{
					unsigned long long totalScanCount = This->mIoDevice->totalScanSamplesTransferred() / This->mIoDevice->scanChanCount();
					This->mDaqEventHandler->setCurrentEventAndData(DE_ON_END_OF_OUTPUT_SCAN, totalScanCount);
				}
			}
			else if(This->mNewSamplesSent)
			{

				This->mNewSamplesSent = false;
			}
		}
	}

	if(scanDone || This->mXferError)
		This->mIoDevice->setScanState(SS_IDLE);

	This->mIoDevice->signalScanDoneWaitEvent();

	return NULL;
}

void UsbScanTransferOut::terminateXferStateThread()
{
	FnLog log("UsbScanTransferOut::terminateXferStateThread");

	UlLock lock(mXferStateThreadHandleMutex);

	if(mXferStateThreadHandle)
	{
		mTerminateXferStateThread = true;

		mXferEvent.signal();

		UL_LOG("waiting for state thread to complete....");

		pthread_join(mXferStateThreadHandle, NULL);

		mXferStateThreadHandle = 0;

		mXferEvent.reset();
	}
}

void UsbScanTransferOut::waitForXferStateThread()
{
	FnLog log("UsbScanTransferOut::waitForXferStateThread");

	UlLock lock(mXferStateThreadHandleMutex);

	if(mXferStateThreadHandle)
	{
		// The following line is not necessary because when the last request is completed mTerminateXferStateThread is set to true
		// just in case there is an unknown problem and the last request never gets completed the mTerminateXferStateThread is set to true here
		if(!mTerminateXferStateThread)
			mTerminateXferStateThread = true;

		mXferEvent.signal();

		pthread_join(mXferStateThreadHandle, NULL);

		mXferStateThreadHandle = 0;

		mXferEvent.reset();
	}
}


void UsbScanTransferOut::printTransferIndex(libusb_transfer* transfer)
{
	for(int i = 0; i < MAX_XFER_COUNT; i++)
	{
		if(mXfer[i].transfer == transfer)
		{
			std::cout << "Transfer index: " << i << std::endl;
			break;
		}
	}
}


} /* namespace ul */
