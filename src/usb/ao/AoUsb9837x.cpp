/*
 * AoUsb9837x.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AoUsb9837x.h"
#include "../ai/AiUsb9837x.h"

#include <unistd.h>
#include <math.h>

namespace ul
{

#define USB_BLOCK_WRITE_SIZE  0xE00

// each USB write must start with 512 bytes, the first four of which are the transfer size.
// The actual data starts at offset 512
#define OUTPUT_PACKET_INFO_BLOCK_SIZE 	512

AoUsb9837x::AoUsb9837x(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA );
	//mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutScanFlags(AOUTSCAN_FF_NOSCALEDATA);

	mAoInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS |SO_EXTTRIGGER /*|SO_EXTCLOCK*|SO_SINGLEIO*/|SO_BLOCKIO/*|SO_RETRIGGER*/);
	mAoInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_RISING);

	mAoInfo.hasPacer(true);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(24);
	mAoInfo.setMinScanRate(10000);

	if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_A)
	{
		mAoInfo.setMaxScanRate(52734);
		mAoInfo.setMaxThroughput(52734);

		mAoInfo.addRange(BIP10VOLTS);
	}
	else //UL_DT9837_C
	{
		mAoInfo.setMaxScanRate(96000);
		mAoInfo.setMaxThroughput(96000);

		mAoInfo.addRange(BIP3VOLTS);
	}

	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefCount(numChans);
	mAoInfo.setSampleSize(4);

	setScanEndpointAddr(0x06);

	mUnderrunOccurred = false;
	mPreviousClockFreq = 0;

}

AoUsb9837x::~AoUsb9837x()
{
}

void AoUsb9837x::initialize()
{
	try
	{
		//sendStopCmd();

		loadDacCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}

	mPreviousClockFreq = 0;
}


void AoUsb9837x::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	UlLock lock(mIoDeviceMutex);

	check_AOut_Args(channel, range, flags, dataValue);

	unsigned int calData = calibrateData(channel, range, flags, dataValue);

	CmdSetSingleValueDAC(calData, channel);
}


double AoUsb9837x::aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AOutScan_Args(lowChan, highChan, range, samplesPerChan, rate, options, flags, data);

	if(rate < mAoInfo.getMinScanRate())
		rate = mAoInfo.getMinScanRate();

	resetScanErrorFlag();
	mUnderrunOccurred = false;

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	int chanCount = highChan - lowChan + 1;
	int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan);


	if(stageSize > USB_BLOCK_WRITE_SIZE)
		stageSize = USB_BLOCK_WRITE_SIZE;

	stageSize += OUTPUT_PACKET_INFO_BLOCK_SIZE;


	std::vector<CalCoef> calCoefs = getScanCalCoefs(lowChan, highChan, range, flags);

	daqDev().clearHalt(epAddr);

	setScanInfo(FT_AO, chanCount, samplesPerChan, mAoInfo.getSampleSize(), mAoInfo.getResolution(), options, flags, calCoefs, data);

	setDAOutputSampleClock(rate);

	// SB:added the following line to prevent bad output when scan is ran after continuous scan stopped.
	sendStopCmd();

	configureScan(samplesPerChan, options);

	daqDev().scanTranserOut()->initilizeTransfers(this, epAddr, stageSize);

	// coverity[sleep]
	usleep(1000);

	try
	{
		//CmdEnableDAEvents();
		CmdSetArmDACtrls(options);

		setScanState(SS_RUNNING);
	}
	catch(UlException& e)
	{
		stopBackground();
		throw e;
	}

		return actualScanRate();
}

void AoUsb9837x::setDAOutputSampleClock(double rate) //LARGE_INTEGER liClockFrequency, LARGE_INTEGER& liActualFreq)
{
	// If A/D clock frequency hasn't changed then just exit
	/*if (rate == mPreviousClockFreq)
	{
		return;
	}*/

	mPreviousClockFreq = rate;

	Usb9837xDefs::CY22150REGISTERS SPIRegisters;

	double freqIn;
	double freqRef;
	double actualFreq;
	double fOut;
	//long long	DelayTimeInUSecs;

	freqRef = 48;
	unsigned char divider;
	unsigned short regValue = 0;

	freqIn = rate;

	Usb9837x::programClock(freqIn, freqRef , &actualFreq, &SPIRegisters,&fOut,&divider, true);

	// Write the data to the PLL
	unsigned char numWrites = 0;

	Usb9837xDefs::WRITE_BYTE_INFO	m_writeByteInfo[10];

	m_writeByteInfo[numWrites].Address = Usb9837xDefs::DIV1SRC_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.Div1Src;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA,Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::DIV2SRC_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = (unsigned char)(SPIRegisters.Div1Src <<1);
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CHARGE_PUMP_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.ChargePump;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::PB_COUNTER_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.PBCounter;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::PO_Q_COUNTER_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.POQCounter;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CROSSPOINT_SWITCH_MATRIX_CNTRL_0_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.CrossPointSw0;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CROSSPOINT_SWITCH_MATRIX_CNTRL_1_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.CrossPointSw1;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CROSSPOINT_SWITCH_MATRIX_CNTRL_2_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.CrossPointSw2;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

    numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CLKOE_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.ClkOe;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_DA, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


	// Bits [9:8] control the divider selection
	regValue =0;
	switch(divider)
	{
		case 2:
			regValue = 0x0 <<10;		// 00 - Divide by 2
			break;
		case 4:
			regValue = 0x1 <<10;		// 01 - Divide by 4
			break;
		case 8:
			regValue = 0x2 <<10;		// 10 - Divide by 8
			break;
		case 16:
			regValue = 0x3 <<10;		// 11 - Divide by 16
			break;
	}

	// Write the divider to Control Register0
	dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG0, (0x3<<10), regValue);

	setActualScanRate(actualFreq);
}

