/*
 * UlDaqIDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDAQIDEVICE_H_
#define INTERFACES_ULDAQIDEVICE_H_

#include "../uldaq.h"
#include "UlDaqIInfo.h"

namespace ul
{

class UlDaqIDevice
{
public:
	virtual ~UlDaqIDevice() {};

	virtual const UlDaqIInfo& getDaqIInfo() = 0;

	virtual double daqInScan(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, double data[]) = 0;
	virtual void setTrigger(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) = 0;

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual void stopBackground() = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQIDEVICE_H_ */
