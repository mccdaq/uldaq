/*
 * AiUsb2001tc.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AiUsb2001tc.h"
#include "../../utility/Nist.h"
#include "../../utility/UlLock.h"

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <math.h>

#define MAX_COUNT 			(1 << 20) - 1

namespace ul
{

AiUsb2001tc::AiUsb2001tc(const UsbDaqDevice& daqDevice) : AiUsbBase(daqDevice)
{
	mAiInfo.setNumChans(1);
	mAiInfo.setNumCjcChans(1);
	mAiInfo.hasTempChan(true);
	mAiInfo.setNumChansByMode(AI_DIFFERENTIAL, 1);
	mAiInfo.setResolution(20);
	mAiInfo.setTInFlags(TIN_FF_DEFAULT);
	mAiInfo.setTInArrayFlags(TINARRAY_FF_DEFAULT);

	mAiInfo.setChanTypes(AI_TC);
	mAiInfo.setChanTypes(0, 0, AI_TC);

	mAiInfo.addInputMode(AI_DIFFERENTIAL);
	mAiInfo.addRange(AI_DIFFERENTIAL, BIPPT078VOLTS);

	mTcType = (TcType) 0;

	initCustomScales();
}

AiUsb2001tc::~AiUsb2001tc()
{

}

void AiUsb2001tc::initialize()
{
	try
	{
		loadAdcCoefficients();

		mTcType = getCfg_ChanTcType(0);

		if(mTcType == 0) // if the stored TC type in EEPROM is invalid then set it to J type
		{
			setCfg_ChanTcType(0, TC_J);
		}

		setAdcRange(RANGE_73PT125_IDX);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}


void AiUsb2001tc::tIn(int channel, TempScale scale, TInFlag flags, double* data)
{
	pthread_mutex_t& devMutex = daqDev().getDeviceMutex();
	UlLock lock(devMutex);

	check_TIn_Args(channel, scale, flags);

	// coverity[sleep]
	waitUntilAdcReady();

	unsigned int maxCount = MAX_COUNT; //(1 << 20) - 1; 20 bit ADC

	unsigned char cmd = 0x80;
	double minTCVal = 0;
	double maxTCVal = 0;

	std::string msgCjc= "?AI{0}:CJC";

	float cjcTemp;
	double tempValue;

	daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msgCjc.c_str(), msgCjc.length(), 2000);
	daqDev().queryCmd(0x81, 0, 0, (unsigned char*) &cjcTemp, sizeof(cjcTemp), 2000);

	if(channel == CJC0)
	{
		tempValue = mEndian.le_ptr_to_cpu_f32((unsigned char*) &cjcTemp);

		tempValue = convertTempUnit(tempValue, (TempUnit)scale);
		*data = tempValue;
	}
	else
	{
		getTcRange(mTcType, &minTCVal, &maxTCVal);

		std::string msgAIn= "?AI{0}:VALUE";
		unsigned int rawData;

		daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msgAIn.c_str(), msgAIn.length(), 2000);
		daqDev().queryCmd(0x81, 0, 0, (unsigned char*) &rawData, sizeof(rawData), 2000);

		rawData = mEndian.le_ui32_to_cpu(rawData);

		if(rawData == maxCount && mTcType == TC_E)
		{
			rawData = 0;

			// coverity[sleep]
			setAdcRange(RANGE_146PT25_IDX);

			daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msgAIn.c_str(), msgAIn.length(), 2000);
			daqDev().queryCmd(0x81, 0, 0, (unsigned char*) &rawData, sizeof(rawData), 2000);

			rawData = mEndian.le_ui32_to_cpu(rawData);

			// coverity[sleep]
			setAdcRange(RANGE_73PT125_IDX);

			if(rawData != maxCount)
			{
				*data=-9999.0f;
				throw UlException(ERR_TEMP_OUT_OF_RANGE);
			}
		}

		if(rawData == maxCount)
		{
			*data=-9999.0f;
			throw UlException(ERR_OPEN_CONNECTION);
		}

		double calData = calibrateInputData(rawData);

		if(scale != TS_NOSCALE)
		{
			double scaledData = scaleData(calData);

			unsigned char tcType = mTcType - 1;  // zero based

			double cjc_volts = NISTCalcVoltage(tcType, cjcTemp);

			double tc_volts = scaledData * 1000;

			double tc_temp = NISTCalcTemp(tcType, tc_volts + cjc_volts);

			if(tc_temp < minTCVal || tc_temp > maxTCVal)
			{
				*data=-9999.0f;
				throw UlException(ERR_TEMP_OUT_OF_RANGE);
			}
			else
			{
				tempValue = convertTempUnit(tc_temp, (TempUnit)scale);
				*data = mCustomScales[channel].slope * tempValue + mCustomScales[channel].offset;
			}
		}
		else
		{
			*data = mCustomScales[channel].slope * calData + mCustomScales[channel].offset;
		}
	}
}

double AiUsb2001tc::calibrateInputData(unsigned int rawData) const
{
    double calData = rawData;

    if (mCalCoefs[0].slope != 0 && !isnan(mCalCoefs[0].slope) && !isnan(mCalCoefs[0].offset))
    {
        calData = rawData * mCalCoefs[0].slope;
        calData += mCalCoefs[0].offset;
    }

    return calData;
}

double AiUsb2001tc::scaleData(double rawData) const
{
	double scaledData = 0;

	double upperLimit = 0.073125;
	double lowerLimit = -0.073125;

	double scale = upperLimit - lowerLimit;
	double lsb = scale / (1 << 20);

	scaledData = lsb * rawData + lowerLimit;


	return scaledData;
}

void AiUsb2001tc::setCfg_ChanTcType(int channel, TcType tcType)
{
	pthread_mutex_t& devMutex = daqDev().getDeviceMutex();
	UlLock lock(devMutex);

	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	if(tcType < TC_J || tcType > TC_N)
		throw UlException(ERR_BAD_TC_TYPE);

	char tcTypeChar = '+';

	switch(tcType)
	{
	case TC_J:
		tcTypeChar = 'J';
		break;
	case TC_K:
		tcTypeChar = 'K';
		break;
	case TC_T:
		tcTypeChar = 'T';
		break;
	case TC_E:
		tcTypeChar = 'E';
		break;
	case TC_R:
		tcTypeChar = 'R';
		break;
	case TC_S:
		tcTypeChar = 'S';
		break;
	case TC_B:
		tcTypeChar = 'B';
		break;
	case TC_N:
		tcTypeChar = 'N';
		break;
	}

	if(tcTypeChar != '+')
	{
		unsigned char cmd = 0x80;

		std::ostringstream msg;
		msg << "AI{0}:SENSOR=TC/" <<  tcTypeChar;

		daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msg.str().c_str(), msg.str().length(), 2000);

		mTcType = tcType;
	}
}

TcType AiUsb2001tc::getCfg_ChanTcType(int channel) const
{
	pthread_mutex_t& devMutex = daqDev().getDeviceMutex();
	UlLock lock(devMutex);

	TcType tcTypeVal;
	if(channel < 0 || channel >= mAiInfo.getNumChans())
		throw UlException(ERR_BAD_AI_CHAN);

	char tcTypeChar;
	unsigned char cmd = 0x80;

	std::string msg= "?AI{0}:SENSOR";
	char reply[64];

	daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msg.c_str(), msg.length(), 2000);

	daqDev().queryCmd(cmd, 0, 0, (unsigned char*) &reply, sizeof(reply), 2000, false);

	tcTypeChar = reply[16];  // reply format is "AI{0}:SENSOR=TC/J"

	//std::cout << "TC Type: " << tcTypeChar << std::endl;

	switch(tcTypeChar)
	{
	case 'J':
	case 'j':
		tcTypeVal = TC_J;
		break;
	case 'K':
	case 'k':
		tcTypeVal = TC_K;
		break;
	case 'T':
	case 't':
		tcTypeVal = TC_T;
		break;
	case 'E':
	case 'e':
		tcTypeVal = TC_E;
		break;
	case 'R':
	case 'r':
		tcTypeVal = TC_R;
		break;
	case 'S':
	case 's':
		tcTypeVal = TC_S;
		break;
	case 'B':
	case 'b':
		tcTypeVal = TC_B;
		break;
	case 'N':
	case 'n':
		tcTypeVal = TC_N;
		break;
	default:
		tcTypeVal = (TcType) 0;
		break;

	}

	return tcTypeVal;
}

void AiUsb2001tc::getTcRange(TcType tcType, double* min, double* max) const
{
	switch(tcType)
	{
	case TC_J:
		*min = -210;
		*max = 1200;
		break;
	case TC_K:
		*min = -270;
		*max = 1372;
		break;
	case TC_T:
		*min = -270;
		*max = 400;
		break;
	case TC_E:
		*min = -270;
		*max = 1000;
		break;
	case TC_R:
		*min = -50;
		*max = 1768.1;
		break;
	case TC_S:
		*min = -50;
		*max = 1768.1;
		break;
	case TC_B:
		*min = 0;
		*max = 1820;
		break;
	case TC_N:
		*min = -270;
		*max = 1300;
		break;
	}
}

void AiUsb2001tc::setAdcRange(int rangeIndex) const
{
	std::string msg = "AI{0}:RANGE=BIP73.125E-3V";

	if(rangeIndex == RANGE_146PT25_IDX)
		msg = "AI{0}:RANGE=BIP146.25E-3V";

	unsigned char cmd = 0x80;

	daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msg.c_str(), msg.length(), 2000);

	waitUntilAdcReady();

}

void AiUsb2001tc::waitUntilAdcReady() const
 {
	unsigned char status;

	std::string msg = "?AI{0}:STATUS";
	char reply[64];

	unsigned char cmd = 0x80;

	int queryCount = 0;
	int errCode = 0;

	do
	{
		daqDev().sendCmd(cmd, 0, 0, (unsigned char*) msg.c_str(), msg.length(), 2000);

		status = ADC_BUSY;

		daqDev().queryCmd(cmd, 0, 0, (unsigned char*) &reply, sizeof(reply), 2000, false);

		if(strcmp(reply, "AI{0}:STATUS=BUSY") != 0)
			status = ADC_READY;

		usleep(100000);
		queryCount++;
	}
	while(status == ADC_BUSY && errCode == 0 && queryCount < 50);

 }

void AiUsb2001tc::loadAdcCoefficients()
{
	pthread_mutex_t& devMutex = daqDev().getDeviceMutex();
	UlLock lock(devMutex);

	mCalCoefs.clear();

	CalCoef calCoef;

	float reply;

	std::string msg = "?AI{0}:SLOPE";

	daqDev().sendCmd(0x80, 0, 0, (unsigned char*) msg.c_str(), msg.length(), 2000);
	daqDev().queryCmd(0x81, 0, 0, (unsigned char*) &reply, sizeof(reply), 2000);
	calCoef.slope = reply;

	msg = "?AI{0}:OFFSET";

	daqDev().sendCmd(0x80, 0, 0, (unsigned char*) msg.c_str(), msg.length(), 2000);
	daqDev().queryCmd(0x81, 0, 0, (unsigned char*) &reply, sizeof(reply), 2000);
	calCoef.offset = reply;

	mCalCoefs.push_back(calCoef);

	readCalDate();

	//std::cout << "slope: " << calCoef.slope << "  offset: " << calCoef.offset << std::endl;

}

void AiUsb2001tc::readCalDate()
{
	int calDateBuf[6];

	std::string msg = "?DEV:MFGCAL";
	char reply[64];

	daqDev().sendCmd(0x80, 0, 0, (unsigned char*) msg.c_str(), msg.length(), 2000);
	daqDev().queryCmd(0x80, 0, 0, (unsigned char*) &reply, sizeof(reply), 2000, false);

	//std::cout << "reply:" << reply << std::endl;

	// reply format DEV:MFGCAL=2014-8-12 12:41:01
	sscanf(reply, "DEV:MFGCAL=%d-%d-%d %d:%d:%d", &calDateBuf[0], &calDateBuf[1], &calDateBuf[2], &calDateBuf[3], &calDateBuf[4], &calDateBuf[5]);

	//std::cout << "reply1:" << calDateBuf[0] << calDateBuf[1] << calDateBuf[2] << calDateBuf[3] << calDateBuf[4] << calDateBuf[5] << std::endl;

	tm time;
	memset(&time, 0, sizeof(time));

	// the date is stored on the device in big-endian format by the test department
	time.tm_year =  calDateBuf[0] - 1900;
	time.tm_mon = calDateBuf[1] - 1;
	time.tm_mday = calDateBuf[2];
	time.tm_hour = calDateBuf[3];
	time.tm_min = calDateBuf[4];
	time.tm_sec = calDateBuf[5];
	time.tm_isdst = -1;

	time_t cal_date_sec = mktime(&time); // seconds since unix epoch

	if(cal_date_sec > 0) // mktime returns  -1 if cal date is invalid
		mCalDate = cal_date_sec;

	// convert seconds to string
	/*
	struct tm *timeinfo;
	timeinfo = localtime(&cal_date_sec);
	char b[100];
	strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
	std::cout << b << std::endl;*/
}

} /* namespace ul */
