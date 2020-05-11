/*
 * AiETc.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiETc.h"

namespace ul
{
AiETc::AiETc(const NetDaqDevice& daqDevice) : AiNetBase(daqDevice)
{
	mAiInfo.setNumChans(8);
	mAiInfo.setNumCjcChans(2);
	mAiInfo.hasTempChan(true);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 8);
	mAiInfo.setResolution(24);
	mAiInfo.setTInFlags(TIN_FF_WAIT_FOR_NEW_DATA);
	mAiInfo.setTInArrayFlags(TINARRAY_FF_WAIT_FOR_NEW_DATA);

	mAiInfo.setChanTypes(AI_TC);
	mAiInfo.setChanTypes(0, 7, AI_TC);

	mAiInfo.addInputMode(AI_DIFFERENTIAL);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT078VOLTS);

	initCustomScales();
}

AiETc::~AiETc()
{
}

void AiETc::initialize()
{
	try
	{
		// All of the channels are disabled when this device is shipped from factory. In order for examples to work by default, ul for linux
		// enables any disabled channels when it loads. users can disable any desire channel by calling the setConfig function in their application
		enableAllChannels();

		setMeasureMode(NORMAL_MEASURE_MODE);

		readCalDate();

	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AiETc::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	check_TIn_Args(channel, scale, flags);

	if(channel & 0x80) // cjc channel
	{
		float cjcVals[2];
		memset(cjcVals, 0 , 2 * sizeof(float));

		daqDev().queryCmd(CMD_CJC,  NULL, 0, (unsigned char*)&cjcVals, sizeof(cjcVals));

		int index = channel - 0x80;

		*data = cjcVals[index];
	}
	else
	{
		tInArray(channel, channel, scale, (TInArrayFlag) flags, data);
	}
}
void AiETc::tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[])
{
	check_TInArray_Args(lowChan, highChan, scale, flags, data);

	int chanCount = highChan - lowChan + 1;
	float fData[8];
	memset(fData, 0 , 8 * sizeof(float));

	unsigned char units = 0;
	unsigned char waitForNewData = 0;

	bool openConnection = false;

	if(scale == TS_VOLTS)
		units = 1;
	else if (scale == TS_NOSCALE)
		units = 2;

	if(flags & TINARRAY_FF_WAIT_FOR_NEW_DATA)
		waitForNewData = 1;

	unsigned char chanMask = 0;

	for(int ch = lowChan; ch <= highChan; ch++)
		chanMask |= 1 << ch;

	unsigned char params[3] = {chanMask, units, waitForNewData};

	daqDev().queryCmd(CMD_TIN,  params, sizeof(params), (unsigned char*)fData, sizeof(float) * chanCount);

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
		case -9999: // channel disabled
			data[i] = -11111.0f;
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


void AiETc::enableAllChannels()
{
	bool chanEnabled = false;
	unsigned char chanTcTypes[8];

	daqDev().queryCmd(CMD_TIN_CONFIG_R, NULL, 0, chanTcTypes, sizeof(chanTcTypes));

	for(int ch = 0; ch < mAiInfo.getNumChans(); ch++)
	{
		if(chanTcTypes[ch] == 0)
		{
			chanTcTypes[ch] = TC_J;
			chanEnabled = true;
		}
	}

	if(chanEnabled)
	{
		daqDev().queryCmd(CMD_TIN_CONFIG_W, chanTcTypes, sizeof(chanTcTypes));
	}

}

void AiETc::setMeasureMode(int mode)
{
	if(mode == NORMAL_MEASURE_MODE || mode == TEST_MEASURE_MODE)
	{
		unsigned char measureMode = mode;
		daqDev().queryCmd(CMD_MEASURE_MODE_W, &measureMode, sizeof(measureMode));
	}
}

void AiETc::readCalDate()
{
	unsigned char calDateBuf[6];

	if(getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().queryCmd(CMD_FACTORY_CAL_DATE_R,  NULL, 0, (unsigned char*)&calDateBuf, sizeof(calDateBuf));

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

		/// read field cal date

		bytesReceived = daqDev().queryCmd(CMD_FIELD_CAL_DATE_R,  NULL, 0, (unsigned char*)&calDateBuf, sizeof(calDateBuf));

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
					mFieldCalDate = cal_date_sec;

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

//////////////////////          Configuration functions          /////////////////////////////////

void AiETc::setCfg_ChanType(int channel, AiChanType chanType)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(!(chanType & (AI_TC | AI_DISABLED)))
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	unsigned char chanTcTypes[8];

	daqDev().queryCmd(CMD_TIN_CONFIG_R, NULL, 0, chanTcTypes, sizeof(chanTcTypes));

	if(chanType == AI_DISABLED && chanTcTypes[channel] != 0)
	{
		chanTcTypes[channel] = 0;
	}
	else if(chanType == AI_TC && chanTcTypes[channel] == 0)
	{
		chanTcTypes[channel] = TC_J; // defaults to J
	}
	else
		return;

	daqDev().queryCmd(CMD_TIN_CONFIG_W, chanTcTypes, sizeof(chanTcTypes));
}

AiChanType AiETc::getCfg_ChanType(int channel) const
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	unsigned char chanTcTypes[8];

	daqDev().queryCmd(CMD_TIN_CONFIG_R, NULL, 0, chanTcTypes, sizeof(chanTcTypes));

	AiChanType chanType = chanTcTypes[channel] == 0 ? AI_DISABLED : AI_TC;

	return chanType;
}

void AiETc::setCfg_ChanTcType(int channel, TcType tcType)
{
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(tcType < TC_J || tcType > TC_N)
		throw UlException(ERR_BAD_TC_TYPE);

	unsigned char chanTcTypes[8];

	daqDev().queryCmd(CMD_TIN_CONFIG_R, NULL, 0, chanTcTypes, sizeof(chanTcTypes));

	chanTcTypes[channel] = tcType;

	daqDev().queryCmd(CMD_TIN_CONFIG_W, chanTcTypes, sizeof(chanTcTypes));
}

TcType AiETc::getCfg_ChanTcType(int channel) const
{
	TcType tcTypeVal;

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	unsigned char chanTcTypes[8];

	daqDev().queryCmd(CMD_TIN_CONFIG_R, NULL, 0, chanTcTypes, sizeof(chanTcTypes));

	// if channel is disabled return J type as current TC type
	tcTypeVal = chanTcTypes[channel] == 0 ? TC_J : (TcType) chanTcTypes[channel];

	return tcTypeVal;
}

void  AiETc::setCfg_OpenTcDetectionMode(int dev, OtdMode mode)
{
	TMEASURE_CFG config;

	daqDev().queryCmd(CMD_MEASURE_CONFIG_R, NULL, 0, (unsigned char*)&config, sizeof(config));

	if(mode == OTD_DISABLED)
		config.otd = 1;
	else
		config.otd = 0;

	daqDev().queryCmd(CMD_MEASURE_CONFIG_W, (unsigned char*)&config, sizeof(config));
}
OtdMode  AiETc::getCfg_OpenTcDetectionMode(int dev) const
{
	OtdMode mode = OTD_ENABLED;

	TMEASURE_CFG config;

	daqDev().queryCmd(CMD_MEASURE_CONFIG_R, NULL, 0, (unsigned char*)&config, sizeof(config));

	if(config.otd == 1)
		mode = OTD_DISABLED;

	return mode;
}

void AiETc::setCfg_CalTableType(int dev, AiCalTableType calTableType)
{
	TMEASURE_CFG config;

	daqDev().queryCmd(CMD_MEASURE_CONFIG_R, NULL, 0, (unsigned char*)&config, sizeof(config));

	if(calTableType == AI_CTT_FIELD)
		config.cal = 1;
	else
		config.otd = 0;

	daqDev().queryCmd(CMD_MEASURE_CONFIG_W, (unsigned char*)&config, sizeof(config));
}

AiCalTableType AiETc::getCfg_CalTableType(int dev) const
{
	AiCalTableType type = AI_CTT_FACTORY;
	TMEASURE_CFG config;

	daqDev().queryCmd(CMD_MEASURE_CONFIG_R, NULL, 0, (unsigned char*)&config, sizeof(config));

	if(config.cal == 1)
		type = AI_CTT_FIELD;

	return type;
}

} /* namespace ul */
