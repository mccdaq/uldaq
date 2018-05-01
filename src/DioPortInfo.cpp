/*
 * DioPortInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioPortInfo.h"

namespace ul
{

DioPortInfo::DioPortInfo(int portNum, DigitalPortType type, unsigned int numBits, DigitalPortIoType ioType)
{
	mNum = portNum;
	mType = type;
	mNumBits = numBits;
	mIoType = ioType;
}

DioPortInfo::~DioPortInfo()
{

}

unsigned int DioPortInfo::getPortNum() const
{
	return mNum;
}

DigitalPortType DioPortInfo::getType() const
{
	return mType;
}

unsigned int DioPortInfo::getNumBits() const
{
	return mNumBits;
}

DigitalPortIoType  DioPortInfo::getPortIOType() const
{
	return mIoType;
}


} /* namespace ul */
