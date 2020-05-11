/*
 * UsbTc32.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbTc32.h"
#include "./ai/AiUsbTc32.h"
#include "./dio/DioUsbTc32.h"

namespace ul
{
UsbTc32::UsbTc32(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	setCmdValue(CMD_STATUS_KEY, 0x42);

	setAiDevice(new AiUsbTc32(*this));
	setDioDevice(new DioUsbTc32(*this));

	setMultiCmdMem(true);
	setCmdValue(CMD_MEM_USER_KEY, 0x30);
	setCmdValue(CMD_MEM_SETTINGS_KEY, 0x32);

	addMemRegion(MR_USER, 0, 3839, MA_READ | MA_WRITE); // 0xEFF is not a valid address
	addMemRegion(MR_SETTINGS, 0, 32, MA_READ | MA_WRITE);
}

UsbTc32::~UsbTc32()
{

}

void UsbTc32::initilizeHardware() const
{
	unsigned char cmd = getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned short status = 0;
	mHasExp = false;

	queryCmd(cmd, 0, 0, (unsigned char*)&status, sizeof(status));

	if(status & 0x01)
		mHasExp = true;

	readMeasurementFwVersions();
}

void UsbTc32::readMeasurementFwVersions() const
{
	unsigned short fwVers[6];
	memset(fwVers, 0, sizeof(fwVers));

	queryCmd(CMD_VERSION, 0, 0, (unsigned char*) fwVers, sizeof(fwVers));

	const_cast<UsbTc32*>(this)->mRawFwMeasurementVersion = fwVers[2];
	const_cast<UsbTc32*>(this)->mRawFwExpMeasurementVersion = fwVers[4];
}
} /* namespace ul */
