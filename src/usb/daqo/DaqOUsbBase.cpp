/*
 * DaqOUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqOUsbBase.h"

namespace ul
{
DaqOUsbBase::DaqOUsbBase(const UsbDaqDevice& daqDevice) : DaqODevice(daqDevice), mUsbDevice(daqDevice)
{
	mScanEndpointAddr = 0;
	mScanStopCmd = 0;

	mTransferMode = SO_BLOCKIO;
	memset(mChanDescs, 0 ,sizeof(mChanDescs));
}

DaqOUsbBase::~DaqOUsbBase()
{

}

UlError DaqOUsbBase::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	UlError err = ERR_NO_ERROR;

	if(status && xferStatus)
	{
		ScanStatus scanStatus = getScanState();

		getXferStatus(xferStatus);

		if(scanStatus != SS_RUNNING)
			err = daqDev().scanTranserOut()->getXferError();

		*status = scanStatus;
	}
	else
		err = ERR_BAD_ARG;

	return err;
}

void DaqOUsbBase::sendStopCmd()
{
	unsigned char cmd = getScanStopCmd();

	daqDev().sendCmd(cmd);
}

UlError DaqOUsbBase::terminateScan()
{
	UlError err = ERR_NO_ERROR;

	unsigned char cmd = getScanStopCmd();

	try
	{
		daqDev().sendCmd(cmd);
	}
	catch(UlException& e)
	{
		err = e.getError();
	}
	catch(...)
	{
		err = ERR_UNHANDLED_EXCEPTION;
	}

	daqDev().scanTranserOut()->stopTransfers();

	storeLastStatus();

	return err;
}

void DaqOUsbBase::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserOut()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError DaqOUsbBase::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned char cmd = daqDev().getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned short status =0;

	try
	{
		daqDev().queryCmd(cmd, 0, 0, (unsigned char*)&status, sizeof(status));

		if((status & daqDev().getScanDoneBitMask()) || !(status & daqDev().getScanRunningBitMask(SD_OUTPUT)))
			*scanDone = true;

		if(status & daqDev().getUnderrunBitMask())
		{
			err = ERR_UNDERRUN;
		}
	}
	catch(UlException& e)
	{
		err = e.getError();
	}
	catch(...)
	{
		err = ERR_UNHANDLED_EXCEPTION;
	}

	return err;
}

int DaqOUsbBase::calcStageSize(int epAddr, double rate, int chanCount, int sampleCount, int sampleSize) const
{
	int stageSize = 0;
	int minStageSize = mUsbDevice.getBulkEndpointMaxPacketSize(epAddr);

	if (getTransferMode() == SO_SINGLEIO)
	{
		stageSize = chanCount * sampleSize;
	}
	else
	{
		double aggRate =  chanCount * rate * sampleSize; // bytes per second
		long long bufferBytesCount = (long long) sampleCount * sampleSize;
		double stageRate = daqDev().scanTranserOut()->getStageRate();
		stageSize = (int)(aggRate * stageRate);

		if(stageSize % minStageSize != 0)
			stageSize = stageSize + minStageSize - (stageSize % minStageSize);

		if(stageSize > bufferBytesCount)
			stageSize = (int)(bufferBytesCount - (bufferBytesCount % minStageSize));

		if (stageSize < minStageSize)
			stageSize = minStageSize;

		if(stageSize > UsbScanTransferOut::MAX_STAGE_SIZE)
			stageSize = UsbScanTransferOut::MAX_STAGE_SIZE;
	}

	return stageSize;
}

void DaqOUsbBase::setTransferMode(long long scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if(!(scanOptions & SO_BURSTIO))
	{
		if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 1000.0))
			mTransferMode = SO_SINGLEIO;
	}
}

int DaqOUsbBase::getTransferMode() const
{
	return mTransferMode;
}

void DaqOUsbBase::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int DaqOUsbBase::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

void DaqOUsbBase::setChanDescriptors(DaqOutChanDescriptor* chanDescs, int chanCount)
{
	memset(mChanDescs, 0 ,sizeof(mChanDescs));

	memcpy(mChanDescs, chanDescs, chanCount * sizeof(DaqOutChanDescriptor));
}

unsigned int DaqOUsbBase::processScanData(void* transfer, unsigned int stageSize)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;

	unsigned int actualStageSize = 0;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		if(mScanInfo.dataBufferType == DATA_UINT64)
			actualStageSize = processScanData16_uint64(usbTransfer, stageSize);
		else
			actualStageSize = processScanData16_dbl(usbTransfer, stageSize);
		break;
	case 4:  // 4 bytes
		if(mScanInfo.dataBufferType == DATA_UINT64)
			actualStageSize = processScanData32_uint64(usbTransfer, stageSize);
		else
			actualStageSize = processScanData32_dbl(usbTransfer, stageSize);
		break;
	case 8:  // 8 bytes
		actualStageSize = processScanData64_uint64(usbTransfer, stageSize);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}

	return actualStageSize;
}

unsigned int DaqOUsbBase::processScanData16_dbl(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	double data;
	double calCount;
	//long rawVal;
	unsigned short count;
	double* dataBuffer = (double*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
		{
			if(mChanDescs[mScanInfo.currentCalCoefIdx].type == DAQO_ANALOG)
			{
				if(data > mScanInfo.fullScale)
					data = mScanInfo.fullScale;
				else if(data < 0)
					data = 0;
			}

			count = data;
		}
		else
		{
			if(mChanDescs[mScanInfo.currentCalCoefIdx].type == DAQO_ANALOG)
			{
				calCount = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;

				if(calCount > mScanInfo.fullScale)
					calCount = mScanInfo.fullScale;
				else if(calCount < 0)
					calCount = 0;

				count = calCount;

			}
			else
				count = data;
		}

		buffer[numOfSampleCopied] = Endian::cpu_to_le_ui16(count);

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}

	actualStageSize = numOfSampleCopied * mScanInfo.sampleSize;

	return actualStageSize;
}

unsigned int DaqOUsbBase::processScanData16_uint64(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	unsigned long long data;
	unsigned short count;
	unsigned long long* dataBuffer = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			count = data;
		else
		{
			count = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
		}

		buffer[numOfSampleCopied] = Endian::cpu_to_le_ui16(count);

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}

	actualStageSize = numOfSampleCopied * mScanInfo.sampleSize;

	return actualStageSize;
}

unsigned int DaqOUsbBase::processScanData32_dbl(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	double data;
	double calCount;
	//long rawVal;
	unsigned int count;
	double* dataBuffer = (double*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
		{
			if(mChanDescs[mScanInfo.currentCalCoefIdx].type == DAQO_ANALOG)
			{
				if(data > mScanInfo.fullScale)
					data = mScanInfo.fullScale;
				else if(data < 0)
					data = 0;
			}

			count = data;
		}
		else
		{
			if(mChanDescs[mScanInfo.currentCalCoefIdx].type == DAQO_ANALOG)
			{
				calCount = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;

				if(calCount > mScanInfo.fullScale)
					calCount = mScanInfo.fullScale;
				else if(calCount < 0)
					calCount = 0;

				count = calCount;

			}
			else
				count = data;
		}

		buffer[numOfSampleCopied] = Endian::cpu_to_le_ui32(count);

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}

	actualStageSize = numOfSampleCopied * mScanInfo.sampleSize;

	return actualStageSize;
}

unsigned int DaqOUsbBase::processScanData32_uint64(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	unsigned long long data;
	//long rawVal;
	unsigned int count;
	unsigned long long* dataBuffer = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			count = data;
		else
		{
			count = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
		}

		buffer[numOfSampleCopied] = Endian::cpu_to_le_ui32(count);

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}

	actualStageSize = numOfSampleCopied * mScanInfo.sampleSize;

	return actualStageSize;
}

unsigned int DaqOUsbBase::processScanData64_uint64(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned long long* buffer = (unsigned long long*)transfer->buffer;

	unsigned long long count;
	unsigned long long* dataBuffer = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		count = dataBuffer[mScanInfo.currentDataBufferIdx];

		buffer[numOfSampleCopied] = Endian::cpu_to_le_ui64(count);

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}

	actualStageSize = numOfSampleCopied * mScanInfo.sampleSize;

	return actualStageSize;
}

} /* namespace ul */
