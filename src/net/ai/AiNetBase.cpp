/*
 * AiNetBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiNetBase.h"

namespace ul
{
AiNetBase::AiNetBase(const NetDaqDevice& daqDevice) : AiDevice(daqDevice), mNetDevice(daqDevice)
{

}

AiNetBase::~AiNetBase()
{

}

UlError AiNetBase::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned short status =0;

	try
	{
		status = daqDev().readStatus();

		if(status & STATUS_DATA_OVERRUN)
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

void AiNetBase::loadAdcCoefficients()
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

		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, address, (unsigned char*)buffer, calBlockSize);

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
unsigned int AiNetBase::processScanData(void* transfer, unsigned int stageSize)
{
	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		processScanData16((unsigned char*) transfer, stageSize);
		break;
	case 4:  // 4 bytes
		processScanData32((unsigned char*) transfer, stageSize);
		break;
	case 8:  // 8 bytes
		processScanData64((unsigned char*) transfer, stageSize);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}

	return 0;
}

void AiNetBase::processScanData16(unsigned char* xferBuf, unsigned int xferLength)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = xferLength / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned short* buffer = (unsigned short*)xferBuf;

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

void AiNetBase::readCalDate()
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

				if(cal_date_sec != -1) // mktime returns  -1 if cal date is invalid
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
