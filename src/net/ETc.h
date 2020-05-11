/*
 * ETc.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_ETC_H_
#define NET_ETC_H_

#include "NetDaqDevice.h"

namespace ul
{

class UL_LOCAL ETc: public NetDaqDevice
{
public:
	ETc(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~ETc();
};
} /* namespace ul */

#endif /* NET_ETC_H_ */
