/*
 * Usb1608fsPlus.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb1608fsPlus.h"
#include "./ai/AiUsb1608fsPlus.h"
#include "./dio/DioUsb1608g.h"
#include "./ctr/CtrUsb1208hs.h"

#include <iostream>

namespace ul
{
Usb1608fs_Plus::Usb1608fs_Plus(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	FnLog log("Usb1608fs_Plus::Usb1608fs_Plus");

	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(40000000);

	setAiDevice(new AiUsb1608fs_Plus(*this));
	setDioDevice(new DioUsb1608g(*this));
	setCtrDevice(new CtrUsb1208hs(*this, 1));

	setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0000);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0);

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

	setMultiCmdMem(true);

	addMemRegion(MR_CAL, 0, 768, MA_READ);
	addMemRegion(MR_USER, 0, 256, MA_READ | MA_WRITE);
}

Usb1608fs_Plus::~Usb1608fs_Plus()
{
	FnLog log("Usb1608fs_Plus::~Usb1608fs_Plus");

}

} /* namespace ul */
