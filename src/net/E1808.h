/*
 * E1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_E1808_H_
#define NET_E1808_H_

#include "VirNetDaqDevice.h"

namespace ul
{

class UL_LOCAL E1808: public VirNetDaqDevice
{
public:
	E1808(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~E1808();
};

} /* namespace ul */

#endif /* NET_E1808_H_ */
