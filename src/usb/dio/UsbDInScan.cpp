/*
 * UsbDInScan.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbDInScan.h"
#include "../UsbScanTransferIn.h"
#include "../../utility/Endian.h"

namespace ul
{

UsbDInScan::UsbDInScan(const UsbDaqDevice& daqDevice): DioUsbBase(daqDevice)
{
	mScanEndpointAddr = DIN_SCAN_EP;
	mScanStopCmd = CMD_DIN_SCAN_STOP;

	mTransferMode = SO_BLOCKIO;

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

UsbDInScan::~UsbDInScan()
{

}

double UsbDInScan::dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[])
{
	UlLock trigCmdLock(daqDev().getTriggerCmdMutex());

	int epAddr = getScanEndpointAddr();

	const DioInfo& dioInfo = (DioInfo&) (daqDev().dioDevice())->getDioInfo();

	int lowPortNum = dioInfo.getPortNum(lowPort);
	int highPortNum = dioInfo.getPortNum(highPort);
	int portCount = highPortNum - lowPortNum + 1;
	int sampleSize = 2;
	int resolution = 16;

	setTransferMode(options, rate);

	int stageSize = calcStageSize(epAddr, rate, portCount, samplesPerPort, sampleSize);

	std::vector<CalCoef> calCoefs;
	std::vector<CustomScale> customScales;

	daqDev().setupTrigger(FT_DI, options);

	daqDev().clearHalt(epAddr);

	daqDev().sendCmd(CMD_DIN_SCAN_CLEARFIFO);

	setScanInfo(FT_DI, portCount, samplesPerPort, sampleSize, resolution, options, flags, calCoefs, customScales, data);

	setScanConfig(lowPortNum, highPortNum, samplesPerPort, rate, options);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_DIN_SCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

	return actualScanRate();
}

void UsbDInScan::setScanConfig(int lowPortNum, int highPortNum, unsigned int scanCount, double rate, ScanOption options)
{
	int portCount = highPortNum - lowPortNum + 1;

	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.chan_map = (lowPortNum + 1) | (highPortNum + 1);
	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	int epAddr = getScanEndpointAddr();
	mScanConfig.packet_size = (getTransferMode() == SO_BLOCKIO) ? daqDev().getBulkEndpointMaxPacketSize(epAddr)/2 - 1 : portCount - 1;

	TriggerConfig trigCfg = daqDev().dioDevice()->getTrigConfig(SD_INPUT);

	if(options & SO_RETRIGGER)
	{
		if(trigCfg.retrigCount == 0)
			mScanConfig.retrig_count = scanCount;
		else
		{
			if(!(options & SO_CONTINUOUS))
				mScanConfig.retrig_count = trigCfg.retrigCount > scanCount ? scanCount : trigCfg.retrigCount;
			else
			{
				// different behavior from the UL for windows in continuous mode
				mScanConfig.retrig_count = trigCfg.retrigCount;
			}
		}
	}
}

unsigned char UsbDInScan::getOptionsCode(ScanOption options) const
{
#pragma pack(1)
	union TOptionsCode
	{
		struct
		{
			unsigned char ext_trigger		 : 1;
			unsigned char pattern_trigger    : 1;
			unsigned char retrigger			 : 1;
			unsigned char reserved0			 : 1;
			unsigned char reserved1			 : 1;
			unsigned char reserved2			 : 1;
			unsigned char reserved3			 : 1;
			unsigned char reserved4			 : 1;
		};
		unsigned char code;
	} option;
#pragma pack()

	TriggerConfig trigCfg = daqDev().getTriggerConfig(FT_DI);

	option.code = 0;

	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		option.ext_trigger = 1;

		if (options & SO_RETRIGGER)
			option.retrigger = 1;

		if(trigCfg.type & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW))
		{
			option.ext_trigger = 0;
			option.pattern_trigger = 1;
		}
	}

	return option.code;
}


void UsbDInScan::setScanState(ScanStatus state)
{
	IoDevice::setScanState(state);

	daqDev().dioDevice()->setScanState(SD_INPUT, state);
}

UlError UsbDInScan::getInputStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	UlError err = ERR_NO_ERROR;

	if(status && xferStatus)
	{
		ScanStatus scanStatus = IoDevice::getScanState();

		getXferStatus(xferStatus);

		if(scanStatus != SS_RUNNING)
			err = daqDev().scanTranserIn()->getXferError();

		*status = scanStatus;
	}
	else
		err = ERR_BAD_ARG;

	return err;
}

void UsbDInScan::sendStopCmd()
{
	unsigned char cmd = getScanStopCmd();

	daqDev().sendCmd(cmd);
}

UlError UsbDInScan::terminateScan()
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

void UsbDInScan::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserIn()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError UsbDInScan::checkScanState(bool* scanDone) const
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

int UsbDInScan::calcStageSize(int epAddr, double rate, int chanCount, int sampleCount, int sampleSize) const
{
	int stageSize = 0;
	int minStageSize = daqDev().getBulkEndpointMaxPacketSize(epAddr);


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

void UsbDInScan::setTransferMode(ScanOption scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if(!(scanOptions & SO_BURSTIO))
	{
		if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 1000.0))
			mTransferMode = SO_SINGLEIO;
	}
}

int UsbDInScan::getTransferMode() const
{
	return mTransferMode;
}

void UsbDInScan::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int UsbDInScan::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

void UsbDInScan::processScanData(void* transfer)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		processScanData16(usbTransfer);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}
}

void UsbDInScan::processScanData16(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	unsigned short data;
	unsigned long long* dataBuffer = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		data =  Endian::le_ui16_to_cpu(buffer[numOfSampleCopied]);

		dataBuffer[mScanInfo.currentDataBufferIdx] = data;

		mScanInfo.currentDataBufferIdx++;
		//mScanInfo.currentCalCoefIdx++;
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
	}
}
/*
void UsbDInScan::processScanData32(libusb_transfer* transfer)
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

			if(mScanInfo.flags & NOSCALEDATA)
			{
				//data += 0.5;

				if(data > mScanInfo.fullScale)
					data = mScanInfo.fullScale;
				else if(data < 0)
					data = 0;
			}
		}

		dataBuffer[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;
		//mScanInfo.dataBuffer[mScanInfo.currentDataBufferIdx] = data;

		//std::cout << "data = " << mScanInfo.dataBuffer[mCurrentDataBufferIdx] << std::endl;

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
}*/

} /* namespace ul */
