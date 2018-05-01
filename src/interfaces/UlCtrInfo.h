/*
 * UlCtrInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULCTRINFO_H_
#define INTERFACES_ULCTRINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlCtrInfo
{
public:
	virtual ~UlCtrInfo() {};

	virtual int getNumCtrs() const = 0;
	virtual unsigned long long getCtrMeasurementTypes(unsigned int ctrNum) const = 0;
	virtual CounterMeasurementMode getCtrMeasurementModes(CounterMeasurementType type) const = 0;
	virtual CounterRegisterType getRegisterTypes() const = 0;
	virtual int getResolution() const = 0;
	virtual double getMinScanRate() const = 0;
	virtual double getMaxScanRate() const = 0;
	virtual double getMaxThroughput() const = 0;
	virtual int getFifoSize() const = 0;
	virtual ScanOption getScanOptions() const = 0;
	virtual bool hasPacer() const = 0;
	virtual bool supportsTrigger() const = 0;
	virtual TriggerType getTriggerTypes() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULCTRINFO_H_ */
