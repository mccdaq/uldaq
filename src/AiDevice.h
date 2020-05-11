/*
 * AiDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AIDEVICE_H_
#define AIDEVICE_H_

#include "ul_internal.h"
#include "IoDevice.h"
#include "AiInfo.h"
#include "AiConfig.h"
#include <vector>

namespace ul
{
class UL_LOCAL AiDevice: public IoDevice, public UlAiDevice
{
public:
	virtual ~AiDevice();
	AiDevice(const DaqDevice& daqDevice);

	virtual const UlAiInfo& getAiInfo() { return mAiInfo;}
	virtual UlAiConfig& getAiConfig() { return *mAiConfig;}

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);
	virtual void aInLoadQueue(AiQueueElement queue[], unsigned int numElements);
	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	virtual void tIn(int channel, TempScale scale, TInFlag flags, double* data);
	virtual void tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]);

	double convertTempUnit(double tempC, TempUnit unit);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_ChanType(int channel, AiChanType chanType);
	virtual AiChanType getCfg_ChanType(int channel) const;

	virtual void setCfg_ChanTcType(int channel, TcType tcType);
	virtual TcType getCfg_ChanTcType(int channel) const;

	virtual void setCfg_ScanTempUnit(TempUnit unit);
	virtual TempUnit getCfg_ScanTempUnit() const;

	//virtual void setCfg_ScanChanTempUnit(int channel, TempUnit unit);
	//virtual TempUnit getCfg_ScanChanTempUnit(int channel) const;

	virtual void setCfg_AutoZeroMode(AutoZeroMode mode);
	virtual AutoZeroMode getCfg_AutoZeroMode() const;

	virtual void setCfg_AdcTimingMode(AdcTimingMode mode);
	virtual AdcTimingMode getCfg_AdcTimingMode();

	virtual void setCfg_ChanIepeMode(int channel, IepeMode mdoe);
	virtual IepeMode getCfg_ChanIepeMode(int channel);

	virtual void setCfg_ChanCouplingMode(int channel, CouplingMode mode);
	virtual CouplingMode getCfg_ChanCouplingMode(int channel);

	virtual void setCfg_ChanSensorSensitivity(int channel, double sensitivity);
	virtual double getCfg_ChanSensorSensitivity(int channel);

	virtual void setCfg_ChanSlope(int channel, double slope);
	virtual double getCfg_ChanSlope(int channel);
	virtual void setCfg_ChanOffset(int channel, double offset);
	virtual double getCfg_ChanOffset(int channel);

	virtual unsigned long long getCfg_CalDate(int calTableIndex);
	virtual void getCfg_CalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen);

	virtual SensorConnectionType getCfg_SensorConnectionType(int channel) const;
	virtual void getCfg_ChanCoefsStr(int channel, char* coefsStr, unsigned int* len) const;

	virtual void setCfg_ChanDataRate(int channel, double rate);
	virtual double getCfg_ChanDataRate(int channel) const;

	virtual void setCfg_ChanOpenTcDetectionMode(int channel, OtdMode mode);
	virtual OtdMode getCfg_ChanOpenTcDetectionMode(int channel) const;

	virtual void setCfg_OpenTcDetectionMode(int dev, OtdMode mode);
	virtual OtdMode getCfg_OpenTcDetectionMode(int dev) const;

	virtual void setCfg_CalTableType(int dev, AiCalTableType calTableType);
	virtual AiCalTableType getCfg_CalTableType(int dev) const;

	virtual void setCfg_RejectFreqType(int dev, AiRejectFreqType calTableType);
	virtual AiRejectFreqType getCfg_RejectFreqType(int dev) const;

	virtual unsigned long long getCfg_ExpCalDate(int calTableIndex);
	virtual void getCfg_ExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen);

protected:
	virtual void loadAdcCoefficients() = 0;
	virtual int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const = 0;
	virtual double calibrateData(int channel, AiInputMode inputMode, Range range, unsigned int count, long long flags) const;

	virtual CalCoef getCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const;
	std::vector<CalCoef> getScanCalCoefs(int lowChan, int highChan, AiInputMode inputMode, Range range, long long flags) const;
	bool queueEnabled() const;
	int queueLength() const;

	virtual void check_AIn_Args(int channel, AiInputMode inputMode, Range range, AInFlag flags) const;
	virtual void check_AInScan_Args(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) const;
	virtual void check_AInLoadQueue_Args(const AiQueueElement queue[], unsigned int numElements) const;
	virtual void check_AInSetTrigger_Args(TriggerType trigtype, int trigChan,  double level, double variance, unsigned int retriggerCount) const;
	virtual void check_TIn_Args(int channel, TempScale scale, TInFlag flags) const;
	virtual void check_TInArray_Args(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]) const;

	bool isValidChanQueue(const AiQueueElement queue[], unsigned int numElements) const;
	bool isValidGainQueue(const AiQueueElement queue[], unsigned int numElements) const;
	bool isValidModeQueue(const AiQueueElement queue[], unsigned int numElements) const;

	virtual void initCustomScales();
	std::vector<CustomScale> getCustomScales(int lowChan, int highChan) const;

	//void initTempUnits();

	void enableCalMode(bool enable) { mCalModeEnabled = enable;}
	bool calModeEnabled() const { return mCalModeEnabled;}

	virtual void readCalDate() {};

protected:
	AiInfo mAiInfo;
	AiConfig* mAiConfig;
	std::vector<CalCoef> mCalCoefs;
	std::vector<CustomScale> mCustomScales;
	std::vector<AiQueueElement> mAQueue;

	bool mScanTempChanSupported;
	TempUnit mScanTempUnit;
	//std::vector<TempUnit> mScanChanTempUnit;

	unsigned long long mCalDate; // cal date in sec
	unsigned long long mFieldCalDate; // cal date in sec

private:
	bool mCalModeEnabled;

};

} /* namespace ul */

#endif /* AIDEVICE_H_ */
