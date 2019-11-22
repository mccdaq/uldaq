/*
 * DaqIUsb9837x.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "DaqIUsb9837x.h"
#include "../ai/AiUsb9837x.h"
#include "../ao/AoUsb9837x.h"

#include <unistd.h>

#define ADC_MODE_BIT				(1 << 0)
#define ADC_MODE_MASK				(ADC_MODE_BIT)

#define ADC_SYNC_BIT				(1 << 1)
#define ADC_SYNC_MASK				(ADC_SYNC_BIT)

namespace ul
{
DaqIUsb9837x::DaqIUsb9837x(const UsbDaqDevice& daqDevice) : DaqIUsbBase(daqDevice)
{
	double minRate = 195.313;

	mDaqIInfo.setDaqInScanFlags(DAQINSCAN_FF_NOSCALEDATA /*| DAQINSCAN_FF_NOCLEAR*/);
	mDaqIInfo.setScanOptions(SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER | SO_SINGLEIO | SO_BLOCKIO | SO_EXTTIMEBASE |SO_TIMEBASEOUT);

	mDaqIInfo.setMaxBurstRate(0);
	mDaqIInfo.setMaxBurstThroughput(0);
	mDaqIInfo.setFifoSize(FIFO_SIZE);

	mDaqIInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_A)
	{
		mDaqIInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_RISING);
		mDaqIInfo.setChannelTypes(DAQI_ANALOG_SE | DAQI_CTR32 | DAQI_DAC);
		mDaqIInfo.setMaxQueueLength(8); // 4 A/D and 3 counters and one D/A

		mDaqIInfo.setMaxScanRate(52734.0);
		mDaqIInfo.setMaxThroughput(mDaqIInfo.getMaxQueueLength() * 52734.0);
	}
	else if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
	{
		mDaqIInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_RISING);
		mDaqIInfo.setChannelTypes(DAQI_ANALOG_SE | DAQI_CTR32);
		mDaqIInfo.setMaxQueueLength(7); // 4 A/D and 3 counters
		mDaqIInfo.setMaxScanRate(105469.0);
		mDaqIInfo.setMaxThroughput(mDaqIInfo.getMaxQueueLength() * 105469.0);

		mDaqIInfo.setFifoSize(FIFO_SIZE * 2);
	}
	else
	{
		mDaqIInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_RISING | TRIG_FALLING);
		mDaqIInfo.setChannelTypes(DAQI_ANALOG_SE);
		mDaqIInfo.setMaxQueueLength(4); // 4 A/D
		mDaqIInfo.setMaxScanRate(105469.0);
		mDaqIInfo.setMaxThroughput(mDaqIInfo.getMaxQueueLength() * 105469.0);
	}

	setScanEndpointAddr(0x82);

	mVariablAdFifoSize = false;
	mPreviousSyncMode = -1;
	mPreviousClockFreq = -1;
	mOverrunOccurred = false;
	mGrpDelayTotalSamples = 0;
	mGrpDelaySamplesProcessed = 0;
	mFirstNoneAdcChanIdx = 0xffff;

	mHasDacChan = false;
	mDacChanIdx = 0;

	memset(&mSwapBuffer, 0, sizeof(mSwapBuffer));
}

DaqIUsb9837x::~DaqIUsb9837x()
{

}

