/*
 * AiUsbTc32.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiUsbTc32.h"

namespace ul
{
AiUsbTc32::AiUsbTc32(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	mAiInfo.setNumChans(64);
	mAiInfo.setNumCjcChans(64);
	mAiInfo.hasTempChan(true);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 64); // including exp channels
	mAiInfo.setResolution(24);
	mAiInfo.setTInFlags(TIN_FF_WAIT_FOR_NEW_DATA);
	mAiInfo.setTInArrayFlags(TINARRAY_FF_WAIT_FOR_NEW_DATA);

	mAiInfo.setChanTypes(AI_TC);
	mAiInfo.setChanTypes(0, 63, AI_TC);

	mAiInfo.addInputMode(AI_DIFFERENTIAL);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT078VOLTS);

	mActualChanCount = mAiInfo.getNumChans();
	mActualCjcCount = mAiInfo.getNumCjcChans();

	initCustomScales();

	mExpFactoryCalDate = 0;
}

AiUsbTc32::~AiUsbTc32()
{
}

void AiUsbTc32::initialize()
{
	mActualChanCount = mAiInfo.getNumChans();
	mActualCjcCount = mAiInfo.getNumCjcChans();

	if(!daqDev().hasExp())
	{
		mActualChanCount /= 2;
		mActualCjcCount	/= 2;
	}

	try
	{
		// All of the channels are disabled when this device is shipped from factory. In order for examples to work by default, ul for linux
		// enables any disabled channels when it loads. users can disable any desire channel by calling the setConfig function in their application
		enableAllChannels();

		setMeasureMode(NORMAL_MEASURE_MODE);

		readCalDate();

		AiRejectFreqType baseRFT = getCfg_RejectFreqType(0);

		if(baseRFT == AI_RFT_50HZ)
			mFieldCalDate = mTc32FieldCalDates[1];
		else
			mFieldCalDate = mTc32FieldCalDates[0];
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AiUsbTc32::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	tInArray(channel, channel, scale, (TInArrayFlag) flags, data);
}
void AiUsbTc32::tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[])
{
	check_TInArray_Args(lowChan, highChan, scale, flags, data);

	bool cjcChan = lowChan & 0x80 ? true : false;
	unsigned char cmd = CMD_TIN_MULTI;

	if(cjcChan)
	{
		cmd = CMD_CJC_MULTI;
		lowChan = lowChan - 0x80;
		highChan = highChan - 0x80;
	}

	int chanCount = highChan - lowChan + 1;

	float fData[64];
	memset(fData, 0 , 64 * sizeof(float));

	unsigned char units = 0;
	unsigned char waitForNewData = 0;

	bool openConnection = false;
	bool cmrExceeded = false;

	if(scale == TS_VOLTS)
		units = 1;
	else if (scale == TS_NOSCALE)
		units = 2;

	if(flags & TINARRAY_FF_WAIT_FOR_NEW_DATA)
		waitForNewData = 1;

	unsigned short allChannels = 1;
	unsigned short param = units | (waitForNewData << 8);

	daqDev().queryCmd(cmd,  allChannels, param, (unsigned char*)fData, sizeof(fData));

	float tempValue = 0;
	int channel = 0;

	for(int i = 0; i < chanCount; i++)
	{
		channel = lowChan + i;
		tempValue = mEndian.le_ptr_to_cpu_f32((unsigned char*) &fData[channel]);

		if(cjcChan)
		{
			data[i] = convertTempUnit(tempValue, (TempUnit)scale);
		}
		else
		{
			switch((int)(tempValue))
			{
				case -7777:                         // Input voltage outside valid common-mode voltage range
					data[i] = -7777.0f;
					cmrExceeded = true;
					break;
				case -8888:            				// if the temp Value is -8888.0  TC open connection is detected
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
	}

	if(openConnection)
		throw UlException(ERR_OPEN_CONNECTION);

	if(cmrExceeded)
		throw UlException(ERR_CMR_EXCEEDED);
}

void AiUsbTc32::check_TInArray_Args(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]) const
{
	if(lowChan < 0 || highChan < 0 || lowChan > highChan )
		throw UlException(ERR_BAD_AI_CHAN);

	int chanCount = mActualChanCount;

	if(lowChan & 0x80)
	{
		lowChan = lowChan - 0x80;
		highChan = highChan - 0x80;

		chanCount = mActualCjcCount;
	}

	if(lowChan >= chanCount || highChan >= chanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(~mAiInfo.getTInArrayFlags() & flags)
		throw UlException(ERR_BAD_FLAG);

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void AiUsbTc32::enableAllChannels()
{
	bool chanEnabled = false;
	unsigned char chanTcTypes[64];

	daqDev().queryCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));

	for(int ch = 0; ch < mActualChanCount; ch++)
	{
		if(chanTcTypes[ch] == 0)
		{
			chanTcTypes[ch] = TC_J;
			chanEnabled = true;
		}
	}

	if(chanEnabled)
	{
		daqDev().sendCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));
	}
}

void AiUsbTc32::setMeasureMode(int mode)
{
	if(mode == NORMAL_MEASURE_MODE || mode == TEST_MEASURE_MODE)
	{
		unsigned char measureMode[2];
		measureMode[0] = mode;
		measureMode[1] = mode;

		daqDev().sendCmd(CMD_MEASURE_MODE, 0, 0, measureMode, sizeof(measureMode));
	}
}

void AiUsbTc32::readCalDate()
{
	unsigned char calDateBuf[12];

	if(getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().queryCmd(CMD_FACTORY_CAL_DATE,  0, 0, (unsigned char*)&calDateBuf, sizeof(calDateBuf));

		if(bytesReceived == sizeof(calDateBuf))
		{
			tm time;
			int offset = 0;
			mExpFactoryCalDate = 0;

			// there are two factory cal dates
			for(int i = 0; i < 2; i++)
			{
				memset(&time, 0, sizeof(time));
				offset = i * 6;

				time.tm_year = calDateBuf[0 + offset] + 100;
				time.tm_mon = calDateBuf[1 + offset] - 1;
				time.tm_mday = calDateBuf[2 + offset];
				time.tm_hour = calDateBuf[3 + offset];
				time.tm_min = calDateBuf[4 + offset];
				time.tm_sec = calDateBuf[5 + offset];
				time.tm_isdst = -1;

				// make sure the date is valid, mktime does not validate the range
				if(time.tm_mon <= 11 && time.tm_mday <= 31 && time.tm_hour <= 23 && time.tm_min <= 59 && time.tm_sec <= 60)
				{
					time_t cal_date_sec = mktime(&time); // seconds since unix epoch

					if(cal_date_sec != -1) // mktime returns  -1 if cal date is invalid
					{
						if(i == 0)
							mCalDate = cal_date_sec;
						else
							mExpFactoryCalDate = cal_date_sec;
					}

					// convert seconds to string

					/*struct tm *timeinfo;
					timeinfo = localtime(&cal_date_sec);
					char b[100];
					strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
					std::cout << b << std::endl;*/
				}
			}
		}

		/// read field cal date

		unsigned char fieldCalDateBuf[24];

		bytesReceived = daqDev().queryCmd(CMD_FIELD_CAL_DATE,  0, 0, (unsigned char*)&fieldCalDateBuf, sizeof(fieldCalDateBuf));

		if(bytesReceived == sizeof(fieldCalDateBuf))
		{
			tm time;
			int offset = 0;
			memset(mTc32FieldCalDates, 0, sizeof(mTc32FieldCalDates));

			// there are four field cal dates
			for(int i = 0; i < 4; i++)
			{
				memset(&time, 0, sizeof(time));
				offset = i * 6;

				time.tm_year = fieldCalDateBuf[0 + offset] + 100;
				time.tm_mon = fieldCalDateBuf[1 + offset] - 1;
				time.tm_mday = fieldCalDateBuf[2 + offset];
				time.tm_hour = fieldCalDateBuf[3 + offset];
				time.tm_min = fieldCalDateBuf[4 + offset];
				time.tm_sec = fieldCalDateBuf[5 + offset];
				time.tm_isdst = -1;

				// make sure the date is valid, mktime does not validate the range
				if(time.tm_mon <= 11 && time.tm_mday <= 31 && time.tm_hour <= 23 && time.tm_min <= 59 && time.tm_sec <= 60)
				{
					time_t cal_date_sec = mktime(&time); // seconds since unix epoch

					if(cal_date_sec != -1) // mktime returns  -1 if cal date is invalid
						mTc32FieldCalDates[i] = cal_date_sec;

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
}

//////////////////////          Configuration functions          /////////////////////////////////

void AiUsbTc32::setCfg_ChanType(int channel, AiChanType chanType)
{
	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(!(chanType & (AI_TC | AI_DISABLED)))
		throw UlException(ERR_BAD_AI_CHAN_TYPE);

	unsigned char chanTcTypes[64];

	daqDev().queryCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));

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

	daqDev().sendCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));
}

