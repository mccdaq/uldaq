/*
 * UlAoDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULAODEVICE_H_
#define INTERFACES_ULAODEVICE_H_

#include "../uldaq.h"
#include "UlAoConfig.h"
#include "UlAoInfo.h"

namespace ul
{

class UlAoDevice
{
public:
	virtual ~UlAoDevice() {};

	virtual const UlAoInfo& getAoInfo() = 0;
	virtual UlAoConfig& getAoConfig() = 0;

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue) = 0;
	virtual void aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]) = 0;
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]) = 0;
	virtual void setTrigger(TriggerType type, int trigChan,  double level, double variance, unsigned int retriggerCount) = 0;

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual void stopBackground() = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULAODEVICE_H_ */
