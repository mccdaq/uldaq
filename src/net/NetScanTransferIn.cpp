/*
 * NetScanTransferIn.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "NetScanTransferIn.h"

namespace ul
{

NetScanTransferIn::NetScanTransferIn(const NetDaqDevice& daqDevice) : mNetDevice(daqDevice), mXferState(TS_IDLE)
{
	mIoDevice = NULL;
	mDaqEventHandler = daqDev().eventHandler();

	mXferThreadHandle = 0;
	mTerminateXferThread = false;

	mXferError = ERR_NO_ERROR;

	UlLock::initMutex(mXferThreadHandleMutex, PTHREAD_MUTEX_RECURSIVE);

	mEnabledDaqEvents = (DaqEventType) 0;
	mAvailableCount = 0;
	mCurrentEventCount = 0;
	mNextEventCount = 0;

	mSampleSize = 0;
}

NetScanTransferIn::~NetScanTransferIn()
{
	UlLock::destroyMutex(mXferThreadHandleMutex);
}

void NetScanTransferIn::initilizeTransfer(IoDevice* ioDevice, int sampleSize, int timeout)
{
	UlError err = ERR_NO_ERROR;

	mIoDevice = ioDevice;
	mSampleSize = sampleSize;

	mXferError = ERR_NO_ERROR;
	mXferState = TS_RUNNING;

	// Just in case thread is not terminated
	terminate();

	mXferEvent.reset();

	mEnabledDaqEvents = mDaqEventHandler->getEnabledEventTypes();
	mDaqEventHandler->resetInputEvents(mEnabledDaqEvents);

	if(mEnabledDaqEvents & DE_ON_DATA_AVAILABLE)
	{
		mCurrentEventCount = 0;
		mAvailableCount = mDaqEventHandler->getEventParameter(DE_ON_DATA_AVAILABLE) * mIoDevice->scanChanCount();
		mNextEventCount = mAvailableCount;
	}

	err = daqDev().openDataSocket(timeout);

	if(err)
	{
		mXferState = TS_IDLE;

		throw(UlException(err));
	}

	startXferThread();

}

void NetScanTransferIn::startXferThread()
{
	FnLog log("NetScanTransferIn::startXferThread");

	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(!status)
	{
		mTerminateXferThread = false;
		mXferThreadInitEvent.reset();
		mXferEvent.reset();

		status = pthread_create(&mXferThreadHandle, &attr, &xferThread, this);

#ifndef __APPLE__
		pthread_setname_np(mXferThreadHandle, "net_xfer_in_td");
#endif

		if(status)
			UL_LOG("#### Unable to start the xfer handler thread");
		else
		{
			mXferThreadInitEvent.wait_for_signal(100);
		}

		status = pthread_attr_destroy(&attr);
	}
	else
		UL_LOG("#### Unable to initialize attributes for the xfer handler thread");
}

void* NetScanTransferIn::xferThread(void *arg)
{
	NetScanTransferIn* This = (NetScanTransferIn*) arg;
	This->mXferError = ERR_NO_ERROR;

	UlError err = ERR_NO_ERROR;
	const int MAX_STAGE_SIZE = 16384; // changed from 10240 to 16384 to match max stage size of USB devices for virnet devices
	unsigned char data[MAX_STAGE_SIZE];
	unsigned char partialSampleBuf[8] = {0};
	int partialSampleSize = 0;
	int partialSampleIndex = 0;
	int bytesToProcess;
	int bytesFromLastXfer = 0;
	unsigned int bytesRead = 0;
	bool scanDone = false;

	This->mXferThreadInitEvent.signal();

	This->mXferEvent.wait_for_signal();

	while (!This->mTerminateXferThread && !scanDone)
	{
		if(bytesFromLastXfer) // There was a partial sample from the previous transfer copy it to the beginning of the buffer
			memcpy(data, partialSampleBuf, bytesFromLastXfer);

		err = This->daqDev().readScanData(&data[bytesFromLastXfer], sizeof(data) - bytesFromLastXfer, &bytesRead);

		if(err == ERR_NO_ERROR)
		{
			if(bytesRead > 0)
			{
				bytesToProcess = bytesRead + bytesFromLastXfer;
				bytesFromLastXfer = 0;

				partialSampleSize = bytesToProcess % This->mSampleSize;

				if(partialSampleSize)
				{
					UL_LOG("a packet containing partial sample received");

					bytesToProcess -= partialSampleSize;

					partialSampleIndex = bytesToProcess;
					memcpy(partialSampleBuf, &data[partialSampleIndex], partialSampleSize);

					bytesFromLastXfer = partialSampleSize;
				}

				This->mIoDevice->processScanData(data, bytesToProcess);

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

				if(This->mIoDevice->allScanSamplesTransferred())
					scanDone = true;
			}

		}
		else if(err == ERR_NET_TIMEOUT)
		{
			if(!This->mTerminateXferThread)
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

					break;
				}
			}
		}
		else
		{

		}
	}

	This->daqDev().closeDataSocket();

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

void NetScanTransferIn::start()
{
	mXferEvent.signal();
}


void NetScanTransferIn::terminate()
{
	FnLog log("NetScanTransferIn::terminateXferStateThread");

	UlLock lock(mXferThreadHandleMutex);

	if(mXferThreadHandle)
	{
		mTerminateXferThread = true;

		mXferEvent.signal();

		//daqDev().closeDataSocket();

		UL_LOG("waiting for state thread to complete....");

		pthread_join(mXferThreadHandle, NULL);

		mXferThreadHandle = 0;

		mXferEvent.reset();
	}
}

bool NetScanTransferIn::isDataAvailable(unsigned long long count, unsigned long long current, unsigned long long next)
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


} /* namespace ul */
