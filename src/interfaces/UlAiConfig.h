/*
 * UlAiConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULAICONFIG_H_
#define INTERFACES_ULAICONFIG_H_

#include "../uldaq.h"

namespace ul
{

class UlAiConfig
{
public:
	virtual ~UlAiConfig() {};

	virtual void setChanType(int channel, AiChanType chanType) = 0;
	virtual AiChanType getChanType(int channel) = 0;

	virtual void setChanTcType(int channel, TcType tcType) = 0;
	virtual TcType getChanTcType(int channel) = 0;

	//virtual void setScanChanTempUnit(int channel, TempUnit unit) = 0;
	//virtual TempUnit getScanChanTempUnit(int channel) = 0;
	virtual void setScanTempUnit(TempUnit unit) = 0;
	virtual TempUnit getScanTempUnit() = 0;

	virtual void setAutoZeroMode(AutoZeroMode mode) = 0;
	virtual AutoZeroMode getAutoZeroMode() = 0;

	virtual void setAdcTimingMode(AdcTimingMode mode) = 0;
	virtual AdcTimingMode getAdcTimingMode() = 0;

	virtual void setChanIepeMode(int channel, IepeMode mode) = 0;
	virtual IepeMode getChanIepeMode(int channel) = 0;

	virtual void setChanCouplingMode(int channel, CouplingMode mode) = 0;
	virtual CouplingMode getChanCouplingMode(int channel) = 0;

	virtual void setChanSensorSensitivity(int channel, double sensitivity) = 0;
	virtual double getChanSensorSensitivity(int channel) = 0;

	virtual void setChanSlope(int channel, double slope) = 0;
	virtual double getChanSlope(int channel) = 0;

	virtual void setChanOffset(int channel, double offset) = 0;
	virtual double getChanOffset(int channel) = 0;

	virtual unsigned long long getCalDate(int calTableIndex) = 0; // returns number of seconds since unix epoch
	virtual void getCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen) = 0;

	virtual void getChanCoefsStr(int channel, char* coefs, unsigned int* maxStrLen) = 0;

	virtual SensorConnectionType getChanSensorConnectionType(int channel) = 0;

	virtual void setChanDataRate(int channel, double rate) = 0;
	virtual double getChanDataRate(int channel) = 0;

	virtual void setChanOpenTcDetectionMode(int channel, OtdMode mode) = 0;
	virtual OtdMode getChanOpenTcDetectionMode(int channel) = 0;

	virtual void setOpenTcDetectionMode(int dev, OtdMode mode) = 0;
	virtual OtdMode getOpenTcDetectionMode(int dev) = 0;

	virtual void setCalTableType(int dev, AiCalTableType calTableType) = 0;
	virtual AiCalTableType getCalTableType(int dev) = 0;

	virtual void setRejectFreqType(int dev, AiRejectFreqType rejectFreqType) = 0;
	virtual AiRejectFreqType getRejectFreqType(int dev) = 0;

	virtual unsigned long long getExpCalDate(int calTableIndex) = 0; // returns number of seconds since unix epoch
	virtual void getExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULAICONFIG_H_ */
