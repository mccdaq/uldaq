/*
 * UlDaqODevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDAQODEVICE_H_
#define INTERFACES_ULDAQODEVICE_H_

#include "../uldaq.h"
#include "UlDaqOInfo.h"

namespace ul
{

class UlDaqODevice
{
public:
	virtual ~UlDaqODevice() {};

	virtual const UlDaqOInfo& getDaqOInfo() = 0;

	virtual double daqOutScan(DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, double data[]) = 0;
	virtual void setTrigger(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) = 0;

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual void stopBackground() = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQODEVICE_H_ */
