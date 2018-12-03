/*
 * UsbSsrxx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbSsrxx.h"
#include "./dio/DioUsbSsrxx.h"

namespace ul
{

UsbSsrxx::UsbSsrxx(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setDioDevice(new DioUsbSsrxx(*this));

	addMemRegion(MR_USER, 0x080, 3968 , MA_READ | MA_WRITE);
}

UsbSsrxx::~UsbSsrxx()
{

}

} /* namespace ul */
