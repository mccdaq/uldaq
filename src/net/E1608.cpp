/*
 * E1608.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "E1608.h"
#include "./ai/AiE1608.h"
#include "./ao/AoE1608.h"
#include "./dio/DioE1608.h"
#include "./ctr/CtrNet.h"

namespace ul
{
E1608::E1608(const DaqDeviceDescriptor& daqDeviceDescriptor) : NetDaqDevice(daqDeviceDescriptor)
{
	FnLog log("E1608::E1608");

	mDaqDeviceInfo.setClockFreq(80000000);

	setAiDevice(new AiE1608(*this));
	setAoDevice(new AoE1608(*this));
	setDioDevice(new DioE1608(*this));
	setCtrDevice(new CtrNet(*this, 1));


	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

	addMemRegion(MR_CAL, 0, 512, MA_READ);
	addMemRegion(MR_USER, 0, 1024, MA_READ | MA_WRITE);
	addMemRegion(MR_SETTINGS, 0, 512, MA_READ | MA_WRITE);
}

E1608::~E1608()
{
	FnLog log("E1608::~E1608");

}
} /* namespace ul */
