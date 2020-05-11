/*
 * EDio24.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "EDio24.h"

#include "./dio/DioEDio24.h"
#include "./ctr/CtrNet.h"

namespace ul
{
EDio24::EDio24(const DaqDeviceDescriptor& daqDeviceDescriptor) : NetDaqDevice(daqDeviceDescriptor)
{
	FnLog log("EDio24::EDio24");

	setDioDevice(new DioEDio24(*this));
	setCtrDevice(new CtrNet(*this, 1));

	addMemRegion(MR_USER, 0, 3824, MA_READ | MA_WRITE);
	addMemRegion(MR_SETTINGS, 0, 256, MA_READ | MA_WRITE);
}

EDio24::~EDio24()
{
	FnLog log("EDio24::~EDio24");
}
} /* namespace ul */
