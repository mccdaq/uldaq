/*
 * DioInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DIOINFO_H_
#define DIOINFO_H_

#include <vector>

#include "ul_internal.h"
#include "DioPortInfo.h"
#include "interfaces/UlDioInfo.h"

namespace ul
{

class UL_LOCAL DioInfo: public UlDioInfo
{
public:
	DioInfo();
	virtual ~DioInfo();

	void addPort(unsigned int portNum, DigitalPortType type, unsigned int numBits, DigitalPortIoType ioType);
	unsigned int getNumPorts() const;

	DioPortInfo getPortInfo(unsigned int portNum) const;
	DigitalPortType getPortType(unsigned int portNum) const;
	unsigned int getNumBits(unsigned int portNum) const;
	DigitalPortIoType getPortIoType(unsigned int portNum) const;

	void setMinScanRate(DigitalDirection direction, double minRate);
	double getMinScanRate(DigitalDirection direction) const;
	void setMaxScanRate(DigitalDirection direction, double maxRate);
	double getMaxScanRate(DigitalDirection direction) const;
	void setMaxThroughput(DigitalDirection direction, double maxThroughput);
	double getMaxThroughput(DigitalDirection direction) const;
	void setDiMaxBurstThroughput(double maxThroughput);
	double getDiMaxBurstThroughput() const;
	void setDiMaxBurstRate(double maxRate);
	double getDiMaxBurstRate() const;
	void setFifoSize(DigitalDirection direction, int size);
	int getFifoSize(DigitalDirection direction) const;
	void setScanOptions(DigitalDirection direction, long long options);
	ScanOption getScanOptions(DigitalDirection direction) const;
	void setScanFlags(DigitalDirection direction, long long flags);
	long long getScanFlags(DigitalDirection direction) const;

	bool hasPacer(DigitalDirection direction) const;
	void hasPacer(DigitalDirection direction, bool hasPacer);

	void setTriggerTypes(DigitalDirection direction, long long triggerTypes);
	TriggerType getTriggerTypes(DigitalDirection direction) const;
	bool supportsTrigger(DigitalDirection direction) const;

	bool isPortSupported(DigitalPortType portType) const;
	unsigned int getPortNum(DigitalPortType portType) const;

private:
	std::vector<DioPortInfo> mPortInfo;
	TriggerType mDiTriggerTypes;
	TriggerType mDoTriggerTypes;
	long long mDiScanFlags;
	long long mDoScanFlags;

	bool mDiHasPacer;
	bool mDoHasPacer;

	double mDiMinScanRate;
	double mDiMaxScanRate;
	double mDiMaxThroughput;
	ScanOption mDiScanOptions;
	double mDiMaxBurstRate;
	double mDiMaxBurstThroughput;
	unsigned int mDiFifoSize;

	double mDoMinScanRate;
	double mDoMaxScanRate;
	double mDoMaxThroughput;
	ScanOption mDoScanOptions;
	unsigned int mDoFifoSize;

};

} /* namespace ul */

#endif /* DIOINFO_H_ */
