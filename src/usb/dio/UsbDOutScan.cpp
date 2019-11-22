/*
 * UsbDOutScan.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbDOutScan.h"
#include "../UsbScanTransferOut.h"

namespace ul
{

UsbDOutScan::UsbDOutScan(const UsbDaqDevice& daqDevice): DioUsbBase(daqDevice)
{
	mScanEndpointAddr = DOUT_SCAN_EP;
	mScanStopCmd = CMD_DOUT_SCAN_STOP;

	mTransferMode = SO_BLOCKIO;

	memset(&mScanConfig, 0, sizeof(mScanConfig));
}

UsbDOutScan::~UsbDOutScan()
{

}

double UsbDOutScan::dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[])
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

	int stageSize = calcStageSize(epAddr, rate, portCount,  samplesPerPort, sampleSize);

	std::vector<CalCoef> calCoefs;// = getScanCalCoefs(lowChan, highChan, range, flags);

	//daqDev().clearHalt(epAddr);

	setScanInfo(FT_DO, portCount, samplesPerPort, sampleSize, resolution, options, flags, calCoefs, data);

	daqDev().setupTrigger(FT_DO, options);

	setScanConfig(lowPortNum, highPortNum, samplesPerPort, rate, options);

	daqDev().sendCmd(CMD_DOUT_SCAN_CLEARFIFO);

	daqDev().scanTranserOut()->initilizeTransfers(this, epAddr, stageSize);

	try
	{
		daqDev().sendCmd(CMD_DOUT_SCAN_START, 0, 0, (unsigned char*) &mScanConfig, sizeof(mScanConfig), 1000);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

		return actualScanRate();
}

void UsbDOutScan::setScanConfig(int lowPortNum, int highPortNum, unsigned int scanCount, double rate, ScanOption options)
{
	memset(&mScanConfig, 0, sizeof(mScanConfig));

	mScanConfig.chan_map = (lowPortNum + 1) | (highPortNum + 1);
	mScanConfig.pacer_period = Endian::cpu_to_le_ui32(calcPacerPeriod(rate, options));
	mScanConfig.options = getOptionsCode(options);
	mScanConfig.scan_count = Endian::cpu_to_le_ui32(scanCount);

	if(options & SO_CONTINUOUS)
		mScanConfig.scan_count = 0;

	TriggerConfig trigCfg = daqDev().dioDevice()->getTrigConfig(SD_OUTPUT);

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

unsigned char UsbDOutScan::getOptionsCode(ScanOption options) const
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

	TriggerConfig trigCfg = daqDev().getTriggerConfig(FT_DO);

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

void UsbDOutScan::setScanState(ScanStatus state)
{
	IoDevice::setScanState(state);

	daqDev().dioDevice()->setScanState(SD_OUTPUT, state);
}

void UsbDOutScan::setTransferMode(long long scanOptions, double rate)
{
	mTransferMode = SO_BLOCKIO;

	if((scanOptions & SO_SINGLEIO) || (!(scanOptions & SO_BLOCKIO) && rate <= 1000.0))
		mTransferMode = SO_SINGLEIO;
}

int UsbDOutScan::getTransferMode() const
{
	return mTransferMode;
}

void UsbDOutScan::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int UsbDOutScan::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

UlError UsbDOutScan::getOutputStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	UlError err = ERR_NO_ERROR;

	if(status && xferStatus)
	{
		ScanStatus scanStatus = IoDevice::getScanState();

		getXferStatus(xferStatus);

		if(scanStatus != SS_RUNNING)
			err = daqDev().scanTranserOut()->getXferError();

		*status = scanStatus;
	}
	else
		err = ERR_BAD_ARG;

	return err;
}

void UsbDOutScan::sendStopCmd()
{
	unsigned char cmd = getScanStopCmd();

	daqDev().sendCmd(cmd);
}

UlError UsbDOutScan::terminateScan()
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

void UsbDOutScan::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserOut()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError UsbDOutScan::checkScanState(bool* scanDone) const
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

int UsbDOutScan::calcStageSize(int epAddr, double rate, int chanCount, int sampleCount, int sampleSize) const
{
	int stageSize = 0;
	int minStageSize = daqDev().getBulkEndpointMaxPacketSize(epAddr);

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

unsigned int UsbDOutScan::processScanData(void* transfer, unsigned int stageSize)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;
	unsigned int actualStageSize = 0;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
		actualStageSize = processScanData16(usbTransfer, stageSize);
		break;
	/*case 4:  // 4 bytes
		processScanData32(usbTransfer);
		break;*/
	default:
		std::cout << "##### undefined sample size";
		break;
	}

	return actualStageSize;
}

unsigned int UsbDOutScan::processScanData16(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	unsigned short data;
	unsigned long long* dataBuffer = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		data = dataBuffer[mScanInfo.currentDataBufferIdx];

		buffer[numOfSampleCopied] = Endian::cpu_to_le_ui16(data);

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

		//if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
		//	mScanInfo.currentCalCoefIdx = 0;
	}

	actualStageSize = numOfSampleCopied * mScanInfo.sampleSize;

	return actualStageSize;
}

} /* namespace ul */
