/*
 * AoUsb24xx.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AoUsb24xx.h"

#include <unistd.h>

namespace ul
{
AoUsb24xx::AoUsb24xx(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;

	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutScanFlags(AOUTSCAN_FF_NOSCALEDATA | AOUTSCAN_FF_NOCALIBRATEDATA);

	mAoInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_BLOCKIO);

	mAoInfo.hasPacer(true);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(16);
	mAoInfo.setMinScanRate(minRate);
	mAoInfo.setMaxScanRate(1000);
	mAoInfo.setMaxThroughput(1000 * numChans);
	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefsStartAddr(0x0180);
	mAoInfo.setCalCoefCount(numChans);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(BIP10VOLTS);

	setScanEndpointAddr(0x01);

	setScanStopCmd(CMD_AOUTSTOP);

	memset(&mScanConfig_2408, 0, sizeof(mScanConfig_2408));
	memset(&mScanConfig_2416, 0, sizeof(mScanConfig_2416));
}

AoUsb24xx::~AoUsb24xx()
{
}

void AoUsb24xx::initialize()
{
	try
	{
		//sendStopCmd(); // do not send stop before mem command.

		loadDacCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AoUsb24xx::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	UlLock lock(mIoDeviceMutex);

	check_AOut_Args(channel, range, flags, dataValue);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		writeData_2416(channel, UPDATE_CH, flags, dataValue);
	}
	else
	{
		writeData_2408(channel, UPDATE_CH, flags, dataValue);
	}
}

void AoUsb24xx::aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutArray_Args(lowChan, highChan, range, flags, data);

	int idx = 0;
	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		for(int channel = lowChan; channel < highChan; channel++)
		{
			writeData_2416(channel, NO_UPDATE, (AOutFlag)flags, data[idx]);
			idx++;
		}

		writeData_2416(highChan, UPDATE_ALL, (AOutFlag)flags, data[idx]);
	}
	else
	{
		for(int channel = lowChan; channel < highChan; channel++)
		{
			writeData_2408(channel, NO_UPDATE, (AOutFlag)flags, data[idx]);
			idx++;
		}

		writeData_2408(highChan, UPDATE_ALL, (AOutFlag)flags, data[idx]);
	}
}

double AoUsb24xx::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutScan_Args(lowChan, highChan, range, samplesPerChan, rate, options, flags, data);

	int epAddr = getScanEndpointAddr();
	setTransferMode(SO_BLOCKIO, rate); // always run in block i/o. Do not enable singe io mode

	// send stop command to clear the FIFO
	sendStopCmd();

	int chanCount = highChan - lowChan + 1;
	int stageSize = daqDev().getBulkEndpointMaxPacketSize(epAddr); // always 64 bytes //calcStageSize(epAddr, rate, chanCount,  samplesPerChan);

	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, range, flags);

	setScanInfo(FT_AO, chanCount, samplesPerChan, mAoInfo.getSampleSize(), mAoInfo.getResolution(), options, flags, calCoefs, data);

	unsigned char cfg[7] = {0};
	unsigned short cfgSize = setScanConfig(lowChan, highChan, samplesPerChan, rate, options, cfg);

	daqDev().scanTranserOut()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		// coverity[sleep]
		usleep(1000);
		daqDev().sendCmd(CMD_AOUTSCAN_START, 0, 0, (unsigned char*) &cfg, cfgSize, 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

unsigned short AoUsb24xx::setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options, unsigned char* cfg)
{
	unsigned short cfgSize = 0;

	double clockFreq = mDaqDevice.getClockFreq();
	double periodDbl = (clockFreq / rate);

	double actualrate = 0;

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		memset(&mScanConfig_2416, 0, sizeof(mScanConfig_2416));

		if(periodDbl > USHRT_MAX)
			periodDbl = USHRT_MAX;

		unsigned short period = periodDbl;

		actualrate = clockFreq / period;

		mScanConfig_2416.pacer_period = Endian::cpu_to_le_ui16(period);
		mScanConfig_2416.scan_count = 0; // since the maximum finite sample count is 65535. We always run the scan in continuous mode
										 // but make sure to send number of requested samples in finite mode and monitor the status and
										 // make sure to ignore the overrun error if all the finite samples already transfered

		for(int chan = lowChan; chan <= highChan; chan++)
			mScanConfig_2416.options |= 1 << chan;

		cfgSize = sizeof(mScanConfig_2416);
		memcpy(cfg, &mScanConfig_2416, cfgSize);
	}
	else
	{
		memset(&mScanConfig_2408, 0, sizeof(mScanConfig_2408));

		if(periodDbl > UINT_MAX)
			periodDbl = UINT_MAX;

		unsigned int period = periodDbl;

		actualrate = clockFreq / period;

		mScanConfig_2408.pacer_period = Endian::cpu_to_le_ui32(period);
		mScanConfig_2408.scan_count = 0; // since the maximum finite sample count is 65535. We always run the scan in continuous mode
		 	 	 	 	 	 	 	 	 // but make sure to send number of requested samples in finite mode and monitor the status and
		 	 	 	 	 	 	 	 	 // make sure to ignore the overrun error if all the finite samples already transfered

		for(int chan = lowChan; chan <= highChan; chan++)
			mScanConfig_2408.options |= 1 << chan;

		cfgSize = sizeof(mScanConfig_2408);
		memcpy(cfg, &mScanConfig_2408, cfgSize);
	}

	setActualScanRate(actualrate);

	return cfgSize;
}

void AoUsb24xx::writeData_2408(int channel, int mode, AOutFlag flags, double dataValue)
{
#pragma pack(1)
	struct
	{
		unsigned short value;
		unsigned char cmd;
	} cfg = {0};
#pragma pack()

	cfg.value = calibrateData(channel, BIP10VOLTS, flags, dataValue);
	cfg.cmd = channel << 2;

	if(mode == UPDATE_CH)
	{
		cfg.cmd |= 0x10 << channel;
	}
	else if(mode == UPDATE_ALL)
	{
		cfg.cmd |= 0x30;
	}

	daqDev().sendCmd(CMD_AOUT, 0, 0, (unsigned char*) &cfg, sizeof(cfg));
}

void AoUsb24xx::writeData_2416(int channel, int mode, AOutFlag flags, double dataValue)
{
#pragma pack(1)
	struct
	{
		short value;
		unsigned char cmd;
	} cfg = {0};
#pragma pack()

	unsigned int count = dataValue;

	// convert to count
	if (!(flags & NOSCALEDATA))
	{
		double scale = 20;

		unsigned long long fullScaleCount = 0x10000;
		double lsb = scale / fullScaleCount;

		count = (dataValue / lsb) + (fullScaleCount / 2);
	}

	if(count > 0xffff)
		count = 0xffff;

	unsigned short u16RawVal = (unsigned short) count;
	short i16RawVal = convertU16ToI16(u16RawVal);

	if(!(flags & NOCALIBRATEDATA))
	{
		int calCoefIdx = getCalCoefIndex(channel, BIP10VOLTS);

		double val = i16RawVal;
		int i32CalVal = i16RawVal;

		val *= mCalCoefs[calCoefIdx].slope;
		val += mCalCoefs[calCoefIdx].offset;
		if (val > 32767.0)
			i32CalVal = 32767.0;
		else if (val < -32768.0)
			i32CalVal = -32768.0;
		else
			i32CalVal = (int) val;

		cfg.value = (short) i32CalVal;
	}
	else
		cfg.value = i16RawVal;

	cfg.cmd = channel << 1;

	if(mode == UPDATE_CH)
	{
		cfg.cmd |= 0x10;
	}
	else if(mode == UPDATE_ALL)
	{
		cfg.cmd |= 0x20;
	}

	daqDev().sendCmd(CMD_AOUT, 0, 0, (unsigned char*) &cfg, sizeof(cfg));
}

short AoUsb24xx::convertU16ToI16(unsigned short val)
{
   int sdata = val;
   sdata -= 0x08000;

   return sdata;
}

UlError AoUsb24xx::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned char cmd = CMD_AOUTSCAN_STATUS;

#pragma pack(1)
	struct
	{
		unsigned short depth;
		unsigned char status;
	}scan;
#pragma pack()

	try
	{
		daqDev().queryCmd(cmd, 0, 0, (unsigned char*)&scan, sizeof(scan));

		if(!(scan.status & 0x01))
			*scanDone = true;

		if(scan.status & 0x02)
		{
			// scans always run in continuous mode. the allSamplesTransferred variable is only set to true in finite mode
			// if allSamplesTransferred is true and overrun bit is set then we ignore the overrun bit.
			if(!(mScanInfo.allSamplesTransferred))
				err = ERR_UNDERRUN;

			// 2416 does not reset bit0 when underrun occurs
			*scanDone = true;
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

std::vector<CalCoef> AoUsb24xx::getScanCalCoefs(int lowChan, int highChan, Range range, long long flags) const
{
	std::vector<CalCoef> calCoefs;

	int chan;
	CalCoef calCoef;

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
		flags |= NOSCALEDATA;

	for (chan = lowChan; chan <= highChan; chan++)
	{
		calCoef = getCalCoef(chan, range, flags);
		calCoefs.push_back(calCoef);
	}

	return calCoefs;
}

void AoUsb24xx::loadDacCoefficients()
{
#pragma pack(1)
	typedef struct
	{
		unsigned char slope[8];
		unsigned char offset[8];
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

		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, address, (unsigned char*)buffer, calBlockSize);

		if(bytesReceived == calBlockSize)
		{
			for(int i = 0; i < calCoefCount; i++)
			{
				calCoef.slope = mEndian.le_ptr_to_cpu_f64(buffer[i].slope);
				calCoef.offset = mEndian.le_ptr_to_cpu_f64(buffer[i].offset);

				mCalCoefs.push_back(calCoef);
			}
		}

		delete [] buffer;

		//readCalDate();
	}
}

unsigned int AoUsb24xx::processScanData(void* transfer, unsigned int stageSize)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;
	unsigned int actualStageSize = 0;

	if(daqDev().getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		actualStageSize = processScanData16_2416(usbTransfer, stageSize);
	}
	else
	{
		actualStageSize = AoUsbBase::processScanData(transfer, stageSize);
	}

	return actualStageSize;
}


unsigned int AoUsb24xx::processScanData16_2416(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	short* buffer = (short*)transfer->buffer;

	double data;
	long long rawVal;
	unsigned short count;
	short countI16;
	unsigned int countU32;
	double* dataBuffer = (double*) mScanInfo.dataBuffer;
	long long fullScale = mScanInfo.fullScale;

	double scale = 20.0;
	unsigned long long fullScaleCount = 0x10000;
	unsigned long long halfFullScaleCount = 0x10000 / 2;
	double lsb = scale / fullScaleCount;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		// convert to count
		if (!(mScanInfo.flags & NOSCALEDATA))
			countU32 = (data / lsb) + halfFullScaleCount;
		else
			countU32 = data;

		if(countU32 > fullScale)
			countU32 = fullScale;

		if(!(mScanInfo.flags & NOCALIBRATEDATA))
		{
			rawVal = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * countU32) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset + 0.5;

			if(rawVal > fullScale)
				count = fullScale;
			else if(rawVal < 0)
				count = 0;
			else
				count = rawVal;
		}
		else
		{
			count = countU32;
		}

		countI16 = convertU16ToI16(count);

		buffer[numOfSampleCopied] = Endian::cpu_to_le_i16(countI16);

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