void DaqIUsb9837x::initialize()
{
	try
	{
		//sendStopCmd();

		mVariablAdFifoSize = false;

		// get AD fifo size
		unsigned short InputFifoFlagMaskValue;

		// get number of samples fifo can hold
		dtDev().Cmd_ReadSingleWordFromLocalBus(Usb9837xDefs::INPUT_FIFO_FLAG_MASK, &InputFifoFlagMaskValue);

		if(InputFifoFlagMaskValue > 0x200)
			mVariablAdFifoSize = true;

		AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

		if(aiDev)
		{
			//aiDev->applyEepromIepeSettings(); Note: uncomment this line and comment out the following line if we need to apply default setting from EEPROM
			aiDev->configureIepe();
		}

		mPreviousSyncMode = -1;
		mPreviousClockFreq = -1;

	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double DaqIUsb9837x::daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data)
{
	UlLock lock(mIoDeviceMutex);

	check_DaqInScan_Args_(functionType, chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);

	resetScanErrorFlag();
	mOverrunOccurred = false;

	int epAddr = getScanEndpointAddr();

	setTransferMode(options, rate);

	AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		int sampleSize = 4;
		int aiResolution = aiDev->getAiInfo().getResolution();
		int chanCount = numChans;
		int stageSize = calcStageSize(epAddr, rate, chanCount,  samplesPerChan, sampleSize);

		std::vector<CalCoef> calCoefs = getScanCalCoefs(chanDescriptors, numChans, flags);
		std::vector<CustomScale> customScales = getCustomScales(chanDescriptors, numChans);

		daqDev().clearHalt(epAddr);

		setScanInfo(functionType, chanCount, samplesPerChan, sampleSize, aiResolution, options, flags, calCoefs, customScales, data);

		// coverity[sleep]
		configureScan(functionType, chanDescriptors, numChans, rate, options);
		configureFifoPacketSize(epAddr, rate, chanCount, samplesPerChan, options);

		daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

		try
		{
			Usb9837xDefs::SUBSYSTEM_INFO ssInfo;

			ssInfo.SubsystemType = Usb9837xDefs::SS_AD;
			ssInfo.ExtTrig = getTrigCode(functionType, options);

			dtDev().Cmd_StartSubsystem(&ssInfo);

			setScanState(SS_RUNNING);
		}
		catch(UlException& e)
		{
			stopBackground();
			throw e;
		}
	}

	return actualScanRate();
}

void DaqIUsb9837x::check_DaqInScan_Args_(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data) const
{
	DaqIDevice::check_DaqInScan_Args(chanDescriptors, numChans, samplesPerChan, rate, options, flags, data);

	if((options & SO_EXTTIMEBASE) && (options & SO_TIMEBASEOUT))
		throw UlException(ERR_BAD_OPTION);

	// external trigger is not supported in slave mode
	if((options & SO_EXTTIMEBASE) && (options & SO_EXTTRIGGER))
	{
		if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_C)
		{
			// we allow users to  specify Analog trigger when SO_EXTTIMEBASE option is specified to be able to disable group delay

			TriggerConfig trigCfg = daqDev().getTriggerConfig(functionType);
			if(trigCfg.type == TRIG_POS_EDGE || trigCfg.type == TRIG_NEG_EDGE)
				throw UlException(ERR_BAD_TRIG_TYPE);
		}
		else
			throw UlException(ERR_BAD_OPTION);
	}

	int lastAIChan = -1;
	int lastCtrChan = -1;

	for(unsigned int i = 0; i < (unsigned int) numChans; i++)
	{
		if(chanDescriptors[i].type == DAQI_ANALOG_SE)
		{
			if(chanDescriptors[i].channel <= lastAIChan)
				throw UlException(ERR_BAD_CHAN_ORDER);

			lastAIChan = chanDescriptors[i].channel;
		}
		else if(chanDescriptors[i].type == DAQI_CTR32)
		{
			if(chanDescriptors[i].channel <= lastCtrChan)
				throw UlException(ERR_BAD_CHAN_ORDER);

			lastCtrChan = chanDescriptors[i].channel;
		}
		else if(chanDescriptors[i].type == DAQI_DAC)
		{
			if(chanDescriptors[i].channel != 0)
				throw UlException(ERR_BAD_AO_CHAN);
		}
	}
}

