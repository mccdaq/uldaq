/*
 * E1608.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_E1608_H_
#define NET_E1608_H_

#include "NetDaqDevice.h"

namespace ul
{

class UL_LOCAL E1608: public NetDaqDevice
{
public:
	E1608(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~E1608();
};

} /* namespace ul */

#endif /* NET_E1608_H_ */
