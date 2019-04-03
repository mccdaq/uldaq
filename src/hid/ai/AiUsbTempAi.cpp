/*
 * AiUsbTempAi.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsbTempAi.h"
#include <unistd.h>
#include <iostream>
#include <sstream>

namespace ul
{

AiUsbTempAi::AiUsbTempAi(const HidDaqDevice& daqDevice) : AiHidBase(daqDevice)
{
	mAiInfo.setNumChans(8);
	mAiInfo.setNumCjcChans(2);
	mAiInfo.hasTempChan(true);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 8);
	mAiInfo.setNumChansByMode(AI_SINGLE_ENDED, 4);
	mAiInfo.setResolution(24);
	mAiInfo.setAInFlags(AIN_FF_NOSCALEDATA);
	mAiInfo.setTInFlags(TIN_FF_DEFAULT);
	mAiInfo.setTInArrayFlags(TINARRAY_FF_DEFAULT);


	if(daqDev().getDeviceType() == DaqDeviceId::USB_TEMP_AI)
	{
		mAiInfo.setChanTypes(AI_VOLTAGE | AI_TC | AI_RTD | AI_THERMISTOR | AI_SEMICONDUCTOR);
		mAiInfo.setChanTypes(0, 3, AI_TC | AI_RTD | AI_THERMISTOR | AI_SEMICONDUCTOR);
		mAiInfo.setChanTypes(4, 7, AI_VOLTAGE);
	}
	else
	{
		mAiInfo.setChanTypes(AI_VOLTAGE | AI_TC);
		mAiInfo.setChanTypes(0, 3, AI_TC);
		mAiInfo.setChanTypes(4, 7, AI_VOLTAGE);
	}

	mAiInfo.addInputMode(AI_SINGLE_ENDED);
	mAiInfo.addInputMode(AI_DIFFERENTIAL);

	mAiInfo.setCalDateAddr(0xF0);

	addSupportedRanges();

	//initTempUnits();
	initCustomScales();

	memset(mCurrentChanCfg, 0, sizeof(mCurrentChanCfg));
}

AiUsbTempAi::~AiUsbTempAi()
{

}

void AiUsbTempAi::initialize()
{
	try
	{
		readCalDate();

		unsigned char adc, adcChan, subItem, modeCode, rangeCode;

		// read and store voltage channels mode and range
		for(int chan = 4; chan < mAiInfo.getNumChans(); chan++)
		{
			adc = chan / 2;
			adcChan = chan % 2;
			subItem = SUBITEM_CHAN_MODE + adcChan;

			daqDev().queryCmd(CMD_GETITEM, adc, subItem, &modeCode);

			mCurrentChanCfg[chan].inputMode = (AiInputMode) 0;

			if(modeCode == 0)
				mCurrentChanCfg[chan].inputMode = AI_DIFFERENTIAL;
			else if(modeCode == 1)
				mCurrentChanCfg[chan].inputMode = AI_SINGLE_ENDED;


			subItem = SUBITEM_CHAN_RANGE + adcChan;

			daqDev().queryCmd(CMD_GETITEM, adc, subItem, &rangeCode);

			mCurrentChanCfg[chan].range = (Range) 0;

			switch(rangeCode)
			{
			case 2:
				mCurrentChanCfg[chan].range = BIP10VOLTS;
				break;
			case 3:
				mCurrentChanCfg[chan].range = BIP5VOLTS;
				break;
			case 4:
				mCurrentChanCfg[chan].range = BIP2PT5VOLTS;
				break;
			case 5:
				mCurrentChanCfg[chan].range = BIP1PT25VOLTS;
				break;
			}
		}
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

double AiUsbTempAi::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;
	bool chanCfgChanged = false;

	if(channel >= 4) // voltage channels
	{
		if(mCurrentChanCfg[channel].inputMode != inputMode)
		{
			setInputMode(channel, inputMode);
			chanCfgChanged = true;
		}

		if(mCurrentChanCfg[channel].range != range)
		{
			setRange(channel, range);
			chanCfgChanged = true;
		}

		if(chanCfgChanged)
			usleep(1000000);

		unsigned char chan = channel;
		unsigned char units = 0;

		if(flags & AIN_FF_NOSCALEDATA)
			units = 1;

		float fData;
		daqDev().queryCmd(CMD_AIN, chan, units, &fData);

		data = mEndian.le_ptr_to_cpu_f32((unsigned char*) &fData);
		data = mCustomScales[channel].slope * data + mCustomScales[channel].offset;

	}
	else
		throw UlException(ERR_BAD_AI_CHAN);

	return data;
}

void AiUsbTempAi::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	check_TIn_Args(channel, scale, flags);

	float tempValue = 0;

	unsigned char chan = channel;
	unsigned char units = 0;

	// remove if we need TIn to read voltage channels
	if(channel > 3)
		throw UlException(ERR_BAD_AI_CHAN);

	if(channel < 4 && (scale == TS_VOLTS || scale == TS_NOSCALE))
		units = 1;

	float fData;
	daqDev().queryCmd(CMD_AIN, chan, units, &fData);

	tempValue = mEndian.le_ptr_to_cpu_f32((unsigned char*) &fData);

	if(channel < 4) // perform conversion if a temp channel
	{
		switch((int)(tempValue))
		{
		case -8888:            // if the temp Value is -8888.0  TC open connection is detected
		  *data =-9999.0f;
		  throw UlException(ERR_OPEN_CONNECTION);
		  break;
		case -9000:            // if the temp Value is -9000.0  device is not ready yet
		throw UlException(ERR_DEV_NOT_READY);
		  break;
		default:
			tempValue = convertTempUnit(tempValue, (TempUnit)scale);

			if(channel & 0x80) // CJC chan
				*data = tempValue;
			else
				*data = mCustomScales[channel].slope * tempValue + mCustomScales[channel].offset;

			break;
		}
	}
}
void AiUsbTempAi::tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[])
{
	check_TInArray_Args(lowChan, highChan, scale, flags, data);

	int chanCount = highChan - lowChan + 1;
	unsigned char startChan = lowChan;
	unsigned char endChan = highChan;
	unsigned char units = 0;

	bool openConnection = false;

	if(scale == TS_VOLTS || scale == TS_NOSCALE)
		units = 1;

	float fData[8];
	float tempValue = 0;
	int channel = 0;

	memset(fData, 0 , 8 * sizeof(float));

	daqDev().queryCmd(CMD_AINSCAN, startChan, endChan, units, (unsigned char*) fData, chanCount * sizeof(float));

	for(int i = 0; i < chanCount; i++)
	{
		tempValue = mEndian.le_ptr_to_cpu_f32((unsigned char*)&fData[i]);
		channel = lowChan + i;

		if((lowChan + i) < 4) // perform conversion if a temp channel
		{
			switch((int)(tempValue))
			{
			case -8888:            // if the temp Value is -8888.0  TC open connection is detected
				data[i] = -9999.0f;
				openConnection = true;
			  break;
			case -9000:            // if the temp Value is -9000.0  device is not ready yet
				throw UlException(ERR_DEV_NOT_READY);
			  break;            // if the temp Value is -9000.0  device is not ready yet
			case -9999:
				data[i] = tempValue;
			  break;
			default:
				data[i] = convertTempUnit(tempValue, (TempUnit)scale);
				data[i] = mCustomScales[channel].slope * data[i] + mCustomScales[channel].offset;
			  break;
			}
		}
	}

	if(openConnection)
		throw UlException(ERR_OPEN_CONNECTION);
}

void AiUsbTempAi::setInputMode(int channel, AiInputMode mode)
{
	unsigned char adc = channel / 2;
	unsigned char adcChan = channel  % 2;
	unsigned char subItem = SUBITEM_CHAN_MODE + adcChan;
	unsigned char modeCode = (mode == AI_SINGLE_ENDED) ? 1 : 0;

	daqDev().sendCmd(CMD_SETITEM, adc, subItem, modeCode);

	mCurrentChanCfg[channel].inputMode = mode;
}
void AiUsbTempAi::setRange(int channel, Range range)
{
	unsigned char adc = channel / 2;
	unsigned char adcChan = channel  % 2;
	unsigned char subItem = SUBITEM_CHAN_RANGE + adcChan;
	unsigned char rangeCode = getRangeCode(range);

	daqDev().sendCmd(CMD_SETITEM, adc, subItem, rangeCode);

	mCurrentChanCfg[channel].range = range;
}

unsigned char AiUsbTempAi::tcCode(TcType tcType) const
{
	unsigned char tcCode = 0;
	switch(tcType)
	{
	case TC_J:
		tcCode = 0;
		break;
	case TC_K:
		tcCode = 1;
		break;
	case TC_T:
		tcCode = 2;
		break;
	case TC_E:
		tcCode = 3;
		break;
	case TC_R:
		tcCode = 4;
		break;
	case TC_S:
		tcCode = 5;
		break;
	case TC_B:
		tcCode = 6;
		break;
	case TC_N:
		tcCode = 7;
		break;
	}

	return tcCode;
}

TcType AiUsbTempAi::tcType(unsigned char tcCode) const
{
	TcType tcType;

	switch(tcCode)
	{
	case 0:
		tcType = TC_J;
		break;
	case 1:
		tcType = TC_K;
		break;
	case 2:
		tcType = TC_T;
		break;
	case 3:
		tcType = TC_E;
		break;
	case 4:
		tcType = TC_R;
		break;
	case 5:
		tcType = TC_S;
		break;
	case 6:
		tcType = TC_B;
		break;
	case 7:
		tcType = TC_N;
		break;
	default:
		tcType = (TcType) 0;
		break;
	}

	return tcType;
}

unsigned char AiUsbTempAi::getRangeCode(Range range) const
{
	unsigned char code;

	switch(range)
	{
	case BIP10VOLTS:
		code = 2;
		break;
	case BIP5VOLTS:
		code = 3;
		break;
	case BIP2PT5VOLTS:
		code = 4;
		break;
	case BIP1PT25VOLTS:
		code = 5;
		break;
	default:
		code = 2;
		break;
	}

	return code;
}

AiChanType AiUsbTempAi::getCfg_ChanType(int channel) const
{
	AiChanType chanType;

	if(daqDev().getDeviceType() != DaqDeviceId::USB_TEMP_AI)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(channel < 4)
	{
		unsigned char adc = channel / 2;
		unsigned char subItem = SUBITEM_SENSOR_TYPE;

		unsigned char buf[4];

		daqDev().queryCmd(CMD_GETITEM, adc, subItem, buf);

		unsigned char sensorType = buf[0];

		switch(sensorType)
		{
		case 0:
			chanType = AI_RTD;
			break;
		case 1:
			chanType = AI_THERMISTOR;
			break;
		case 2:
			chanType = AI_TC;
			break;
		case 3:
			chanType = AI_SEMICONDUCTOR;
			break;
		default:
			chanType = AI_DISABLED;
			break;
		}
	}
	else
	{
		unsigned char adc = channel / 2;
		unsigned char adcChan = channel % 2;
		unsigned char subItem = SUBITEM_CHAN_MODE + adcChan;
		unsigned char modeCode = 0;

		daqDev().queryCmd(CMD_GETITEM, adc, subItem, &modeCode);

		if(modeCode == 0x02)
			chanType = AI_DISABLED;
		else
			chanType = AI_VOLTAGE;
	}

	return chanType;
}

SensorConnectionType AiUsbTempAi::getCfg_SensorConnectionType(int channel) const
{
	SensorConnectionType connectionType = (SensorConnectionType) 0;

	if(daqDev().getDeviceType() != DaqDeviceId::USB_TEMP_AI)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	AiChanType chanType = getCfg_ChanType(channel);

	if(chanType == AI_RTD || chanType == AI_THERMISTOR)
	{
		unsigned char adc = channel / 2;
		unsigned char subItem = SUBITEM_CONNECTION_TYPE;

		unsigned char buf[4];

		daqDev().queryCmd(CMD_GETITEM, adc, subItem, buf);

		unsigned char sensorType = buf[0];

		switch(sensorType)
		{
		case 0:
			connectionType = SCT_2_WIRE_1;
			break;
		case 1:
			connectionType = SCT_2_WIRE_2;
			break;
		case 2:
			connectionType = SCT_3_WIRE;
			break;
		case 3:
			connectionType = SCT_4_WIRE;
			break;
		}
	}

	return connectionType;
}

void AiUsbTempAi::getCfg_ChanCoefsStr(int channel, char* coefsStr, unsigned int* len) const
{
	if(daqDev().getDeviceType() != DaqDeviceId::USB_TEMP_AI)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(!coefsStr)
		throw UlException(ERR_BAD_BUFFER);

	AiChanType chanType = getCfg_ChanType(channel);

	int coefCount = 0;
	if(chanType ==  AI_RTD)
		coefCount = 4;
	else if (chanType ==  AI_THERMISTOR)
		coefCount = 3;
	else if (chanType ==  AI_SEMICONDUCTOR)
		coefCount = 2;

	if(coefCount)
	{
		std::ostringstream coefs;

		unsigned char adc = channel / 2;
		unsigned char adcChan = channel % 2;
		unsigned char subItem = SUBITEM_COEF0 + adcChan;
		float coef;

		for(int i = 0; i < coefCount; i++)
		{
			daqDev().queryCmd(CMD_GETITEM, adc, subItem, &coef);

			coefs << coef;

			if(i != (coefCount -1))
				coefs << ", ";

			subItem += 2;
		}

		if(*len > coefs.str().length())
		{
			strcpy(coefsStr, coefs.str().c_str());
			*len = coefs.str().length() + 1;
		}
		else
		{
			*len = coefs.str().length() + 1;
			throw UlException(ERR_BAD_BUFFER);
		}
	}
	else
		*len = 0;
}


void AiUsbTempAi::setCfg_ChanTcType(int channel, TcType tcType)
{
	if(channel < 0 || channel > 3)
		throw UlException(ERR_BAD_AI_CHAN);

	if(tcType < TC_J || tcType > TC_N)
		throw UlException(ERR_BAD_TC_TYPE);

	unsigned char adc = channel / 2;
	unsigned char adcChan = channel % 2;
	unsigned char subItem = SUBITEM_TC_TYPE + adcChan;
	unsigned char tcCodeVal = tcCode(tcType);

	daqDev().sendCmd(CMD_SETITEM, adc, subItem, tcCodeVal);
}

TcType AiUsbTempAi::getCfg_ChanTcType(int channel) const
{
	TcType tcTypeVal;
	if(channel < 0 || channel > 3)
		throw UlException(ERR_BAD_AI_CHAN);

	unsigned char adc = channel / 2;
	unsigned char adcChan = channel % 2;
	unsigned char subItem = SUBITEM_TC_TYPE + adcChan;

	unsigned char buf[4];

	daqDev().queryCmd(CMD_GETITEM, adc, subItem, buf);

	unsigned char tcCodeVal = buf[0];

	tcTypeVal = tcType(tcCodeVal);

	return tcTypeVal;
}

void AiUsbTempAi::check_AIn_Args(int channel, AiInputMode inputMode, Range range, AInFlag flags) const
{
	if(channel < 0 || channel > mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(!mAiInfo.isInputModeSupported(inputMode))
		throw UlException(ERR_BAD_INPUT_MODE);

	if(!mAiInfo.isRangeSupported(inputMode, range))
		throw UlException(ERR_BAD_RANGE);

	if(channel > 3 && range == BIPPT078VOLTS)
		throw UlException(ERR_BAD_RANGE);

	if(~mAiInfo.getAInFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

	if((int) mCustomScales.size() < mAiInfo.getNumChans())
		throw UlException(ERR_INTERNAL);
}

TempScale AiUsbTempAi::getTempScale(TempUnit unit)
{
	TempScale scale;

	switch(unit)
	{
	case TU_FAHRENHEIT:
		scale = TS_FAHRENHEIT;
		break;
	case TU_KELVIN:
		scale = TS_KELVIN;
		break;
	default:
		scale = TS_CELSIUS;
	}

	return scale;
}


void AiUsbTempAi::addSupportedRanges()
{
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP10VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP2PT5VOLTS);
	mAiInfo.addRange(AI_SINGLE_ENDED, BIP1PT25VOLTS);

	mAiInfo.addRange(AI_DIFFERENTIAL, BIP10VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP2PT5VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIP1PT25VOLTS);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT078VOLTS);
}


} /* namespace ul */
