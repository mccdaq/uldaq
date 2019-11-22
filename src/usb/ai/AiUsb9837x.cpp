/*
 * AiUsb9837x.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AiUsb9837x.h"
#include "../daqi/DaqIUsb9837x.h"

#include <unistd.h>

namespace ul
{
AiUsb9837x::AiUsb9837x(const Usb9837x& daqDevice) : AiUsbBase(daqDevice)
{
	double minRate = 195.313;

	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA);
	mAiInfo.setAInScanFlags(AINSCAN_FF_NOSCALEDATA);

	mAiInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_SINGLEIO|SO_BLOCKIO|SO_EXTTIMEBASE|SO_TIMEBASEOUT);
	mAiInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_RISING);

	if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_C)
		mAiInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_RISING | TRIG_FALLING);

	mAiInfo.hasPacer(true);

	mAiInfo.setNumChans(4);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 4);
	mAiInfo.setChanTypes(AI_VOLTAGE);
	mAiInfo.setChanTypes(0, 3, AI_VOLTAGE);

	mAiInfo.setResolution(24);
	mAiInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_A)
	{
		mAiInfo.setMaxScanRate(52734.0);
		mAiInfo.setMaxThroughput(52734.0 * 4);
	}
	else
	{
		mAiInfo.setMaxScanRate(105469.0);
		mAiInfo.setMaxThroughput(105469.0 * 4);
	}

	mAiInfo.setMaxBurstRate(0);
	mAiInfo.setMaxBurstThroughput(0);
	mAiInfo.setFifoSize(FIFO_SIZE);

	if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
		mAiInfo.setFifoSize(FIFO_SIZE * 2);


	mAiInfo.addInputMode(AI_SINGLE_ENDED);

	mAiInfo.setCalCoefCount(4);
	mAiInfo.setSampleSize(4);

	mAiInfo.supportsIepe(true);

	addSupportedRanges();
	addQueueInfo();

	setScanEndpointAddr(0x82);

	initCustomScales();

	for(int i = 0; i < NUMBER_OF_ADC; i++)
	{
		mCurrentSource[i] = DISABLED;
		mCouplingType[i] = DC;
		mSensorSensitivity[i] = 1.0;
		mCurrentChanRange[i] = (Range) -1;
	}
}

AiUsb9837x::~AiUsb9837x()
{

}

void AiUsb9837x::initialize()
{
	try
	{
		loadAdcCoefficients();


		DaqIUsb9837x* daqIDev = dynamic_cast<DaqIUsb9837x*>(mDaqDevice.daqIDevice());

		// running aIn while the the device is in slave mode will result in device not responding error. since the state of sync mode
		// is unknown here we reset the sync mode. (i.e if device left in slave mode last time the program was closed)

		if(daqIDev)
			daqIDev->resetSyncMode();

		// in order to convert AOutScan trigger threshold to count we need to know what range is used for the analog trigger channel (+/-10 or +/-1)
		// AIn is called with +/-10 range to set all channels to +/-10 and from this point we keep track of the last range used for each individual
		// analog input channel
		for(int ch = 0; ch < NUMBER_OF_ADC; ch++)
		{
			aIn(ch, AI_SINGLE_ENDED, BIP10VOLTS, AIN_FF_DEFAULT);
			mCurrentChanRange[ch] = BIP10VOLTS;
		}

	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}




double AiUsb9837x::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	// calling AIn will change the clock rate to 50KHz therefore DaqI object need to be notified to apply the scan rate for next scan
	DaqIUsb9837x* daqIDev = dynamic_cast<DaqIUsb9837x*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		daqIDev->resetClockFreq();

		// running aIn while the the device is in slave mode will result in device not responding error
		if(daqIDev->syncMode() == DaqIUsb9837x::SYNC_MODE_SLAVE)
		{
			// coverity[sleep]
			daqIDev->resetSyncMode();
		}
	}

	double data = 0.0;
	unsigned int rawVal = 0;

	Usb9837xDefs::READ_SINGLE_VALUE_INFO readSingleValueInfo;

	readSingleValueInfo.Channel = channel;
	readSingleValueInfo.Gain = (range == BIP1VOLTS) ? 10 : 1;

	dtDev().Cmd_ReadSingleValue(&readSingleValueInfo, &rawVal);

	data = calibrateData(channel, inputMode, range, rawVal, flags);

	CustomScale customScale = getChanCustomScale(channel);

	data = customScale.slope * data + customScale.offset;

	setCurrentChanRange(channel, range);

	return data;
}

double AiUsb9837x::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsb9837x* daqIDev = dynamic_cast<DaqIUsb9837x*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		int numChans = (queueEnabled() ? queueLength() : highChan - lowChan + 1);

		DaqInChanDescriptor* chanDescriptors = new DaqInChanDescriptor[numChans];

		for(int i = 0; i < numChans; i++)
		{
			if(queueEnabled())
			{
				chanDescriptors[i].channel = mAQueue[i].channel;
				chanDescriptors[i].type = (mAQueue[i].inputMode == AI_DIFFERENTIAL) ? DAQI_ANALOG_DIFF : DAQI_ANALOG_SE;
				chanDescriptors[i].range = mAQueue[i].range;
			}
			else
			{
				chanDescriptors[i].channel = lowChan + i;
				chanDescriptors[i].type = (inputMode == AI_DIFFERENTIAL) ? DAQI_ANALOG_DIFF : DAQI_ANALOG_SE;
				chanDescriptors[i].range = range;
			}
		}

		actualRate =  daqIDev->daqInScan(FT_AI, chanDescriptors, numChans, samplesPerChan, rate, options, (DaqInScanFlag) flags, data);

		delete [] chanDescriptors;
	}

	return actualRate;
}

void AiUsb9837x::check_AInSetTrigger_Args(TriggerType trigType, int trigChan, double level, double variance, unsigned int retriggerCount) const
{
	AiDevice::check_AInSetTrigger_Args(trigType, trigChan, level, variance, retriggerCount);

	if(trigType & TRIG_RISING)
	{
		if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_A ||
		   daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
		{
			if(trigChan != 0)
				throw UlException(ERR_BAD_TRIG_CHANNEL);

			double VoltageRangeHigh = 9.8;
			double VoltageRangeLow = 0.2;

			if ((level >= VoltageRangeHigh) || (level <= VoltageRangeLow))
				throw UlException(ERR_BAD_TRIG_LEVEL);
		}
	}
}

void AiUsb9837x::VoltsToRawValue (double volts, double gain, unsigned int* rawValue, int Resolution)
{
	// fix any out of range requests before converting
	double VoltageRangeHigh = 10;
	double VoltageRangeLow = -10;


	double dfFullScaleRange = VoltageRangeHigh - VoltageRangeLow;

	//double oneLsb = dfFullScaleRange / (1 <<  Resolution);
	if ((volts >= VoltageRangeHigh / gain) || (volts <= VoltageRangeLow / gain))
	{
		throw UlException(ERR_BAD_TRIG_LEVEL);
	}

	double dfRatioVoltsToFullScale = ( ( volts * gain) - VoltageRangeLow ) / dfFullScaleRange;

	*rawValue = (int)(dfRatioVoltsToFullScale * ( (double) (1 <<  Resolution)));
}

void AiUsb9837x::applyEepromIepeSettings()
{
	// Set the default coupling and current source. The defaults are stored in EEPROM.
	// If for some reason those defaults have not been made, use out absolute defaults.

	for (int i = 0; i < NUMBER_OF_ADC; i++)
	{
		// Read EEPROM Coupling data for this ADC
		unsigned char address = Usb9837xDefs::EEPROM_OFFSET_COUPLING_0 + i;
		unsigned char data;
		dtDev().Cmd_ReadDevMultipleRegs(Usb9837xDefs::EEPROM_DEV_ADR, sizeof(data), &address, &data);

		if ((data != AC) && (data != DC))
		{
			switch (daqDev().getDeviceType())
			{
				case DaqDeviceId::UL_DT9837_B:
					mCouplingType[i] = AC; // default coupling for model B is AC
					break;
				default:
					// 9837A is the default
					mCouplingType[i] = DC;
					break;
			}
		}
		else
		{
			mCouplingType[i] = data;
		}

		// Read EEPROM Current Source data for this ADC
		address = Usb9837xDefs::EEPROM_OFFSET_CURRENT_SOURCE_0 + i;
		dtDev().Cmd_ReadDevMultipleRegs(Usb9837xDefs::EEPROM_DEV_ADR, sizeof(data), &address, &data);
		if ((data != DISABLED) && (data != INTERNAL) && (data != EXTERNAL))
		{
			switch (daqDev().getDeviceType())
			{
				case DaqDeviceId::UL_DT9837_B:
					mCurrentSource[i] = DISABLED;
					break;
				default:
					// 9837A is the default
					mCurrentSource[i] = DISABLED;
					break;
			}
		}
		else
		{
			mCurrentSource[i] = data;
		}
	}

	configureIepe();

}

void AiUsb9837x::configureIepe()
{
	// While we have no control over how the user send down commands to the driver,
	// we can do what we can to take the bundled changes and program the hardware
	// in the order we'd like to see it done. Not perfect but better than nothing.

	// If we're setting up for AC Coupling, do that first, then set the current source
	// If we're setting up for DC Coupling, set the current source first.
	// Since each ADC can be programmed independently, set each one at a time.

	unsigned short CurrentSourceBitMask [4] = {0x1<<8, 0x1<<9, 0x1<<10, 0x1<<11};
	unsigned short CouplingTypeBitMask [4] = {0x1<<0, 0x1<<1, 0x1<<2, 0x1<<3};
	unsigned short CurrentSourceValue;

	for (int i = 0; i < NUMBER_OF_ADC; i++)
	{
		CurrentSourceValue = (mCurrentSource[i] == INTERNAL) ? CurrentSourceBitMask[i] : 0x0;

		// What's the new Coupling ?
		if (mCouplingType[i] == AC)
		{	// AC
			// Set coupling
			dtDev().Cmd_RMWSingleWordToLocalBus((unsigned short) (Usb9837xDefs::GENERAL_CNTRL_REG2) ,
																	CouplingTypeBitMask[i],
																	CouplingTypeBitMask[i]);

			// Set current source
			dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG2 ,
																	CurrentSourceBitMask[i],
																	CurrentSourceValue);
		}
		else
		{	// DC
			// Set Current source
			dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG2,
																	CurrentSourceBitMask[i],
																	CurrentSourceValue);

			// Set Coupling
			dtDev().Cmd_RMWSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG2 ,
																	CouplingTypeBitMask[i],
																	0);
		}
	}
}

void AiUsb9837x::loadAdcCoefficients()
{
	CalCoef calCoef;

	if(getScanState() == SS_IDLE)
	{
		mCalCoefs.clear();

		int calCoefCount = mAiInfo.getCalCoefCount();

		for(int i = 0; i < calCoefCount; i++)
		{
			calCoef.slope = 1.0;
			calCoef.offset = 0.0;

			mCalCoefs.push_back(calCoef);
		}
	}
}

CalCoef AiUsb9837x::getChanCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const
{
	return getCalCoef(channel, inputMode, range, flags);
}

CustomScale AiUsb9837x::getChanCustomScale(int channel) const
{
	CustomScale customScale = mCustomScales[channel];
	customScale.slope /= mSensorSensitivity[channel];

	return customScale;
}

UlError AiUsb9837x::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return mDaqDevice.daqIDevice()->getStatus(FT_AI, status, xferStatus);
}

UlError AiUsb9837x::waitUntilDone(double timeout)
{
	return mDaqDevice.daqIDevice()->waitUntilDone(FT_AI, timeout);
}


void AiUsb9837x::stopBackground()
{
	mDaqDevice.daqIDevice()->stopBackground(FT_AI);
}

ScanStatus AiUsb9837x::getScanState() const
{
	return mDaqDevice.daqIDevice()->getScanState();
}

void AiUsb9837x::setCfg_ChanIepeMode(int channel, IepeMode mode)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(mode != IEPE_DISABLED && mode != IEPE_ENABLED)
		throw UlException(ERR_BAD_IEPE_MODE);

	if(mode == IEPE_ENABLED)
		mCurrentSource[channel] = INTERNAL;
	else
		mCurrentSource[channel] = DISABLED;
}

IepeMode AiUsb9837x::getCfg_ChanIepeMode(int channel)
{
	IepeMode mode = IEPE_DISABLED;

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(mCurrentSource[channel] == INTERNAL)
		mode = IEPE_ENABLED;

	return mode;
}

void AiUsb9837x::setCfg_ChanCouplingMode(int channel, CouplingMode mode)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(mode != CM_DC && mode != CM_AC)
		throw UlException(ERR_BAD_COUPLING_MODE);

	if(mode == CM_AC)
		mCouplingType[channel] = AC;
	else
		mCouplingType[channel] = DC;
}

CouplingMode AiUsb9837x::getCfg_ChanCouplingMode(int channel)
{
	CouplingMode mode = CM_DC;

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(mCouplingType[channel] == AC)
		mode = CM_AC;

	return mode;
}

void AiUsb9837x::setCfg_ChanSensorSensitivity(int channel, double sensitivity)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(sensitivity < 100E-9) // minimum sensitivity from NIMAX
		throw UlException(ERR_BAD_SENSOR_SENSITIVITY);

	mSensorSensitivity[channel] = sensitivity;
}

double AiUsb9837x::getCfg_ChanSensorSensitivity(int channel)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	return mSensorSensitivity[channel];
}

void AiUsb9837x::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP1VOLTS);
}

void AiUsb9837x::addQueueInfo()
{
	mAiInfo.setMaxQueueLength(AI_DIFFERENTIAL, 0);
	mAiInfo.setMaxQueueLength(AI_SINGLE_ENDED, 4);

	// Add queue types
	mAiInfo.setQueueTypes(CHAN_QUEUE | GAIN_QUEUE);

	// Add queue limitations
	mAiInfo.setChanQueueLimitations(UNIQUE_CHAN | ASCENDING_CHAN);
}

void AiUsb9837x::setCurrentChanRange(int channel, Range range) const
{
	if(channel < NUMBER_OF_ADC)
		mCurrentChanRange[channel] = range;
}

Range AiUsb9837x::getCurrentChanRange(int channel) const
{
	Range range = BIP10VOLTS;

	if(channel < NUMBER_OF_ADC)
		range = mCurrentChanRange[channel];

	return range;
}

} /* namespace ul */