void DaqIUsb9837x::configureFifoPacketSize(int epAddr, double rate, int chanCount, int sampleCount, ScanOption options) const
{
	// from OnReadWriteData()

	unsigned short	InputFifoFlagMaskValue = 0;

	if (mVariablAdFifoSize)
	{

		unsigned int minSamplesPerFifoNotEmptyInt = Usb9837xDefs::MIN_SAMPLES_PER_FIFO_NOT_EMPTY_INT;
		unsigned maxSamplesPerFifoNotEmptyInt = Usb9837xDefs::MAX_SAMPLES_PER_FIFO_NOT_EMPTY_INT;
		unsigned fifoSizeInSamples = Usb9837xDefs::FIFO_SIZE_IN_SAMPLES;

		// The FIFO on the DT9837B is double the other variations
		if (daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
		{
			minSamplesPerFifoNotEmptyInt*=2;
			maxSamplesPerFifoNotEmptyInt *=2;
			fifoSizeInSamples*=2;
		}

		double numSamplesPerPacket;

		if (getTransferMode() == SO_SINGLEIO)
		{
			numSamplesPerPacket = minSamplesPerFifoNotEmptyInt;
		}
		else
		{
			//double aggRate =  chanCount * rate ; // samples per second
			numSamplesPerPacket = daqDev().scanTranserIn()->getStageRate() * rate;
		}

		if(!(options & SO_CONTINUOUS))
		{
			if(numSamplesPerPacket > (/*chanCount **/ sampleCount))
				numSamplesPerPacket = /*chanCount */ sampleCount;
		}

		// Convert from bytes to samples
		//ULONG numSamplesPerBuffer = I.ReadSize()/4;
		//t << "numSamplesPerBuffer="<< numSamplesPerBuffer<<"\n";

		unsigned long threshold = 1;
		while (threshold < numSamplesPerPacket)
		{
			threshold <<= 1;
		}

		// Read 1/4 of data at a time
		//threshold >>= 2;

		// minimum read is 2 samples
		if (threshold < minSamplesPerFifoNotEmptyInt)
			threshold = minSamplesPerFifoNotEmptyInt;

		if (threshold > maxSamplesPerFifoNotEmptyInt)
			threshold = maxSamplesPerFifoNotEmptyInt;

		InputFifoFlagMaskValue = (unsigned short)(fifoSizeInSamples - threshold);

		// write the fifo flag mask (number of samples per xfer)
		dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::INPUT_FIFO_FLAG_MASK, (unsigned short) 0xFFFF, InputFifoFlagMaskValue);
	}
}

void DaqIUsb9837x::resetSyncMode()
{
	// reset the synchronization mode to none
	unsigned short generalControlRegMask = 0x3;
	unsigned short generalControlRegValue = 0x00;

	dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG0 , generalControlRegMask, generalControlRegValue);

	// from configureScan function below
	// Wait for the slave's PLL to lock with the master clock 3000 uSec worst case.
	// Wait another 128 samples for the A/D to settle
	usleep(5000); // 3000 + (2000 arbitrary value)

	dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG1 , ADC_SYNC_MASK, ADC_SYNC_BIT);

	mPreviousSyncMode = -1;
}

