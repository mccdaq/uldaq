/*
 * UsbPdiso8.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbPdiso8.h"
#include "./dio/DioUsbPdiso8.h"

namespace ul
{

UsbPdiso8::UsbPdiso8(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setDioDevice(new DioUsbPdiso8(*this));

	addMemRegion(MR_USER, 0x080, 3968 , MA_READ | MA_WRITE);
}

UsbPdiso8::~UsbPdiso8()
{

}

} /* namespace ul */
