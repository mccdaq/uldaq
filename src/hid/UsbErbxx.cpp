/*
 * UsbErbxx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbErbxx.h"
#include "./dio/DioUsbErbxx.h"

namespace ul
{

UsbErbxx::UsbErbxx(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setDioDevice(new DioUsbErbxx(*this));

	addMemRegion(MR_USER, 0x080, 3968 , MA_READ | MA_WRITE);
}

UsbErbxx::~UsbErbxx()
{

}

} /* namespace ul */
