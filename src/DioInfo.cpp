/*
 * DioInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioInfo.h"

namespace ul
{
DioInfo::DioInfo()
{
	mDiHasPacer = false;
	mDoHasPacer = false;

	mDiMinScanRate = 0;
	mDiMaxScanRate = 0;
	mDiMaxThroughput = 0.0;
	mDiMaxBurstRate = 0;
	mDiMaxBurstThroughput = 0.0;
	mDiScanOptions = SO_DEFAULTIO;
	mDiScanFlags = 0;
	mDiTriggerTypes = (TriggerType) 0;
	mDiFifoSize = 0;

	mDoMinScanRate = 0;
	mDoMaxScanRate = 0;
	mDoMaxThroughput = 0;
	mDoScanOptions = SO_DEFAULTIO;
	mDoScanFlags = 0;
	mDoTriggerTypes = (TriggerType) 0;
	mDoFifoSize = 0;
}

DioInfo::~DioInfo()
{

}

void DioInfo::addPort(unsigned int portNum, DigitalPortType type, unsigned int numBits, DigitalPortIoType ioType)
{
	mPortInfo.push_back(DioPortInfo(portNum, type, numBits, ioType));
}

unsigned int DioInfo::getNumPorts() const
{
	return mPortInfo.size();
}

DioPortInfo DioInfo::getPortInfo(unsigned int portNum) const
{
	DioPortInfo portInfo(-1, (DigitalPortType)-1, -1, (DigitalPortIoType)-1);

	if(portNum < getNumPorts())
	{
		portInfo = mPortInfo[portNum];
	}

	return portInfo;
}

DigitalPortType DioInfo::getPortType(unsigned int portNum) const
{
	DigitalPortType type = (DigitalPortType) 0;

	if(portNum < getNumPorts())
	{
		type = mPortInfo[portNum].getType();
	}

	return type;
}


unsigned int DioInfo::getNumBits(unsigned int portNum) const
{
	unsigned int bitCount = 0;

	if(portNum < getNumPorts())
	{
		bitCount = mPortInfo[portNum].getNumBits();
	}

	return bitCount;
}

DigitalPortIoType DioInfo::getPortIoType(unsigned int portNum) const
{
	DigitalPortIoType ioType = (DigitalPortIoType) 0;

	if(portNum < getNumPorts())
	{
		ioType = mPortInfo[portNum].getPortIOType();
	}

	return ioType;
}


bool DioInfo::hasPacer(DigitalDirection direction) const
{
	bool hasPaser = false;

	if(direction == DD_INPUT)
		hasPaser = mDiHasPacer;
	else if(direction == DD_OUTPUT)
		hasPaser = mDoHasPacer;

	return hasPaser;
}

void DioInfo::hasPacer(DigitalDirection direction, bool hasPacer)
{
	if(direction == DD_INPUT)
		mDiHasPacer = hasPacer;
	else
		mDoHasPacer = hasPacer;
}


void DioInfo::setMinScanRate(DigitalDirection direction, double minRate)
{
	if(direction == DD_INPUT)
		mDiMinScanRate = minRate;
	else
		mDoMinScanRate = minRate;
}


double DioInfo::getMinScanRate(DigitalDirection direction) const
{
	double minRate = 0;

	if(direction == DD_INPUT)
		minRate = mDiMinScanRate;
	else if(direction == DD_OUTPUT)
		minRate = mDoMinScanRate;

	return minRate;
}

void DioInfo::setMaxScanRate(DigitalDirection direction, double maxRate)
{
	if(direction == DD_INPUT)
		mDiMaxScanRate = maxRate;
	else
		mDoMaxScanRate = maxRate;
}

double DioInfo::getMaxScanRate(DigitalDirection direction) const
{
	double maxRate = 0;

	if(direction == DD_INPUT)
		maxRate = mDiMaxScanRate;
	else if(direction == DD_OUTPUT)
		maxRate = mDoMaxScanRate;

	return maxRate;
}

void DioInfo::setMaxThroughput(DigitalDirection direction, double maxThroughput)
{
	if(direction == DD_INPUT)
		mDiMaxThroughput = maxThroughput;
	else
		mDoMaxThroughput = maxThroughput;
}

double DioInfo::getMaxThroughput(DigitalDirection direction) const
{
	double maxThroughput = 0;

	if(direction == DD_INPUT)
		maxThroughput = mDiMaxThroughput;
	else if(direction == DD_OUTPUT)
		maxThroughput = mDoMaxThroughput;

	return maxThroughput;
}

void DioInfo::setDiMaxBurstThroughput(double maxThroughput)
{
	mDiMaxBurstThroughput = maxThroughput;
}

double DioInfo::getDiMaxBurstThroughput() const
{
	return mDiMaxBurstThroughput;
}

void DioInfo::setDiMaxBurstRate(double maxRate)
{
	mDiMaxBurstRate = maxRate;
}

double DioInfo::getDiMaxBurstRate() const
{
	return mDiMaxBurstRate;
}

void DioInfo::setFifoSize(DigitalDirection direction, int size)
{
	if(direction == DD_INPUT)
		mDiFifoSize = size;
	else
		mDoFifoSize = size;
}

int DioInfo::getFifoSize(DigitalDirection direction) const
{
	int fifoSize = 0;
	if(direction == DD_INPUT)
		fifoSize =  mDiFifoSize;
	else if(direction == DD_OUTPUT)
		fifoSize = mDoFifoSize;

	return fifoSize;
}

void DioInfo::setScanOptions(DigitalDirection direction, long long options)
{
	if(direction == DD_INPUT)
		mDiScanOptions = (ScanOption) options;
	else
		mDoScanOptions = (ScanOption) options;
}

ScanOption DioInfo::getScanOptions(DigitalDirection direction) const
{
	ScanOption scanOptions = SO_DEFAULTIO;

	if(direction == DD_INPUT)
		scanOptions = mDiScanOptions;
	else if(direction == DD_OUTPUT)
		scanOptions = mDoScanOptions;

	return scanOptions;
}

void DioInfo::setScanFlags(DigitalDirection direction, long long flags)
{
	if(direction == DD_INPUT)
		mDiScanFlags = flags;
	else
		mDoScanFlags = flags;
}

long long DioInfo::getScanFlags(DigitalDirection direction) const
{
	long long flags = 0;

	if(direction == DD_INPUT)
		flags = mDiScanFlags;
	else if(direction == DD_OUTPUT)
		flags = mDoScanFlags;

	return flags;
}

void DioInfo::setTriggerTypes(DigitalDirection direction, long long triggerTypes)
{
	if(direction == DD_INPUT)
		mDiTriggerTypes = (TriggerType) triggerTypes;
	else
		mDoTriggerTypes = (TriggerType) triggerTypes;
}


TriggerType DioInfo::getTriggerTypes(DigitalDirection direction) const
{
	TriggerType types = TRIG_NONE;

	if(direction == DD_INPUT)
		types = mDiTriggerTypes;
	else if(direction == DD_OUTPUT)
		types = mDoTriggerTypes;

	return types;
}


bool DioInfo::supportsTrigger(DigitalDirection direction) const
{
	bool supportsTrig = false;

	TriggerType types;

	if(direction == DD_INPUT)
		types = mDiTriggerTypes;
	else
		types = mDoTriggerTypes;

	if(types)
		supportsTrig = true;

	return supportsTrig;
}

bool DioInfo::isPortSupported(DigitalPortType portType) const
{
	bool supported = false;

	DigitalPortType type;

	for(unsigned int i = 0; i < getNumPorts(); i++)
	{
		type = getPortType(i);

		if(type == portType)
		{
			supported = true;
			break;
		}
	}

	return supported;
}

unsigned int DioInfo::getPortNum(DigitalPortType portType) const
{
	unsigned int portIndex = 0;
	DigitalPortType type;

	for(unsigned int i = 0; i < getNumPorts(); i++)
	{
		type = getPortType(i);

		if(type == portType)
		{
			portIndex = i;
			break;
		}
	}

	return portIndex;
}

} /* namespace ul */
