/*
 * UlDioInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDIOINFO_H_
#define INTERFACES_ULDIOINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlDioInfo
{
public:
	virtual ~UlDioInfo() {};

	virtual unsigned int getNumPorts() const = 0;
	virtual	DigitalPortType getPortType(unsigned int portNum) const = 0;
	virtual	unsigned int getNumBits(unsigned int portNum) const = 0;
	virtual	DigitalPortIoType getPortIoType(unsigned int portNum) const = 0;
	virtual bool hasPacer(DigitalDirection direction) const = 0;


	virtual double getMinScanRate(DigitalDirection direction) const = 0;
	virtual double getMaxScanRate(DigitalDirection direction) const = 0;
	virtual double getMaxThroughput(DigitalDirection direction) const = 0;
	virtual int getFifoSize(DigitalDirection direction) const = 0;
	virtual ScanOption getScanOptions(DigitalDirection direction) const = 0;

	virtual TriggerType getTriggerTypes(DigitalDirection direction) const = 0;
	virtual bool supportsTrigger(DigitalDirection direction) const = 0;

	/*virtual void getTriggerTypes(DigitalDirection direction, TriggerType types[], int* count) const = 0;
	virtual int getTriggerTypeCount(DigitalDirection direction) const = 0;
	virtual TriggerType getTriggerType(DigitalDirection direction, unsigned int index) const = 0;*/
};

} /* namespace ul */

#endif /* INTERFACES_ULDIOINFO_H_ */
