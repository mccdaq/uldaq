/*
 * AoUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsbBase.h"

namespace ul
{

AoUsbBase::AoUsbBase(const UsbDaqDevice& daqDevice): AoDevice(daqDevice), mUsbDevice(daqDevice)
{
	mScanEndpointAddr = 0;
	mScanStopCmd = 0;

	mTransferMode = SO_BLOCKIO;
}

AoUsbBase::~AoUsbBase()
{

}

void AoUsbBase::setTransferMode(long long scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 1000.0))
		mTransferMode = SO_SINGLEIO;
}

int AoUsbBase::getTransferMode() const
{
	return mTransferMode;
}

void AoUsbBase::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int AoUsbBase::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

UlError AoUsbBase::getStatus(ScanStatus* status, TransferStatus* xferStatus)
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

void AoUsbBase::sendStopCmd()
{
	unsigned char cmd = getScanStopCmd();

	daqDev().sendCmd(cmd);
}

UlError AoUsbBase::terminateScan()
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

	return err;
}

void AoUsbBase::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserOut()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError AoUsbBase::checkScanState(bool* scanDone) const
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

void AoUsbBase::loadDacCoefficients()
{
#pragma pack(1)
	typedef struct
	{
		unsigned char slope[4];
		unsigned char offset[4];
	}  coef;
#pragma pack()

	CalCoef calCoef;

	if(getScanState() == SS_IDLE)
	{
		mCalCoefs.clear();

		int calCoefCount = mAoInfo.getCalCoefCount();
		int calBlockSize = calCoefCount * sizeof(coef);
		int address = mAoInfo.getCalCoefsStartAddr();

		coef* buffer = new coef[calCoefCount];

		int bytesReceived = mUsbDevice.memRead(MT_EEPROM, MR_CAL, address, (unsigned char*)buffer, calBlockSize);

		if(bytesReceived == calBlockSize)
		{
			for(int i = 0; i < calCoefCount; i++)
			{
				calCoef.slope = mEndian.le_ptr_to_cpu_f32(buffer[i].slope);
				calCoef.offset = mEndian.le_ptr_to_cpu_f32(buffer[i].offset);

				mCalCoefs.push_back(calCoef);
			}
		}

		delete [] buffer;

		readCalDate();
	}
}

int AoUsbBase::calcStageSize(int epAddr, double rate, int chanCount, int sampleCount) const
{
	int stageSize = 0;
	int minStageSize = mUsbDevice.getBulkEndpointMaxPacketSize(epAddr);

	if (getTransferMode() == SO_SINGLEIO)
	{
		stageSize = chanCount * mAoInfo.getSampleSize();
	}
	else
	{
		double aggRate =  chanCount * rate * mAoInfo.getSampleSize(); // bytes per second
		long long bufferBytesCount = (long long) sampleCount * mAoInfo.getSampleSize();
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

unsigned int AoUsbBase::processScanData(void* transfer, unsigned int stageSize)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;
	unsigned int actualStageSize = 0;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		actualStageSize = processScanData16(usbTransfer, stageSize);
		break;
	case 4:  // 4 bytes
		actualStageSize = processScanData32(usbTransfer, stageSize);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}

	return actualStageSize;
}

unsigned int AoUsbBase::processScanData16(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	double data;
	long long rawVal;
	unsigned short count;
	double* dataBuffer = (double*) mScanInfo.dataBuffer;
	long long fullScale = mScanInfo.fullScale;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			count = data;
		else
		{
			rawVal = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset + 0.5;

			if(rawVal > fullScale)
				count = fullScale;
			else if(rawVal < 0)
				count = 0;
			else
				count = rawVal;
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

unsigned int AoUsbBase::processScanData32(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	double data;
	long long rawVal;
	unsigned int count;
	double* dataBuffer = (double*) mScanInfo.dataBuffer;
	long long fullScale = mScanInfo.fullScale;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			count = data;
		else
		{
			rawVal = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset + 0.5;

			if(rawVal > fullScale)
				count = fullScale;
			else if(rawVal < 0)
				count = 0;
			else
				count = rawVal;
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

void AoUsbBase::readCalDate()
{
	unsigned char calDateBuf[6];
	int calDateAddr = mAoInfo.getCalDateAddr();

	if(calDateAddr != -1 && getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

		if(bytesReceived == sizeof(calDateBuf))
		{
			tm time;
			memset(&time, 0, sizeof(time));

			time.tm_year = calDateBuf[0] + 100;
			time.tm_mon = calDateBuf[1] - 1;
			time.tm_mday = calDateBuf[2];
			time.tm_hour = calDateBuf[3];
			time.tm_min = calDateBuf[4];
			time.tm_sec = calDateBuf[5];
			time.tm_isdst = -1;

			// make sure the date is valid, mktime does not validate the range
			if(time.tm_mon <= 11 && time.tm_mday <= 31 && time.tm_hour <= 23 && time.tm_min <= 59 && time.tm_sec <= 60)
			{
				time_t cal_date_sec = mktime(&time); // seconds since unix epoch

				if(cal_date_sec > 0) // mktime returns  -1 if cal date is invalid
					mCalDate = cal_date_sec;

				// convert seconds to string

				/*struct tm *timeinfo;
				timeinfo = localtime(&cal_date_sec);
				char b[100];
				strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
				std::cout << b << std::endl;*/
			}
		}
	}
}

} /* namespace ul */
