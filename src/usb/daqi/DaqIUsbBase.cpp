/*
 * DaqIUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqIUsbBase.h"
#include "../../CtrDevice.h"

namespace ul
{

DaqIUsbBase::DaqIUsbBase(const UsbDaqDevice& daqDevice) : DaqIDevice(daqDevice), mUsbDevice(daqDevice)
{
	mScanEndpointAddr = 0;
	mScanStopCmd = 0;

	mTransferMode = SO_BLOCKIO;
}

DaqIUsbBase::~DaqIUsbBase()
{

}

UlError DaqIUsbBase::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	UlError err = ERR_NO_ERROR;

	if(status && xferStatus)
	{
		ScanStatus scanStatus = getScanState();

		getXferStatus(xferStatus);

		if(scanStatus != SS_RUNNING)
			err = daqDev().scanTranserIn()->getXferError();

		*status = scanStatus;
	}
	else
		err = ERR_BAD_ARG;

	return err;
}

void DaqIUsbBase::sendStopCmd()
{
	unsigned char cmd = getScanStopCmd();

	daqDev().sendCmd(cmd);
}

UlError DaqIUsbBase::terminateScan()
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

	daqDev().scanTranserIn()->stopTransfers();

	storeLastStatus();

	if(daqDev().ctrDevice())
		daqDev().ctrDevice()->setScanCountersInactive();

	return err;
}

void DaqIUsbBase::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserIn()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError DaqIUsbBase::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned char cmd = daqDev().getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned short status =0;

	try
	{
		daqDev().queryCmd(cmd, 0, 0, (unsigned char*)&status, sizeof(status));

		if(status & daqDev().getOverrunBitMask())
		{
			err = ERR_OVERRUN;
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

int DaqIUsbBase::calcStageSize(int epAddr, double rate, int chanCount, int sampleCount, int sampleSize) const
{
	int stageSize = 0;
	int minStageSize = mUsbDevice.getBulkEndpointMaxPacketSize(epAddr);

	if (getTransferMode() == SO_SINGLEIO)
	{
		stageSize = minStageSize;
	}
	else
	{
		double aggRate =  chanCount * rate * sampleSize; // bytes per second
		long long bufferBytesCount = (long long) sampleCount * sampleSize;
		double stageRate = daqDev().scanTranserIn()->getStageRate();
		stageSize = (int)(aggRate * stageRate);

		if(stageSize % minStageSize != 0)
			stageSize = stageSize + minStageSize - (stageSize % minStageSize);

		if(stageSize > bufferBytesCount)
			stageSize = (int)(bufferBytesCount - (bufferBytesCount % minStageSize));

		if (stageSize < minStageSize)
			stageSize = minStageSize;

		if(stageSize > UsbScanTransferIn::MAX_STAGE_SIZE)
			stageSize = UsbScanTransferIn::MAX_STAGE_SIZE;
	}

	return stageSize;
}

void DaqIUsbBase::setTransferMode(long long scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if(!(scanOptions & SO_BURSTIO))
	{
		if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 1000.0))
			mTransferMode = SO_SINGLEIO;
	}
}

int DaqIUsbBase::getTransferMode() const
{
	return mTransferMode;
}

void DaqIUsbBase::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int DaqIUsbBase::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

void DaqIUsbBase::processScanData(void* transfer)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		if(mScanInfo.dataBufferType == DATA_UINT64)
			processScanData16_uint64(usbTransfer);
		else
			processScanData16_dbl(usbTransfer);
		break;
	case 4:  // 4 bytes
		if(mScanInfo.dataBufferType == DATA_UINT64)
			processScanData32_uint64(usbTransfer);
		else
			processScanData32_dbl(usbTransfer);
		break;
	case 8:  // 8 bytes
			processScanData64_uint64(usbTransfer);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}
}

void DaqIUsbBase::processScanData16_dbl(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	double data;
	unsigned int rawVal;
	double* dataBuf = (double*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui16_to_cpu(buffer[numOfSampleCopied]);

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			data = rawVal;
		else
		{
			data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * rawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
		}

		dataBuf[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;

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

}

void DaqIUsbBase::processScanData16_uint64(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	unsigned int rawVal;
	unsigned long long* dataBuf = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui16_to_cpu(buffer[numOfSampleCopied]);

		dataBuf[mScanInfo.currentDataBufferIdx] = rawVal;

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
}

void DaqIUsbBase::processScanData32_dbl(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	double data;
	unsigned int rawVal;
	double* dataBuf = (double*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui32_to_cpu(buffer[numOfSampleCopied]);

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			data = rawVal;
		else
		{
			data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * rawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
		}

		dataBuf[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;

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
}

void DaqIUsbBase::processScanData32_uint64(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	unsigned int rawVal;
	unsigned long long* dataBuf = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui32_to_cpu(buffer[numOfSampleCopied]);

		dataBuf[mScanInfo.currentDataBufferIdx] = rawVal;

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
}

void DaqIUsbBase::processScanData64_uint64(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned long long* buffer = (unsigned long long*)transfer->buffer;

	unsigned long long rawVal;
	unsigned long long* dataBuf = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui64_to_cpu(buffer[numOfSampleCopied]);

		dataBuf[mScanInfo.currentDataBufferIdx] = rawVal;

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
}

} /* namespace ul */
