/*
 * AiConfig.cpp
 *
 *     Author: Measurement Computing Corporation
 */
#include "AiDevice.h"
#include "AiConfig.h"

namespace ul
{

AiConfig::~AiConfig()
{
}

AiConfig::AiConfig(AiDevice& aiDevice) : mAiDevice(aiDevice)
{

}

void AiConfig::setChanType(int channel, AiChanType chanType)
{
	mAiDevice.setCfg_ChanType(channel, chanType);
}
AiChanType AiConfig::getChanType(int channel)
{
	return mAiDevice.getCfg_ChanType(channel);
}

void AiConfig::setChanTcType(int channel, TcType tcType)
{
	mAiDevice.setCfg_ChanTcType(channel, tcType);
}

TcType AiConfig::getChanTcType(int channel)
{
	return mAiDevice.getCfg_ChanTcType(channel);
}

/*
void AiConfig::setScanChanTempUnit(int channel, TempUnit unit)
{
	mAiDevice.setCfg_ScanChanTempUnit(channel, unit);
}

TempUnit AiConfig::getScanChanTempUnit(int channel)
{
	return mAiDevice.getCfg_ScanChanTempUnit(channel);
}*/

void AiConfig::setScanTempUnit(TempUnit unit)
{
	mAiDevice.setCfg_ScanTempUnit(unit);
}

TempUnit AiConfig::getScanTempUnit()
{
	return mAiDevice.getCfg_ScanTempUnit();
}

void AiConfig::setAutoZeroMode(AutoZeroMode mode)
{
	mAiDevice.setCfg_AutoZeroMode(mode);
}

AutoZeroMode AiConfig::getAutoZeroMode()
{
	return mAiDevice.getCfg_AutoZeroMode();
}

void AiConfig::setAdcTimingMode(AdcTimingMode mode)
{
	mAiDevice.setCfg_AdcTimingMode(mode);
}
AdcTimingMode AiConfig::getAdcTimingMode()
{
	return mAiDevice.getCfg_AdcTimingMode();
}

void AiConfig::setChanIepeMode(int channel, IepeMode mode)
{
	mAiDevice.setCfg_ChanIepeMode(channel, mode);
}

IepeMode AiConfig::getChanIepeMode(int channel)
{
	return mAiDevice.getCfg_ChanIepeMode(channel);
}

void AiConfig::setChanCouplingMode(int channel, CouplingMode mode)
{
	mAiDevice.setCfg_ChanCouplingMode(channel, mode);
}

CouplingMode AiConfig::getChanCouplingMode(int channel)
{
	return mAiDevice.getCfg_ChanCouplingMode(channel);
}

void AiConfig::setChanSensorSensitivity(int channel, double sensitivity)
{
	mAiDevice.setCfg_ChanSensorSensitivity(channel, sensitivity);
}

double AiConfig::getChanSensorSensitivity(int channel)
{
	return mAiDevice.getCfg_ChanSensorSensitivity(channel);
}

void AiConfig::AiConfig::setChanSlope(int channel, double slope)
{
	mAiDevice.setCfg_ChanSlope(channel, slope);
}

double AiConfig::getChanSlope(int channel)
{
	return mAiDevice.getCfg_ChanSlope(channel);
}

void AiConfig::setChanOffset(int channel, double offset)
{
	mAiDevice.setCfg_ChanOffset(channel, offset);
}

double AiConfig::getChanOffset(int channel)
{
	return mAiDevice.getCfg_ChanOffset(channel);
}

unsigned long long AiConfig::getCalDate(int calTableIndex)
{
	return mAiDevice.getCfg_CalDate(calTableIndex);
}

void AiConfig::getCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen)
{
	return mAiDevice.getCfg_CalDateStr(calTableIndex, calDate, maxStrLen);
}

void AiConfig::getChanCoefsStr(int channel, char* coefs, unsigned int* maxStrLen)
{
	return mAiDevice.getCfg_ChanCoefsStr(channel, coefs, maxStrLen);
}

SensorConnectionType AiConfig::getChanSensorConnectionType(int channel)
{
	return mAiDevice.getCfg_SensorConnectionType(channel);
}

void AiConfig::setChanDataRate(int channel, double rate)
{
	mAiDevice.setCfg_ChanDataRate(channel, rate);
}
double AiConfig::getChanDataRate(int channel)
{
	return mAiDevice.getCfg_ChanDataRate(channel);
}
void AiConfig::setChanOpenTcDetectionMode(int channel, OtdMode mode)
{
	mAiDevice.setCfg_ChanOpenTcDetectionMode(channel, mode);
}
OtdMode AiConfig::getChanOpenTcDetectionMode(int channel)
{
	return mAiDevice.getCfg_ChanOpenTcDetectionMode(channel);
}

void AiConfig::setOpenTcDetectionMode(int dev, OtdMode mode)
{
	mAiDevice.setCfg_OpenTcDetectionMode(dev, mode);
}
OtdMode AiConfig::getOpenTcDetectionMode(int dev)
{
	return mAiDevice.getCfg_OpenTcDetectionMode(dev);
}

void AiConfig::setCalTableType(int dev, AiCalTableType calTableType)
{
	mAiDevice.setCfg_CalTableType(dev, calTableType);
}

AiCalTableType AiConfig::getCalTableType(int dev)
{
	return mAiDevice.getCfg_CalTableType(dev);
}

void AiConfig::setRejectFreqType(int dev, AiRejectFreqType rejectFreqType)
{
	mAiDevice.setCfg_RejectFreqType(dev, rejectFreqType);
}

AiRejectFreqType AiConfig::getRejectFreqType(int dev)
{
	return mAiDevice.getCfg_RejectFreqType(dev);
}

unsigned long long AiConfig::getExpCalDate(int calTableIndex)
{
	return mAiDevice.getCfg_ExpCalDate(calTableIndex);
}

void AiConfig::getExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen)
{
	return mAiDevice.getCfg_ExpCalDateStr(calTableIndex, calDate, maxStrLen);
}


} /* namespace ul */
