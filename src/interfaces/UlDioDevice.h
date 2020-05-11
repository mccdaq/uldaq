/*
 * UlDioDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDIODEVICE_H_
#define INTERFACES_ULDIODEVICE_H_

#include "../uldaq.h"
#include "UlDioConfig.h"
#include "UlDioInfo.h"

namespace ul
{

class UlDioDevice
{
public:
	virtual ~UlDioDevice() {};

	virtual const UlDioInfo& getDioInfo() = 0;
	virtual UlDioConfig& getDioConfig() = 0;

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction) = 0;
	virtual void dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction) = 0;

	virtual unsigned long long dIn(DigitalPortType portType) = 0;
	virtual void dOut(DigitalPortType portType, unsigned long long data) = 0;
	virtual bool dBitIn(DigitalPortType portType, int bitNum) = 0;
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue) = 0;
	virtual double dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]) = 0;
	virtual double dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]) = 0;
	virtual void dInSetTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount) = 0;
	virtual void dOutSetTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount) = 0;
	virtual UlError dInGetStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual UlError dOutGetStatus(ScanStatus* status, TransferStatus* xferStatus) = 0;
	virtual void dInStopBackground() = 0;
	virtual void dOutStopBackground() = 0;
	virtual void dClearAlarm(DigitalPortType portType, unsigned long long mask) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDIODEVICE_H_ */