void DaqIUsb9837x::configureScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, double rate, ScanOption options) // from OnConfigure
{
	TriggerConfig trigCfg = daqDev().getTriggerConfig(functionType);

	unsigned short generalControlRegValue = 0x0;
	unsigned short generalControlRegMask = 0x0;

	int syncMode = SYNC_MODE_NONE;

	if(options & SO_EXTTIMEBASE)
		syncMode = SYNC_MODE_SLAVE;
	else if(options & SO_TIMEBASEOUT)
		syncMode = SYNC_MODE_MASTER;


	if(syncMode != mPreviousSyncMode)
	{
		mPreviousSyncMode = syncMode;

		// Set the synchronization mode
		generalControlRegMask = 0x3;
		switch (syncMode)
		{
			case SYNC_MODE_NONE:
				generalControlRegValue = 0x00;
				break;
			case SYNC_MODE_MASTER:
				generalControlRegValue = 0x02;
				break;
			case SYNC_MODE_SLAVE:
				generalControlRegValue = 0x01;
				break;
		}

		dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG0 , generalControlRegMask, generalControlRegValue);

		// Wait for the slave's PLL to lock with the master clock 3000 uSec worst case.
		// Wait another 128 samples for the A/D to settle
		long long syncPulseDelayInUSecs = (128 * 1000000 / rate) + 3000;

		usleep(syncPulseDelayInUSecs);

		dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG1 , ADC_SYNC_MASK, ADC_SYNC_BIT);
	}

	AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

	if(aiDev)
		aiDev->configureIepe();

	configureClock(chanDescriptors, numChans, rate, options);

	configureCGL(chanDescriptors, numChans);

	if(options & SO_EXTTRIGGER)
	{
		if(trigCfg.type == TRIG_RISING || trigCfg.type == TRIG_FALLING)
		{
			if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_C)
			{
				// Important: on DT9837_C module if trigger type is TRIG_RISING or TRIG_FALLING then
				// we need to disable group delay and process all of the samples
				mGrpDelayTotalSamples = 0;

				// validate and set the trigger configurations only in master mode
				if(!(options & SO_EXTTIMEBASE))
				{
					// Validate the trigger channel is part of the CGL
					bool validTrigChan = false;

					Range trigChanRange;

					for(int i = 0; i < numChans; i++)
					{
						if(chanDescriptors[i].channel == trigCfg.trigChan)
						{
							validTrigChan = true;
							trigChanRange = chanDescriptors[i].range;
							break;
						}
					}

					if(!validTrigChan)
						throw UlException(ERR_BAD_TRIG_CHANNEL);

					// Since the threshold start trigger on the DT9837C is digital, we need to use the channel gain
					// to calculate the raw 16-bit trigger code.

					unsigned int thresholdValue = 0;

					double gain = (trigChanRange == BIP1VOLTS) ? 10 : 1;

					AiUsb9837x::VoltsToRawValue(trigCfg.level, gain, &thresholdValue, 16);

					// Write the triggerChannel to the hardware
					dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::THRSHOLD_CNTRL_REG , Usb9837xDefs::THRSHOLD_CNTRL_REG_MASK, trigCfg.trigChan);


					// Write the trigger threshold value to the hardware
					dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::THRSHOLD_TRIGLEVEL_REG, 0xFFFF, (unsigned short) thresholdValue);
				}
			}
			else
			{
				dtDev().CmdSetAnalogTriggerThreshold(trigCfg.level);
			}
		}
	}

	// Setup Stop trigger
	// SB: Stop trigger or analog reference trigger is a software trigger, users can achieve the behavior of OL by
	// monitoring the the circular buffer data

}


