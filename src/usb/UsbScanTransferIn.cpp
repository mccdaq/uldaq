/*
 * UsbScanTransferIn.cpp
 *
 *      Author: Measurement Computing Corporation
 */
#include <cstring>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>


#include "UsbScanTransferIn.h"
#include "../utility/UlLock.h"

#define STAGE_RATE 		0.010

namespace ul
{

UsbScanTransferIn::UsbScanTransferIn(const UsbDaqDevice& daqDevice) : mUsbDevice(daqDevice), mXferState(TS_IDLE)
{
	mIoDevice = NULL;
	mDaqEventHandler = daqDev().eventHandler();

	mStageRate = STAGE_RATE;

	mXferStateThreadHandle = 0;
	mTerminateXferStateThread = false;

	mNumXferPending = 0;
	mStageSize = 0;
	mResubmit = true;
	mNewSamplesReceived = false;

	mXferError = ERR_NO_ERROR;

	//UlLock::initMutex(mXferMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mXferStateThreadHandleMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mStopXferMutex, PTHREAD_MUTEX_RECURSIVE);

	memset(&mXfer, 0, sizeof(mXfer));

	mEnabledDaqEvents = (DaqEventType) 0;
	mAvailableCount = 0;
	mCurrentEventCount = 0;
	mNextEventCount = 0;
}

UsbScanTransferIn::~UsbScanTransferIn()
{
	//UlLock::destroyMutex(mXferMutex);
	UlLock::destroyMutex(mXferStateThreadHandleMutex);
	UlLock::destroyMutex(mStopXferMutex);
}

void UsbScanTransferIn::initilizeTransfers(IoDevice* ioDevice, int endpointAddress, int stageSize)
{
	UlError err = ERR_NO_ERROR;

	mIoDevice = ioDevice;

	mXferError = ERR_NO_ERROR;
	mStageSize = stageSize;
	mXferState = TS_RUNNING;
	mResubmit = true;
	mNewSamplesReceived = false;
	memset(&mXfer, 0, sizeof(mXfer));

	if(mStageSize > MAX_STAGE_SIZE)
		mStageSize = MAX_STAGE_SIZE;

	// Just in case thread is not terminated
	terminateXferStateThread();

	int numOfXfers;
	numOfXfers = MAX_XFER_COUNT;

	mXferEvent.reset();
	mXferDoneEvent.reset();

	mEnabledDaqEvents = mDaqEventHandler->getEnabledEventTypes();
	mDaqEventHandler->resetInputEvents(mEnabledDaqEvents);

	if(mEnabledDaqEvents & DE_ON_DATA_AVAILABLE)
	{
		mCurrentEventCount = 0;
		mAvailableCount = mDaqEventHandler->getEventParameter(DE_ON_DATA_AVAILABLE) * mIoDevice->scanChanCount();
		mNextEventCount = mAvailableCount;
	}

	for(int i = 0; i < numOfXfers; i++)
	{
		mXfer[i].transfer = mUsbDevice.allocTransfer();
		err = mUsbDevice.asyncBulkTransfer(mXfer[i].transfer, endpointAddress, mXfer[i].buffer, mStageSize, tarnsferCallback, this,  0);

		if(err)
		{
			if(mNumXferPending)
				stopTransfers();

			throw(UlException(err));
		}

		mNumXferPending++;
	}

	startXferStateThread();

}

void UsbScanTransferIn::initilizeOnDemandTransfer(IoDevice* ioDevice, int endpointAddress, int stageSize)
{
	UlError err = ERR_NO_ERROR;

	mIoDevice = ioDevice;

	mXferError = ERR_NO_ERROR;
	mStageSize = stageSize;
	mXferState = TS_RUNNING;
	mResubmit = true;
	mNewSamplesReceived = false;
	memset(&mXfer, 0, sizeof(mXfer));

	if(mStageSize > MAX_STAGE_SIZE)
		mStageSize = MAX_STAGE_SIZE;

	// Just in case thread is not terminated
	terminateXferStateThread();

	mXferEvent.reset();
	mXferDoneEvent.reset();

	mXfer[0].transfer = mUsbDevice.allocTransfer();
	err =  mUsbDevice.asyncBulkTransfer(mXfer[0].transfer, endpointAddress, mXfer[0].buffer, mStageSize, tarnsferCallback, this,  0);

	if(err)
		throw(UlException(err));

	mNumXferPending++;
}

void LIBUSB_CALL UsbScanTransferIn::tarnsferCallback(libusb_transfer* transfer)
{
	UsbScanTransferIn* This = (UsbScanTransferIn*)transfer->user_data;

	//This->printTransferIndex(transfer);  // uncomment for debugging

	//UlLock lock(This->mXferMutex);  //uncomment if needed

	if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
	{
		if(!This->mIoDevice->scanErrorOccurred()) // only DT devices set this to true
		{
			if(!This->mIoDevice->allScanSamplesTransferred() && This->mResubmit)
			{
				This->mIoDevice->processScanData(transfer);

				unsigned long long samplesTransfered = This->mIoDevice->totalScanSamplesTransferred();

				if(This->mEnabledDaqEvents & DE_ON_DATA_AVAILABLE)
				{
					if(isDataAvailable(samplesTransfered, This->mCurrentEventCount, This->mNextEventCount))
					{
						This->mCurrentEventCount = samplesTransfered;
						This->mNextEventCount = This->mCurrentEventCount + This->mAvailableCount;
						This->mDaqEventHandler->setCurrentEventAndData(DE_ON_DATA_AVAILABLE, This->mCurrentEventCount / This->mIoDevice->scanChanCount());
					}
				}
			}
		}

		//check if processScanData() has set allScanSamplesTransferred to true, if that's the case then no need to resubmit
		//the request. Also we should not set mNewSamplesReceived to true to prevent sending the tmr command
		if(!This->mIoDevice->allScanSamplesTransferred() && This->mResubmit)
		{
			libusb_submit_transfer(transfer);

			This->mNewSamplesReceived = true;
		}
		else
			This->mNumXferPending--;
	}
	else
		This->mNumXferPending--;

	if(This->mNumXferPending == 0)
	{
		//This->terminateXferStateThread();
		This->mTerminateXferStateThread = true;

		// check the status of the last request to see if the device is unplugged
		// do not read transfer->status because the transfer objects are freed in stopTransfers()
		// when mNumXferPending is set to zero

		if(transfer->status == LIBUSB_TRANSFER_ERROR || transfer->status  == LIBUSB_TRANSFER_NO_DEVICE)
		{
			This->mXferError = ERR_DEAD_DEV;
		}

		// Note: Do not access the transfer object beyond here, because as soon as mXferState is set to TS_IDLE
		// the stopTransfers() function calls libusb_free_transfer on all transfers and transfer objects here are not valid anymore

		This->mXferState = TS_IDLE;

		This->mXferDoneEvent.signal();

		/*if((This->mEnabledDaqEvents & DE_ON_END_OF_INPUT_SCAN) && This->mIoDevice->allScanSamplesTransferred())
		{
			This->mDaqEventHandler->setCurrentEventAndData(DE_ON_END_OF_INPUT_SCAN, 0);
		}*/
	}

	// DT devices will continue sending data even when scan error occurs, we manually prevent
	// mXferEvent to send signal to the status thread so the wait times out and the status thread performs status check
	if(!This->mIoDevice->scanErrorOccurred()) // only DT devices set this true
		This->mXferEvent.signal();
}

void UsbScanTransferIn::stopTransfers()
{
	FnLog log("UsbScanTransferIn::stopTransfers");

	mResubmit = false;
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

void UsbScanTransferIn::startXferStateThread()
{
	FnLog log("UsbScanTransferIn::startXferStateThread");

	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(!status)
	{
		mTerminateXferStateThread = false;
		mStateThreadInitEvent.reset();

		status = pthread_create(&mXferStateThreadHandle, &attr, &xferStateThread, this);

#ifndef __APPLE__
		pthread_setname_np(mXferStateThreadHandle, "xfer_in_state_td");
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

void* UsbScanTransferIn::xferStateThread(void *arg)
{
	UsbScanTransferIn* This = (UsbScanTransferIn*) arg;
	int count = 0;
	unsigned long long timeout = 250000;  // first timeout

	int niceVal = 0;  // make sure this thread does not get a high priority if the parent thread is running with high priority
	setpriority(PRIO_PROCESS, 0, niceVal);

	This->mStateThreadInitEvent.signal();

	while (!This->mTerminateXferStateThread)
	{
		while(This->mXferEvent.wait_for_signal(timeout) != ETIMEDOUT) // wait up to 100 ms (initial timeout is 250 ms)
		{
			timeout = 100000;

			if(!This->mTerminateXferStateThread)
			{
				if(!This->mIoDevice->recycleMode() && This->mIoDevice->allScanSamplesTransferred())
				{
					This->mIoDevice->terminateScan();

					This->mTerminateXferStateThread = true;
					break;
				}

				count++;
				if(count == 100)
				{
					// this function updates scan parameters such as TCR value for NI devices
					This->mIoDevice->updateScanParam(SCAN_PARAM_TCR);

					count = 0;
				}

				if(This->mNewSamplesReceived)
				{
					This->mIoDevice->updateScanParam(SCAN_PARAM_TMR);
					This->mNewSamplesReceived = false;
				}
			}
			else
				break;
		}

		if(!This->mTerminateXferStateThread)
		{
			UL_LOG("#### retrieving status");

			This->mXferError = This->mIoDevice->checkScanState();

			if(This->mXferError)
			{
				if(This->mEnabledDaqEvents & DE_ON_INPUT_SCAN_ERROR)
				{
					This->mDaqEventHandler->setCurrentEventAndData(DE_ON_INPUT_SCAN_ERROR, This->mXferError);
				}

				This->mIoDevice->terminateScan();
			}
			else
			{
				// this function updates scan parameters such as TCR value for NI devices
				This->mIoDevice->updateScanParam(SCAN_PARAM_TCR);

				if(This->mNewSamplesReceived)
				{
					This->mIoDevice->updateScanParam(SCAN_PARAM_TMR);
					This->mNewSamplesReceived = false;
				}
			}
		}

	}

	// if scan stop is not initiated by the users, i.e. when scan is in finite mode and all samples received or
	// an error occurred we need to set scan status here
	if(This->mIoDevice->allScanSamplesTransferred() || This->mXferError)
	{
		This->mIoDevice->setScanState(SS_IDLE);
	}

	if((This->mEnabledDaqEvents & DE_ON_END_OF_INPUT_SCAN) && This->mIoDevice->allScanSamplesTransferred())
	{
		unsigned long long totalScanCount = This->mIoDevice->totalScanSamplesTransferred() / This->mIoDevice->scanChanCount();
		This->mDaqEventHandler->setCurrentEventAndData(DE_ON_END_OF_INPUT_SCAN, totalScanCount);
	}

	This->mIoDevice->signalScanDoneWaitEvent();

	return NULL;
}

void UsbScanTransferIn::terminateXferStateThread()
{
	FnLog log("UsbScanTransferIn::terminateXferStateThread");

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

void UsbScanTransferIn::waitForXferStateThread()
{
	FnLog log("UsbScanTransferIn::waitForXferStateThread");

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

bool UsbScanTransferIn::isDataAvailable(unsigned long long count, unsigned long long current, unsigned long long next)
{
	bool available = false;

	if (count == next)
		available = true;
	else if (count > next)
	{
		if (next > current || ((next < current) && (count < current)))//both next and count wrapped
			available = true;
	}
	else if (count < current) //count has wrapped around 64-bit limit
	{
		if ((count < next) && (next > current)) //while next hasn't
			available = true;
	}

	return available;
}


void UsbScanTransferIn::printTransferIndex(libusb_transfer* transfer)
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
