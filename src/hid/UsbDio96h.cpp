/*
 * UsbDio96h.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbDio96h.h"
#include "./dio/DioUsbDio96h.h"
#include "./ctr/CtrHid.h"

namespace ul
{

UsbDio96h::UsbDio96h(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setDioDevice(new DioUsbDio96h(*this));

	if(getDeviceType() != DaqDeviceId::USB_DIO96H_50)
		setCtrDevice(new CtrHid(*this, 1));

	addMemRegion(MR_USER, 0x080, 3968 , MA_READ | MA_WRITE);
}

UsbDio96h::~UsbDio96h()
{

}

} /* namespace ul */
