/*
 * main.cpp
 *
 *     Author: Measurement Computing Corporation
 */
#include <iostream>

#include "./utility/ErrorMap.h"
#include "./utility/Endian.h"
#include "./ul_internal.h"
#include "./DaqDeviceManager.h"
#include "./usb/UsbDaqDevice.h"
#include "./hid/HidDaqDevice.h"

void __attribute__ ((constructor)) lib_load(void);
void __attribute__ ((destructor)) lib_unload(void);

using namespace ul;


void lib_load(void)
{
	UL_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>");
	UL_LOG("UL loading...");

	if(Endian::isLittleEndian())
		UL_LOG("Little_Endian");
	else
		UL_LOG("Big_Endian");

	ErrorMap::init();

	/*UsbDaqDevice::usb_init();
	HidDaqDevice::hidapi_init();

	SuspendMonitor::init();*/
}


void lib_unload(void)
{
	DaqDeviceManager::releaseDevices();

	HidDaqDevice::hidapi_exit();

	UsbDaqDevice::usb_exit();

	UL_LOG("UL unloaded");
	UL_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}


