/*
 * UsbTempAi.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbTempAi.h"
#include "./ai/AiUsbTempAi.h"
#include "./dio/DioHidAux.h"
#include "./ctr/CtrHid.h"

namespace ul
{

UsbTempAi::UsbTempAi(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setAiDevice(new AiUsbTempAi(*this));
	setDioDevice(new DioHidAux(*this));
	setCtrDevice(new CtrHid(*this, 1));

	addMemRegion(MR_USER, 0, 256 , MA_READ | MA_WRITE);
	addMemRegion(MR_CAL, 0xF0, 6, MA_READ);

}

UsbTempAi::~UsbTempAi()
{

}

} /* namespace ul */