void AoUsb9837x::configureScan(int samplesPerChan, ScanOption options)
{
	unsigned short wAndVal, wOrVal;

	unsigned int WorkingOutputFifoSize = mAoInfo.getFifoSize();//0x8000; //FIFO_SIZE;//DAC_HDW_FIFO_SIZE_IN_BYTES;

	if(!(options & SO_CONTINUOUS))
	{
		if((samplesPerChan * mAoInfo.getSampleSize()) < mAoInfo.getFifoSize())
		{
			WorkingOutputFifoSize = samplesPerChan * mAoInfo.getSampleSize();
		}
	}

	//WorkingOutputFifoSize = min( CurrentIrpBuffSize, DAC_HDW_FIFO_SIZE_IN_BYTES);

   // clear out DAC trigger bits
	wAndVal = ( 1<<6) + (1<<5);
	wOrVal = 0;
	dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) Usb9837xDefs::GENERAL_CNTRL_REG0, wAndVal, wOrVal);


	// Set the DA stop bit
	wAndVal = 0;
	wOrVal = 1<<2;
	dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) Usb9837xDefs::GENERAL_CNTRL_REG3, wAndVal, wOrVal);


	// update: Keep the D/A output enabled
	// clear the DA Output Enable bit, isolator
	//wAndVal = 1<<3;
	//wOrVal = 0;
	// m_rDt9837Device.Cmd_RMWSingleWordToLocalBus(GENERAL_CNTRL_REG3, wAndVal, wOrVal);

   // Clear DA stop bit
	wAndVal = 1<<2;
	wOrVal = 0;
	dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) Usb9837xDefs::GENERAL_CNTRL_REG3, wAndVal, wOrVal);


	// Put FIFO in reset
	wAndVal = 1<<8;
	wOrVal = wAndVal;
	dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) Usb9837xDefs::GENERAL_CNTRL_REG3, wAndVal, wOrVal);

	// Take FIFO out of reset
	wAndVal = 1<<8;
	wOrVal = 0;
	dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) Usb9837xDefs::GENERAL_CNTRL_REG3, wAndVal, wOrVal);

   // set the DA Output Enable bit, isolator
	wAndVal = 1<<3;
	wOrVal = wAndVal;
	dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) Usb9837xDefs::GENERAL_CNTRL_REG3, wAndVal, wOrVal);

	dtDev().Cmd_SetDaFifoSize( WorkingOutputFifoSize);
}

void AoUsb9837x::CmdSetArmDACtrls(ScanOption options)
{
	//start the subsystem
	Usb9837xDefs::SUBSYSTEM_INFO subsystemInfo;
	subsystemInfo.SubsystemType = Usb9837xDefs::SS_DA;
	subsystemInfo.ExtTrig = 0x00;

	if(options & SO_EXTTRIGGER)
	{
		subsystemInfo.ExtTrig = 0x01;

		if(mTrigCfg.type == TRIG_RISING)
		{
			subsystemInfo.ExtTrig = 0x02;


			if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_C)
			{
				// Since the threshold start trigger on the DT9837C is digital, we need to use the channel gain
				// to calculate the raw 16-bit trigger code.

				// important note: whatever range is used for trigger channel when AIn or AInScan or DaqInScan is invoked, will be used
				// for threshold. user can change the range by calling AIn before calling AOutScan

				const AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

				if(aiDev)
				{
					Range trigChanRange = aiDev->getCurrentChanRange(mTrigCfg.trigChan); // last range used when AIn or AInscan were called

					unsigned int thresholdValue = 0;

					double gain = (trigChanRange == BIP1VOLTS) ? 10 : 1;

					AiUsb9837x::VoltsToRawValue(mTrigCfg.level, gain, &thresholdValue, 16);

					// Write the triggerChannel to the hardware
					dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::THRSHOLD_CNTRL_REG , Usb9837xDefs::THRSHOLD_CNTRL_REG_MASK, mTrigCfg.trigChan);

					// Write the trigger threshold value to the hardware
					dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::THRSHOLD_TRIGLEVEL_REG, 0xFFFF, (unsigned short) thresholdValue);
				}

			}
			else
			{
				dtDev().CmdSetAnalogTriggerThreshold(mTrigCfg.level);
			}
		}
	}

	dtDev().Cmd_StartSubsystem (&subsystemInfo);
}

