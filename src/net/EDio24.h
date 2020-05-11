/*
 * EDio24.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_EDIO24_H_
#define NET_EDIO24_H_

#include "NetDaqDevice.h"

namespace ul
{

class UL_LOCAL EDio24: public NetDaqDevice
{
public:
	EDio24(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~EDio24();
};
} /* namespace ul */

#endif /* NET_EDIO24_H_ */