void DaqIUsb9837x::configureClock(DaqInChanDescriptor chanDescriptors[], int numChans, double rate, ScanOption options)
{
	// SB: moved after calculating the actual rate
	// If A/D clock frequency hasn't changed then just exit
	//if(iPreviousClockFreq == iRate)
	//	return;

	// check if analog channel range changed since last run then reset clock otherwise bad samples will be returned at the beginning of scan
	AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		for(int idx = 0 ; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQI_ANALOG_SE)
			{
				Range currentRange = aiDev->getCurrentChanRange(chanDescriptors[idx].channel);
				if(chanDescriptors[idx].range != currentRange)
				{
					mPreviousClockFreq = -1;
					break;
				}
			}
		}
	}

	if(rate < mDaqIInfo.getMinScanRate())
		rate = mDaqIInfo.getMinScanRate();

	Usb9837xDefs::CY22150REGISTERS SPIRegisters;

	double freqIn;
	double freqRef;
	double actualFreq;
	double fOut;
	long long	DelayTimeInUSecs;
	long long syncPulseDelayInUSecs;
	bool doSyncPulseDelay = false;

	freqRef = 48;
	unsigned char divider;
	unsigned short regValue = 0;

	freqIn = rate;

	bool bIsHighFrequency = false;

	if (freqIn > (double) ADS1271_LOW_POWER_UPPER_FREQ)
		bIsHighFrequency = true;

	Usb9837x::programClock(freqIn, freqRef , &actualFreq, &SPIRegisters,&fOut,&divider, false);

	// If A/D clock frequency hasn't changed then just exit
	if(mPreviousClockFreq == actualFreq)
		return;

	// if we are transitioning for high or low speed mode we need to perform delay to synchronize the A/D
	if (bIsHighFrequency && (mPreviousClockFreq <=  ADS1271_LOW_POWER_UPPER_FREQ))
	{
		doSyncPulseDelay = true;
		syncPulseDelayInUSecs = (long long) ((12288.0/(actualFreq * 256)) * 1000000);
	}

	if (!bIsHighFrequency && (mPreviousClockFreq >  ADS1271_LOW_POWER_UPPER_FREQ))
	{
		doSyncPulseDelay = true;
		syncPulseDelayInUSecs = (long long) ((12288.0/(actualFreq * 512)) * 1000000);
	}

	//CONVERT_DBL_TO_LARGE_INT(actualFreq,pCfg->liClockFrequency);
	mPreviousClockFreq = actualFreq;

	// Calculate time in uSecs of 256 A/D clock cycles. That is the time we need to
	// delay after synchronizing the channels.
	DelayTimeInUSecs = (unsigned long long) (1 / actualFreq * 256 * 1000000);

	// Set the correct value for the AD Mode bit in Control Register 1
	if (bIsHighFrequency)
	{	// set the AD mode bit to high power / high speed mode
		dtDev().Cmd_RMWSingleWordToLocalBus( Usb9837xDefs::GENERAL_CNTRL_REG1, (unsigned short)ADC_MODE_MASK, (unsigned short) ADC_MODE_BIT );
	}
	else
	{	// set the AD mode bit to low power mode
		dtDev().Cmd_RMWSingleWordToLocalBus( Usb9837xDefs::GENERAL_CNTRL_REG1, (unsigned short) ADC_MODE_MASK, 0 );
	}

	if (doSyncPulseDelay)
	{
		usleep(syncPulseDelayInUSecs);
	}

	// Write the data to the PLL
	unsigned char numWrites = 0;

	Usb9837xDefs::WRITE_BYTE_INFO	m_writeByteInfo[10];

	m_writeByteInfo[numWrites].Address = Usb9837xDefs::DIV1SRC_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.Div1Src;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


	numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::DIV2SRC_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = (unsigned char)(SPIRegisters.Div1Src <<1);
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);


	numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CHARGE_PUMP_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.ChargePump;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::PB_COUNTER_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.PBCounter;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	numWrites = 0;
	m_writeByteInfo[numWrites].Address = Usb9837xDefs::PO_Q_COUNTER_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.POQCounter;
	numWrites++;
	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	numWrites = 0;

	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CROSSPOINT_SWITCH_MATRIX_CNTRL_0_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.CrossPointSw0;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	numWrites = 0;

	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CROSSPOINT_SWITCH_MATRIX_CNTRL_1_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.CrossPointSw1;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	numWrites = 0;

	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CROSSPOINT_SWITCH_MATRIX_CNTRL_2_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.CrossPointSw2;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	numWrites = 0;

	m_writeByteInfo[numWrites].Address = Usb9837xDefs::CLKOE_REG_ADDR;
	m_writeByteInfo[numWrites].DataVal = SPIRegisters.ClkOe;
	numWrites++;

	dtDev().Cmd_WriteMultiplePLLReg(Usb9837xDefs::SS_AD, Usb9837xDefs::PLL_DEV_ADDR, numWrites, m_writeByteInfo);

	// Bits [9:8] control the divider selection
	regValue =0;
	switch(divider)
	{
		case 2:
			regValue = 0x0 <<8;		// 00 - Divide by 2
			break;
		case 4:
			regValue = 0x1 <<8;		// 01 - Divide by 4
			break;
		case 8:
			regValue = 0x2 <<8;		// 10 - Divide by 8
			break;
		case 16:
			regValue = 0x3 <<8;		// 11 - Divide by 16
			break;
	}

	// Write the divider to Control Register0
	dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG0, (0x3<<8), regValue);

	if(options & SO_EXTTRIGGER)
	{
		// Wait delay time so that channel resync can complete before continuing.
		usleep(DelayTimeInUSecs);

	}

	setActualScanRate(actualFreq);
}

