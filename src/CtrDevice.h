/*
 * CtrDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef CTRDEVICE_H_
#define CTRDEVICE_H_

#include "IoDevice.h"
#include "CtrInfo.h"
#include "CtrConfig.h"
#include "interfaces/UlCtrDevice.h"

namespace ul
{

class UL_LOCAL CtrDevice: public IoDevice, public UlCtrDevice
{
public:
	CtrDevice(const DaqDevice& daqDevice);
	virtual ~CtrDevice();

	virtual const UlCtrInfo& getCtrInfo() { return mCtrInfo;}
	virtual UlCtrConfig& getCtrConfig() { return *mCtrConfig;}

	virtual unsigned long long cIn(int ctrNum);
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);
	virtual double cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]);

	virtual void cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
								CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
								CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag);

	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	void initScanCountersState();
	void setScanCounterActive(int ctrNum);
	void setScanCountersInactive();
	bool isScanCounterActive(int ctrNum) const;

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_CtrReg(int ctrNum, long long regVal);
	virtual long long getCfg_CtrReg(int ctrNum) const;

protected:
	void check_CIn_Args(int ctrNum) const;
	void check_CLoad_Args(int ctrNum, CounterRegisterType regType, unsigned long long loadValue) const;
	void check_CClear_Args(int ctrNum) const;
	void check_CRead_Args(int ctrNum, CounterRegisterType regType) const;
	void check_CInScan_Args(int lowCtr, int highCtr, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]) const;
	void check_CConfigScan_Args(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
								CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
								CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag) const;

	virtual void check_CtrSetTrigger_Args(TriggerType trigtype, int trigChan,  double level, double variance, unsigned int retriggerCount) const;


protected:
	CtrInfo mCtrInfo;
	CtrConfig* mCtrConfig;

private:
	std::vector<bool> mScanCtrActive;
};

} /* namespace ul */

#endif /* CTRDEVICE_H_ */
