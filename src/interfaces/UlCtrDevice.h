/*
 * UlCtrDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULCTRDEVICE_H_
#define INTERFACES_ULCTRDEVICE_H_

#include "../uldaq.h"
#include "UlCtrInfo.h"
#include "UlCtrConfig.h"

namespace ul
{

class UlCtrDevice
{
public:
	virtual ~UlCtrDevice() {};

	virtual const UlCtrInfo& getCtrInfo() = 0;
	virtual UlCtrConfig& getCtrConfig() = 0;

	virtual unsigned long long cIn(int ctrNum) = 0;
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue) = 0;
	virtual void cClear(int ctrNum) = 0;
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType) = 0;

	virtual double cInScan(int lowCtr, int highCtr, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]) = 0;

	virtual void cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
							CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
							CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag) = 0;

	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount) = 0;

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual void stopBackground() = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULCTRDEVICE_H_ */
