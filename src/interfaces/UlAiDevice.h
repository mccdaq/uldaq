/*
 * UlAiDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULAIDEVICE_H_
#define INTERFACES_ULAIDEVICE_H_

#include "../uldaq.h"
#include "UlAiConfig.h"
#include "UlAiInfo.h"

namespace ul
{

class UlAiDevice
{
public:
	virtual ~UlAiDevice() {};

	virtual const UlAiInfo& getAiInfo() = 0;
	virtual UlAiConfig& getAiConfig() = 0;

	virtual double aIn(int channel, AiInputMode mode, Range range, AInFlag flags) = 0;
	virtual double aInScan(int lowChan, int highChan, AiInputMode mode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) = 0;
	virtual void aInLoadQueue(AiQueueElement queue[], unsigned int numElements) = 0;
	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount) = 0;

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual void stopBackground() = 0;

	virtual void tIn(int channel, TempScale scale, TInFlag flags, double* data) = 0;
	virtual void tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULAIDEVICE_H_ */