AiChanType AiUsbTc32::getCfg_ChanType(int channel) const
{
	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	unsigned char chanTcTypes[64];

	daqDev().queryCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));

	AiChanType chanType = chanTcTypes[channel] == 0 ? AI_DISABLED : AI_TC;

	return chanType;
}

void AiUsbTc32::setCfg_ChanTcType(int channel, TcType tcType)
{
	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	if(tcType < TC_J || tcType > TC_N)
		throw UlException(ERR_BAD_TC_TYPE);

	unsigned char chanTcTypes[64];

	daqDev().queryCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));

	chanTcTypes[channel] = tcType;

	daqDev().sendCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));
}

TcType AiUsbTc32::getCfg_ChanTcType(int channel) const
{
	TcType tcTypeVal;

	if(channel < 0 || channel >= mActualChanCount)
		throw UlException(ERR_BAD_AI_CHAN);

	unsigned char chanTcTypes[64];

	daqDev().queryCmd(CMD_TIN_CONFIG, 0, 0, chanTcTypes, sizeof(chanTcTypes));

	// if channel is disabled return J type as current TC type
	tcTypeVal = chanTcTypes[channel] == 0 ? TC_J : (TcType) chanTcTypes[channel];

	return tcTypeVal;
}

void  AiUsbTc32::setCfg_OpenTcDetectionMode(int dev, OtdMode mode)
{
	if(dev >= 0 && dev < 2)
	{
		TMEASURE_CFG config[2];

		daqDev().queryCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));

		if(mode == OTD_DISABLED)
			config[dev].otd = 1;
		else
			config[dev].otd = 0;

		daqDev().sendCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));
	}
}
OtdMode  AiUsbTc32::getCfg_OpenTcDetectionMode(int dev) const
{
	OtdMode mode = OTD_ENABLED;

	if(dev >= 0 && dev < 2)
	{
		TMEASURE_CFG config[2];

		daqDev().queryCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));

		if(config[dev].otd == 1)
			mode = OTD_DISABLED;
	}

	return mode;
}

