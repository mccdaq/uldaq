/*
 * UsbDio24.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbDio24.h"
#include "./dio/DioUsbDio24.h"
#include "./ctr/CtrUsbDio24.h"

namespace ul
{

UsbDio24::UsbDio24(const DaqDeviceDescriptor& daqDeviceDescriptor) : HidDaqDevice(daqDeviceDescriptor)
{
	setDioDevice(new DioUsbDio24(*this));
	setCtrDevice(new CtrUsbDio24(*this, 1));

}

UsbDio24::~UsbDio24()
{

}

void UsbDio24::flashLed(int flashCount) const
{
	unsigned char buffer[MAX_PACKET_SIZE] = {0};

	buffer[1] = CMD_FLASH_LED;
	size_t length = sizeof(buffer);

	sendRawCmd(buffer, &length);
}

} /* namespace ul */
