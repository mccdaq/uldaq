/*
 * AiConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AICONFIG_H_
#define AICONFIG_H_

#include "interfaces/UlAiConfig.h"
#include "ul_internal.h"

namespace ul
{
class AiDevice;
class UL_LOCAL AiConfig: public UlAiConfig
{
public:
	virtual ~AiConfig();
	AiConfig(AiDevice& aiDevice);

	virtual void setChanType(int channel, AiChanType chanType);
	virtual AiChanType getChanType(int channel);

	virtual void setChanTcType(int channel, TcType tcType);
	virtual TcType getChanTcType(int channel);

	//virtual void setScanChanTempUnit(int channel, TempUnit unit);
	//virtual TempUnit getScanChanTempUnit(int channel);

	virtual void setScanTempUnit(TempUnit unit);
	virtual TempUnit getScanTempUnit();

	virtual void setAutoZeroMode(AutoZeroMode mode);
	virtual AutoZeroMode getAutoZeroMode();

	virtual void setAdcTimingMode(AdcTimingMode mode);
	virtual AdcTimingMode getAdcTimingMode();

	virtual void setChanIepeMode(int channel, IepeMode mode);
	virtual IepeMode getChanIepeMode(int channel);

	virtual void setChanCouplingMode(int channel, CouplingMode mode);
	virtual CouplingMode getChanCouplingMode(int channel);

	virtual void setChanSensorSensitivity(int channel, double sensitivity);
	virtual double getChanSensorSensitivity(int channel);

	virtual void setChanSlope(int channel, double slope);
	virtual double getChanSlope(int channel);

	virtual void setChanOffset(int channel, double offset);
	virtual double getChanOffset(int channel);

	virtual unsigned long long getCalDate(int calTableIndex); // returns number of seconds since unix epoch
	virtual void getCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen);

	virtual void getChanCoefsStr(int channel, char* coefs, unsigned int* maxStrLen);
	virtual SensorConnectionType getChanSensorConnectionType(int channel);

	virtual void setChanDataRate(int channel, double rate);
	virtual double getChanDataRate(int channel);

	virtual void setChanOpenTcDetectionMode(int channel, OtdMode mode);
	virtual OtdMode getChanOpenTcDetectionMode(int channel);

	virtual void setOpenTcDetectionMode(int dev, OtdMode mode);
	virtual OtdMode getOpenTcDetectionMode(int dev);

	virtual void setCalTableType(int dev, AiCalTableType calTableType);
	virtual AiCalTableType getCalTableType(int dev);

	virtual void setRejectFreqType(int dev, AiRejectFreqType rejectFreqType);
	virtual AiRejectFreqType getRejectFreqType(int dev);

	virtual unsigned long long getExpCalDate(int calTableIndex); // returns number of seconds since unix epoch
	virtual void getExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen);

private:
	AiDevice& mAiDevice;
};

} /* namespace ul */

#endif /* AICONFIG_H_ */