void AiUsbTc32::setCfg_CalTableType(int dev, AiCalTableType calTableType)
{
	if(dev >= 0 && dev < 2)
	{
		TMEASURE_CFG config[2];

		daqDev().queryCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));

		if(calTableType == AI_CTT_FIELD)
			config[dev].cal = 1;
		else
			config[dev].cal = 0;

		daqDev().sendCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));
	}
}

AiCalTableType AiUsbTc32::getCfg_CalTableType(int dev) const
{
	AiCalTableType type = AI_CTT_FACTORY;

	if(dev >= 0 && dev < 2)
	{
		TMEASURE_CFG config[2];

		daqDev().queryCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));

		if(config[dev].cal == 1)
			type = AI_CTT_FIELD;
	}

	return type;
}

void AiUsbTc32::setCfg_RejectFreqType(int dev, AiRejectFreqType rejectFreqType)
{
	if(dev >= 0 && dev < 2)
	{
		TMEASURE_CFG config[2];

		daqDev().queryCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));

		if(rejectFreqType == AI_RFT_50HZ)
			config[dev].filter = 1;
		else
			config[dev].filter = 0;

		daqDev().sendCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));
	}
}
AiRejectFreqType AiUsbTc32::getCfg_RejectFreqType(int dev) const
{
	AiRejectFreqType type = AI_RFT_60HZ;

	if(dev >= 0 && dev < 2)
	{
		TMEASURE_CFG config[2];

		daqDev().queryCmd(CMD_MEASURE_CONFIG, 0, 0, (unsigned char*)&config, sizeof(config));

		if(config[dev].filter == 1)
			type = AI_RFT_50HZ;
	}

	return type;
}

unsigned long long AiUsbTc32::getCfg_ExpCalDate(int calTableIndex)
{
	mDaqDevice.checkConnection();

	unsigned long long expCalDate = 0;

	if(daqDev().hasExp())
	{
		if(calTableIndex == 0)
		{
			expCalDate = mExpFactoryCalDate;
		}
		else
		{
			AiRejectFreqType expRFT = getCfg_RejectFreqType(1);

			if(expRFT == AI_RFT_50HZ)
				expCalDate = mTc32FieldCalDates[3];
			else
				expCalDate = mTc32FieldCalDates[2];
		}
	}

	return expCalDate;
}

void AiUsbTc32::getCfg_ExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen)
{
	mDaqDevice.checkConnection();

	long int calDateSec = getCfg_ExpCalDate(calTableIndex);

	// convert seconds to string
	struct tm *timeinfo;
	timeinfo = localtime(&calDateSec);
	char calDateStr[128];
	strftime(calDateStr, 128, "%c", timeinfo);

	unsigned int len = strlen(calDateStr) + 1;

	if(len <= *maxStrLen)
	{
		memcpy(calDate, calDateStr, len);
		*maxStrLen = len;
	}
	else
	{
		*maxStrLen = len;

		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}


} /* namespace ul */
