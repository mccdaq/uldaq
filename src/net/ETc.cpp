/*
 * ETc.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "ETc.h"
#include "./ai/AiETc.h"
#include "./dio/DioETc.h"
#include "./ctr/CtrNet.h"

namespace ul
{
ETc::ETc(const DaqDeviceDescriptor& daqDeviceDescriptor) : NetDaqDevice(daqDeviceDescriptor)
{
	FnLog log("ETc::ETc");

	setAiDevice(new AiETc(*this));
	setDioDevice(new DioETc(*this));
	setCtrDevice(new CtrNet(*this, 1));

	addMemRegion(MR_USER, 0, 3584, MA_READ | MA_WRITE);
	addMemRegion(MR_SETTINGS, 0, 32, MA_READ | MA_WRITE);
}

ETc::~ETc()
{
	FnLog log("ETc::~ETc");
}
} /* namespace ul */