//Configures the channel gain list in the board
void DaqIUsb9837x::configureCGL(DaqInChanDescriptor chanDescriptors[], int numChans)
{
	// added for cov
	UlLock lock(mProcessScanDataMutex);

	unsigned char gainMask = 0;
	unsigned short GeneralControlReg2= 0x0;
	unsigned short GeneralControlReg5 = 0x0;
	unsigned short channelGainSelMask [] = {0x1<<4,0x1<<5,0x1<<6,0x1<<7};

	mFirstNoneAdcChanIdx = 0xffff; //initial value
	int idx = 0;
	mHasDacChan = false;
	mDacChanIdx = 0;

	mGrpDelayTotalSamples = numChans * Usb9837xDefs::GRP_DELAY_SIZE_IN_SAMPLES;
	mGrpDelaySamplesProcessed = 0;
	mSwapBuffer.idx = 0;
	mSwapBuffer.size = mGrpDelayTotalSamples;

	int actualChanNum = 0;
	for(int i = 0; i < numChans; i++)
	{
		actualChanNum = chanDescriptors[i].channel;

		if(chanDescriptors[i].type == DAQI_ANALOG_SE)
		{
			gainMask |= channelGainSelMask[actualChanNum];
			GeneralControlReg2 |= (chanDescriptors[i].range ==  BIP1VOLTS) ? channelGainSelMask[actualChanNum] : 0x0;

			const AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

			if(aiDev)
			{
				aiDev->setCurrentChanRange(chanDescriptors[i].channel, chanDescriptors[i].range);
			}
		}
		else
		{

			if(chanDescriptors[i].type == DAQI_CTR32)
			{
				actualChanNum += 4;
			}
			else if(chanDescriptors[i].type == DAQI_DAC)
			{
				actualChanNum = 7;

				mHasDacChan = true;
				mDacChanIdx = idx;
			}

			if(mFirstNoneAdcChanIdx == 0xffff)
				mFirstNoneAdcChanIdx = idx;
		}

		// Set the mask indicating whether to add the channel to input stream
		GeneralControlReg5 |= 0x1 << actualChanNum;

		idx++;
	}

	//Send the gain value to ControlReg2
	dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG2 , gainMask, GeneralControlReg2);

	//Send the channel selection to ControlReg5
	dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG5 , 0xFF, GeneralControlReg5);
}




std::vector<CalCoef> DaqIUsb9837x::getScanCalCoefs(DaqInChanDescriptor chanDescriptors[], int numChans, DaqInScanFlag flags) const
{
	std::vector<CalCoef> calCoefs;

	CalCoef calCoef;
	AiInputMode inputMode;

	const AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		for(int idx = 0; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQI_ANALOG_SE || chanDescriptors[idx].type == DAQI_ANALOG_DIFF)
			{
				inputMode = chanDescriptors[idx].type == DAQI_ANALOG_SE ? AI_SINGLE_ENDED : AI_DIFFERENTIAL;

				calCoef = aiDev->getChanCalCoef(chanDescriptors[idx].channel, inputMode, chanDescriptors[idx].range, flags);
			}
			else if(chanDescriptors[idx].type == DAQI_DAC)
			{
				const AoUsb9837x* aoDev = dynamic_cast<AoUsb9837x*>(mDaqDevice.aoDevice());

				if(aoDev)
				{
					calCoef = aoDev->getInputChanCalCoef(chanDescriptors[idx].channel, flags);
				}
				else
				{
					calCoef.slope = 1;
					calCoef.offset = 0;
				}
			}
			else
			{
				calCoef.slope = 1;
				calCoef.offset = 0;
			}

			calCoefs.push_back(calCoef);
		}
	}

	return calCoefs;
}

std::vector<CustomScale> DaqIUsb9837x::getCustomScales(DaqInChanDescriptor chanDescriptors[], int numChans) const
{
	std::vector<CustomScale> customScales;

	CustomScale customScale;

	const AiUsb9837x* aiDev = dynamic_cast<AiUsb9837x*>(mDaqDevice.aiDevice());

	if(aiDev)
	{
		for(int idx = 0; idx < numChans; idx++)
		{
			if(chanDescriptors[idx].type == DAQI_ANALOG_SE || chanDescriptors[idx].type == DAQI_ANALOG_DIFF)
			{
				customScale = aiDev->getChanCustomScale(chanDescriptors[idx].channel);
			}
			else
			{
				customScale.slope = 1;
				customScale.offset = 0;
			}

			customScales.push_back(customScale);
		}
	}

	return customScales;
}

