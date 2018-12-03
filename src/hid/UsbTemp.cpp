/*
 * UsbTemp.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbTemp.h"
#include "./ai/AiUsbTemp.h"
#include "./dio/DioHidAux.h"

namespace ul
{

UsbTemp::UsbTemp(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setAiDevice(new AiUsbTemp(*this));
	setDioDevice(new DioHidAux(*this));

	addMemRegion(MR_USER, 0, 256 , MA_READ | MA_WRITE);
	addMemRegion(MR_CAL, 0xF0, 6, MA_READ);
}

UsbTemp::~UsbTemp()
{

}

} /* namespace ul */