void AoUsb9837x::StopUsbOutputStreaming()
{
	Usb9837xDefs::SUBSYSTEM_INFO subsystemInfo;
	subsystemInfo.SubsystemType = Usb9837xDefs::SS_DA;
	subsystemInfo.ExtTrig = 0x00;
	dtDev().Cmd_StopSubsystem (&subsystemInfo);
}

void AoUsb9837x::CmdEnableDAEvents()
{
	unsigned short andMask = Usb9837xDefs::DAC_UNDERRUN_EN | Usb9837xDefs::DAC_UNDERRUN_ERR;
	unsigned short orValue = Usb9837xDefs::DAC_UNDERRUN_EN & ~(Usb9837xDefs::DAC_UNDERRUN_ERR);
	return dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG6, andMask, orValue);
}

void AoUsb9837x::CmdDisableDAEvents()
{
	unsigned short andMask = Usb9837xDefs::DAC_UNDERRUN_EN | Usb9837xDefs::DAC_UNDERRUN_ERR;
	unsigned short orValue = Usb9837xDefs::DAC_UNDERRUN_ERR;
	return dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG6, andMask, orValue);
}

void AoUsb9837x::CmdSetSingleValueDAC(unsigned int wAnalogOutValue, unsigned char Channel)
{
	Usb9837xDefs::WRITE_SINGLE_VALUE_INFO writeSingleValueInfo;

	writeSingleValueInfo.Channel		 = Channel;
	writeSingleValueInfo.DataValue 	 = wAnalogOutValue & 0xFFFFFF;
	//writeSingleValueInfo.DataValue = HIWORD(writeSingleValueInfo.DataValue) | LOWORD(writeSingleValueInfo.DataValue) << 16;

	writeSingleValueInfo.SubsystemType = Usb9837xDefs::SS_DA;

	dtDev().Cmd_WriteSingleValue(&writeSingleValueInfo);
}

void AoUsb9837x::sendStopCmd()
{
	StopUsbOutputStreaming();
	CmdDisableDAEvents();
}

UlError AoUsb9837x::terminateScan()
{
	UlError err = ERR_NO_ERROR;


	try
	{
		sendStopCmd();
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

UlError AoUsb9837x::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;

	if(allScanSamplesTransferred() && mUnderrunOccurred)
	{
		*scanDone = true;
	}
	else if(mUnderrunOccurred)
	{
		err = ERR_UNDERRUN;
	}

	return err;
}

void AoUsb9837x::check_AOutSetTrigger_Args(TriggerType trigType, int trigChan, double level, double variance, unsigned int retriggerCount) const
{
	AoDevice::check_AOutSetTrigger_Args(trigType, trigChan, level, variance, retriggerCount);

	if(trigType & TRIG_RISING)
	{
		if(trigChan != 0)
			throw UlException(ERR_BAD_TRIG_CHANNEL);

		double VoltageRangeHigh = 9.8;
		double VoltageRangeLow = 0.2;

		if ((level >= VoltageRangeHigh) || (level <= VoltageRangeLow))
			throw UlException(ERR_BAD_TRIG_LEVEL);
	}
}

unsigned int AoUsb9837x::processScanData32(libusb_transfer* transfer, unsigned int stageSize)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	// each USB write must start with 512 bytes, the first four of which are the transfer size.
	// The actual data starts at offset 512
	/************************/
	stageSize -= OUTPUT_PACKET_INFO_BLOCK_SIZE;
	unsigned int* buffer = (unsigned int*) (transfer->buffer + OUTPUT_PACKET_INFO_BLOCK_SIZE);

	/************************/


	int numOfSampleCopied = 0;
	unsigned int actualStageSize = 0;
	int requestSampleCount = stageSize / mScanInfo.sampleSize;


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

	/************************/
	unsigned int* packetSize =  (unsigned int*) transfer->buffer;
	*packetSize = actualStageSize;

	actualStageSize += OUTPUT_PACKET_INFO_BLOCK_SIZE;

	/************************/


	return actualStageSize;
}

void AoUsb9837x::loadDacCoefficients()
{
	CalCoef calCoef;

	if(getScanState() == SS_IDLE)
	{
		mCalCoefs.clear();

		int calCoefCount = mAoInfo.getCalCoefCount();

		for(int i = 0; i < calCoefCount; i++)
		{
			calCoef.slope = 1.0;
			calCoef.offset = 0.0;

			mCalCoefs.push_back(calCoef);
		}
	}
}

CalCoef AoUsb9837x::getInputChanCalCoef(int channel, long long flags) const
{
	std::vector<Range> ranges = mAoInfo.getRanges();
	Range range = ranges[0];

	double offset = 0;
	double scale = 0;
	mDaqDevice.getEuScaling(range, scale, offset);

	double lsb = scale / pow(2.0, mAoInfo.getResolution());

	CalCoef coef = { lsb, offset };

	return coef;
}


} /* namespace ul */