unsigned short DaqIUsb9837x::getTrigCode(FunctionType functionType, ScanOption options)
{
	unsigned short code = 0;

	TriggerConfig trigCfg = daqDev().getTriggerConfig(functionType);

	if(options & SO_EXTTRIGGER)
	{
		switch (trigCfg.type)
		{
			case TRIG_POS_EDGE:
				code = Usb9837xDefs::SUBSYS_FLG_POS_EXTTRIG;
				break;
			case TRIG_NEG_EDGE:
				code = Usb9837xDefs::SUBSYS_FLG_NEG_EXTTRIG;
				break;
			case TRIG_RISING:
				code = Usb9837xDefs::SUBSYS_FLG_POS_THRESHOLDTRIG;
				break;

			case TRIG_FALLING:
				code = Usb9837xDefs::SUBSYS_FLG_NEG_THRESHOLDTRIG;
				break;
			default:
				break;
		}
	}

	return code;
}

void DaqIUsb9837x::sendStopCmd()
{
	Usb9837xDefs::SUBSYSTEM_INFO ssInfo;

	ssInfo.SubsystemType = Usb9837xDefs::SS_AD;
	ssInfo.ExtTrig = 0;

	dtDev().Cmd_StopSubsystem(&ssInfo);
}

UlError DaqIUsb9837x::terminateScan()
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

	daqDev().scanTranserIn()->stopTransfers();

	return err;
}

UlError DaqIUsb9837x::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;

	if(mOverrunOccurred)
		err = ERR_OVERRUN;

	return err;
}

void DaqIUsb9837x::check_DaqInSetTrigger_Args(TriggerType trigType, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) const
{
	DaqIDevice::check_DaqInSetTrigger_Args(trigType, trigChanDesc, level, variance, retriggerCount);

	if(trigType & TRIG_RISING)
	{
		if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_A ||
		   daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
		{
			if(trigChanDesc.channel != 0)
				throw UlException(ERR_BAD_TRIG_CHANNEL);

			double VoltageRangeHigh = 9.8;
			double VoltageRangeLow = 0.2;

			if ((level >= VoltageRangeHigh) || (level <= VoltageRangeLow))
				throw UlException(ERR_BAD_TRIG_LEVEL);
		}
	}
}


void DaqIUsb9837x::processScanData32_dbl(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	if(mGrpDelaySamplesProcessed < mGrpDelayTotalSamples)
	{
		// if all of the channels in the scan list are ADC channels throw away the group delay samples otherwise
		//retain the group delay samples in the swap buffer for none ADC channels

		int i = 0;
		for(i = 0; i < requestSampleCount; i++)
		{
			mSwapBuffer.buf_dbl[mSwapBuffer.idx] = Endian::le_ui32_to_cpu(buffer[i]); // All samples are copied to the swap buffer but we only look at no ADC data
			mSwapBuffer.idx++;

			mGrpDelaySamplesProcessed++;

			if(mGrpDelaySamplesProcessed == mGrpDelayTotalSamples)
			{
				mSwapBuffer.idx = 0;
				i++;
				break;
			}
		}

		if(i < requestSampleCount)
			buffer = &buffer[i];

		requestSampleCount = requestSampleCount - i;
	}

	if(requestSampleCount > 0)
	{
		double data;
		unsigned int rawVal;

		double* dataBuffer = (double*) mScanInfo.dataBuffer;

		while(numOfSampleCopied < requestSampleCount)
		{
			rawVal = Endian::le_ui32_to_cpu(buffer[numOfSampleCopied]);

			if(mScanInfo.currentCalCoefIdx < mFirstNoneAdcChanIdx ) // ADC channels
			{
				if(mScanInfo.flags & NOSCALEDATA)
					data = rawVal;
				else
				{
					data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * rawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
				}

				dataBuffer[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;
			}
			else
			{
				data = mSwapBuffer.buf_dbl[mSwapBuffer.idx];
				mSwapBuffer.buf_dbl[mSwapBuffer.idx] = rawVal;

				if(mHasDacChan && (mScanInfo.currentCalCoefIdx == mDacChanIdx)) // DAC channel
				{
					if(!(mScanInfo.flags & NOSCALEDATA))
					{
						data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * data) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;
					}
				}

				dataBuffer[mScanInfo.currentDataBufferIdx] = data;
			}

			mSwapBuffer.idx++;

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

			if(mSwapBuffer.idx == mSwapBuffer.size)
				mSwapBuffer.idx = 0;

		}
	}
}

} /* namespace ul */
