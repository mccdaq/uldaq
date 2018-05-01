/*
 * DioPortInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DIOPORTINFO_H_
#define DIOPORTINFO_H_

#include "interfaces/UlDioPortInfo.h"
#include "uldaq.h"
#include "ul_internal.h"

namespace ul
{

class UL_LOCAL DioPortInfo: public UlDioPortInfo
{
public:
	DioPortInfo(int portNum, DigitalPortType type, unsigned int numBits, DigitalPortIoType ioType);
	virtual ~DioPortInfo();

	unsigned int getPortNum() const;
	DigitalPortType getType() const;
	unsigned int getNumBits() const;
	DigitalPortIoType  getPortIOType() const;

private:
	unsigned int mNum;
	DigitalPortType mType;
	unsigned int mNumBits;
	DigitalPortIoType mIoType;
};

} /* namespace ul */

#endif /* DIOPORTINFO_H_ */
