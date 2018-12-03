/*
 * Usb3100.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb3100.h"
#include "./ao/AoUsb3100.h"
#include "./dio/DioHidAux.h"
#include "./ctr/CtrHid.h"

namespace ul
{

Usb3100::Usb3100(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setAoDevice(new AoUsb3100(*this));
	setDioDevice(new DioHidAux(*this));
	setCtrDevice(new CtrHid(*this, 1));

	addMemRegion(MR_USER, 0, 256 , MA_READ | MA_WRITE);
	addMemRegion(MR_CAL, 0x100, 1024, MA_READ);
}

Usb3100::~Usb3100()
{

}

void Usb3100::flashLed(int flashCount) const
{
	sendCmd(CMD_FLASH_LED, (unsigned char) flashCount);
}

} /* namespace ul */
