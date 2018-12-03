/*
 * Usb1208fsPlus.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb1208fsPlus.h"
#include "./ai/AiUsb1208fsPlus.h"
#include "./ao/AoUsb1208fsPlus.h"
#include "./dio/DioUsb1208fsPlus.h"
#include "./ctr/CtrUsb1208hs.h"

namespace ul
{

Usb1208fs_Plus::Usb1208fs_Plus(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	FnLog log("Usb1208fs_Plus::Usb1208fs_Plus");

	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(60000000);

	setAiDevice(new AiUsb1208fs_Plus(*this));
	setAoDevice(new AoUsb1208fs_Plus(*this, 2));
	setDioDevice(new DioUsb1208fs_Plus(*this));
	setCtrDevice(new CtrUsb1208hs(*this, 1));

	setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0010);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0x0008);
	setScanDoneBitMask(0);

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);

	setMultiCmdMem(true);

	addMemRegion(MR_CAL, 0, 768, MA_READ);
	addMemRegion(MR_USER, 0, 256, MA_READ | MA_WRITE);
}

Usb1208fs_Plus::~Usb1208fs_Plus()
{
	FnLog log("Usb1208fs_Plus::~Usb1208fs_Plus");

}
} /* namespace ul */
