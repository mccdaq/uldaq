/*
 * AiUsbTemp.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsbTemp.h"

#include <iostream>
#include <sstream>

namespace ul
{

AiUsbTemp::AiUsbTemp(const HidDaqDevice& daqDevice) : AiHidBase(daqDevice)
{
	mAiInfo.setNumChans(8);
	mAiInfo.setNumCjcChans(2);
	mAiInfo.hasTempChan(true);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 8);
	mAiInfo.setResolution(24);
	mAiInfo.setTInFlags(TIN_FF_DEFAULT);
	mAiInfo.setTInArrayFlags(TINARRAY_FF_DEFAULT);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_TEMP)
	{
		mAiInfo.setChanTypes(AI_TC | AI_RTD | AI_THERMISTOR | AI_SEMICONDUCTOR);
		mAiInfo.setChanTypes(0, 7, AI_TC | AI_RTD | AI_THERMISTOR | AI_SEMICONDUCTOR);
	}
	else
	{
		mAiInfo.setChanTypes(AI_TC);
		mAiInfo.setChanTypes(0, 7, AI_TC);
	}

	mAiInfo.addInputMode(AI_DIFFERENTIAL);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT078VOLTS);

	mAiInfo.setCalDateAddr(0xF0);

	initCustomScales();
}

AiUsbTemp::~AiUsbTemp()
{

}

void AiUsbTemp::initialize()
{
	try
	{
		readCalDate();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AiUsbTemp::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	check_TIn_Args(channel, scale, flags);

	float tempValue = 0;

	unsigned char chan = channel;
	unsigned char units = 0;

	if(scale == TS_VOLTS || scale == TS_NOSCALE)
		units = 1;

	float fData;

	daqDev().queryCmd(CMD_TIN, chan, units, &fData);

	tempValue = mEndian.le_ptr_to_cpu_f32((unsigned char*) &fData);

	switch((int)(tempValue))
	{
	case -8888:            // if the temp Value is -8888.0  TC open connection is detected
		*data=-9999.0f;
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
void AiUsbTemp::tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[])
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
	memset(fData, 0 , 8 * sizeof(float));

	daqDev().queryCmd(CMD_TINSCAN, startChan, endChan, units, (unsigned char*) fData, chanCount * sizeof(float));

	float tempValue = 0;
	int channel = 0;

	for(int i = 0; i < chanCount; i++)
	{
		tempValue = mEndian.le_ptr_to_cpu_f32((unsigned char*) &fData[i]);
		channel = lowChan + i;

		switch((int)(tempValue))
		{
		case -8888:            // if the temp Value is -8888.0  TC open connection is detected
			data[i] = -9999.0f;
			openConnection = true;
		  break;
		case -9000:            // if the temp Value is -9000.0  device is not ready yet
			throw UlException(ERR_DEV_NOT_READY);
			break;
		case -9999:
		  break;
		default:
			data[i] = convertTempUnit(tempValue, (TempUnit)scale);
			data[i] = mCustomScales[channel].slope * data[i] + mCustomScales[channel].offset;
		  break;
		}
	}

	if(openConnection)
		throw UlException(ERR_OPEN_CONNECTION);

}

unsigned char AiUsbTemp::tcCode(TcType tcType) const
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

TcType AiUsbTemp::tcType(unsigned char tcCode) const
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

AiChanType AiUsbTemp::getCfg_ChanType(int channel) const
{
	AiChanType chanType;

	if(daqDev().getDeviceType() != DaqDeviceId::USB_TEMP)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	unsigned char adc = channel / 2;
	unsigned char subItem = SUBITEM_SENSOR_TYPE;

	unsigned char buf[4];

	daqDev().queryCmd(CMD_GETITEM, adc, subItem, buf);

	unsigned char sensorType = buf[0];

	switch(sensorType)
	{
	case ST_RTD:
		chanType = AI_RTD;
		break;
	case ST_THERMISTOR:
		chanType = AI_THERMISTOR;
		break;
	case ST_THERMOCOUPLE:
		chanType = AI_TC;
		break;
	case ST_SEMICONDUCTOR:
		chanType = AI_SEMICONDUCTOR;
		break;
	default:
		chanType = AI_DISABLED;
		break;
	}

	return chanType;
}

SensorConnectionType AiUsbTemp::getCfg_SensorConnectionType(int channel) const
{
	SensorConnectionType connectionType = (SensorConnectionType) 0;

	if(daqDev().getDeviceType() != DaqDeviceId::USB_TEMP)
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

void AiUsbTemp::getCfg_ChanCoefsStr(int channel, char* coefsStr, unsigned int* len) const
{
	if(daqDev().getDeviceType() != DaqDeviceId::USB_TEMP)
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

void AiUsbTemp::setCfg_ChanTcType(int channel, TcType tcType)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(tcType < TC_J || tcType > TC_N)
		throw UlException(ERR_BAD_TC_TYPE);

	unsigned char adc = channel / 2;
	unsigned char adcChan = channel % 2;
	unsigned char subItem = SUBITEM_TC_TYPE + adcChan;
	unsigned char tcCodeVal = tcCode(tcType);

	daqDev().sendCmd(CMD_SETITEM, adc, subItem, tcCodeVal);
}

TcType AiUsbTemp::getCfg_ChanTcType(int channel) const
{
	TcType tcTypeVal;
	if(channel < 0 || channel >= mAiInfo.getNumChans())
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

} /* namespace ul */
