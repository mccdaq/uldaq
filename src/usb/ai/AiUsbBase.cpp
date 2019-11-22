/*
 * AiUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsbBase.h"

namespace ul
{

AiUsbBase::AiUsbBase(const UsbDaqDevice& daqDevice) : AiDevice(daqDevice), mUsbDevice(daqDevice)
{
	mScanEndpointAddr = 0;
	mScanStopCmd = 0;

	mTransferMode = SO_BLOCKIO;
}

AiUsbBase::~AiUsbBase()
{

}

UlError AiUsbBase::getStatus(ScanStatus* status, TransferStatus* xferStatus)
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

void AiUsbBase::sendStopCmd()
{
	unsigned char cmd = getScanStopCmd();

	daqDev().sendCmd(cmd);
}

UlError AiUsbBase::terminateScan()
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

	return err;
}

void AiUsbBase::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserIn()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError AiUsbBase::checkScanState(bool* scanDone) const
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

void AiUsbBase::loadAdcCoefficients()
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

		int calCoefCount = mAiInfo.getCalCoefCount();
		int calBlockSize = calCoefCount * sizeof(coef);
		int address = mAiInfo.getCalCoefsStartAddr();

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

int AiUsbBase::calcStageSize(int epAddr, double rate, int chanCount, int sampleCount) const
{
	int stageSize = 0;
	int minStageSize = mUsbDevice.getBulkEndpointMaxPacketSize(epAddr);

	if (getTransferMode() == SO_SINGLEIO)
	{
		stageSize = minStageSize;//chanCount * mAiInfo.getSampleSize();
	}
	else
	{
		double aggRate =  chanCount * rate * mAiInfo.getSampleSize(); // bytes per second
		long bufferBytesCount = sampleCount * mAiInfo.getSampleSize();
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

void AiUsbBase::setTransferMode(ScanOption scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if(!(scanOptions & SO_BURSTIO))
	{
		if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 1000.0))
			mTransferMode = SO_SINGLEIO;
	}
}

int AiUsbBase::getTransferMode() const
{
	return mTransferMode;
}

void AiUsbBase::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int AiUsbBase::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

void AiUsbBase::processScanData(void* transfer)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		processScanData16(usbTransfer);
		break;
	case 4:  // 4 bytes
		processScanData32(usbTransfer);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}
}

void AiUsbBase::processScanData16(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	double data;
	long rawVal;

	double* dataBuffer = (double*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui16_to_cpu(buffer[numOfSampleCopied]);

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			data = rawVal;
		else
		{
			data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * rawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;

			/* commented out. According to CH calibrated count should not be clipped if it is out of range
			if(mScanInfo.flags & NOSCALEDATA)
			{
				//data += 0.5;

				if(data > mScanInfo.fullScale)
					data = mScanInfo.fullScale;
				else if(data < 0)
					data = 0;
			}*/
		}

		dataBuffer[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;

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

void AiUsbBase::processScanData32(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	double data;
	unsigned int rawVal;

	double* dataBuffer = (double*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui32_to_cpu(buffer[numOfSampleCopied]);

		if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			data = rawVal;
		else
		{
			data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * rawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;

			/* commented out. According to CH calibrated count should not be clipped if it is out of range
			if(mScanInfo.flags & NOSCALEDATA)
			{
				if(data > mScanInfo.fullScale)
					data = mScanInfo.fullScale;
				else if(data < 0)
					data = 0;
			}*/
		}

		dataBuffer[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;

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

void AiUsbBase::readCalDate()
{
	unsigned char calDateBuf[6];
	int calDateAddr = mAiInfo.getCalDateAddr();

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
