/*
 * Usb20x.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb20x.h"
#include "./ai/AiUsb20x.h"
#include "./ao/AoUsb20x.h"
#include "./dio/DioUsb1608g.h"
#include "./ctr/CtrUsb1208hs.h"

namespace ul
{

Usb20x::Usb20x(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(70000000);

	setAiDevice(new AiUsb20x(*this));
	setDioDevice(new DioUsb1608g(*this));
	setCtrDevice(new CtrUsb1208hs(*this, 1));

	if(getDeviceType() == DaqDeviceId::USB_202 || getDeviceType() == DaqDeviceId::USB_205)
		setAoDevice(new AoUsb20x(*this, 2));

	setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0010);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0);
	setScanDoneBitMask(0);

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

	setMultiCmdMem(true);

	addMemRegion(MR_CAL, 0, 768, MA_READ);
	addMemRegion(MR_USER, 0, 256, MA_READ | MA_WRITE);
}

Usb20x::~Usb20x()
{

}
} /* namespace ul */
