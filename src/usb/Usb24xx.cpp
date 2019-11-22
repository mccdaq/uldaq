/*
 * Usb24xx.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include <unistd.h>

#include "Usb24xx.h"
#include "./ai/AiUsb24xx.h"
#include "./ao/AoUsb24xx.h"
#include "./dio/DioUsb24xx.h"
#include "./ctr/CtrUsb24xx.h"

namespace ul
{
Usb24xx::Usb24xx(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	setCmdValue(CMD_RESET_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);

	mDaqDeviceInfo.setClockFreq(50000);

	setAiDevice(new AiUsb24xx(*this));
	setDioDevice(new DioUsb24xx(*this));
	setCtrDevice(new CtrUsb24xx(*this, 2));

	if(getDeviceType() == DaqDeviceId::USB_2416_4AO)
		setAoDevice(new AoUsb24xx(*this, 4));
	else if(getDeviceType() == DaqDeviceId::USB_2408_2AO)
		setAoDevice(new AoUsb24xx(*this, 2));

	if(mDaqDeviceInfo.hasAoDevice())
		mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);
	else
		mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

	setMultiCmdMem(false);
	setCmdValue(CMD_MEM_KEY, 0x30);


	// EEPROM addresses 0 - 0xFFF  (4 KB)
	addMemRegion(MR_CAL, 0, 1616, MA_READ);
	addMemRegion(MR_USER, 0x0C80, 864, MA_READ | MA_WRITE); // the last 32 bytes are used for storing self cal  date (0xFE0 - 0xFFF)
	addMemRegion(MR_RESERVED0, 0xFD0, 48, MA_READ); //self cal  date (0xFE0 - 0xFFF)
}

Usb24xx::~Usb24xx()
{

}

void Usb24xx::initilizeHardware() const
{
	unsigned char cmd = getCmdValue(UsbDaqDevice::CMD_STATUS_KEY);
	unsigned char status = 0;

	// make sure the isolated micro is ready for commands
	int retryCount = 25;
	do
	{
		queryCmd(cmd, 0, 0, &status, sizeof(status));

		if(!(status & 0x01))
			usleep(100000);

		retryCount--;
	}
	while(!(status & 0x01) && retryCount > 0);

	// check if expansion board is attached
	if(getDeviceType() == DaqDeviceId::USB_2416 || getDeviceType() == DaqDeviceId::USB_2416_4AO)
	{
		if(status & 0x02)
			mHasExp = true;
	}

	// get isolated fw version
	cmd = CMD_FW_VER;
	unsigned short fwVers[4];

	queryCmd(cmd, 0, 0, (unsigned char*) fwVers, sizeof(fwVers));

	const_cast<Usb24xx*>(this)->mRawFwMeasurementVersion = fwVers[2];

	// workaround for Bug 903348
	flashLed(1);
}

int Usb24xx::memRead_SingleCmd(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count, false);

	int bytesToRead;
	int totalBytesRead = 0;
	int bytesRead = 0;
	int remaining = count;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	int maxTransfer = getMemMaxReadSize(memType);

	if(maxTransfer)
	{
		//setMemAddress(memType, address);

		unsigned char cmd = getCmdValue(CMD_MEM_KEY);

		unsigned char* readBuff = buffer;

		int addr = address;

		do
		{
			bytesToRead = (remaining > maxTransfer ? maxTransfer : remaining);

			bytesRead  = queryCmd(cmd, addr, 0, readBuff, bytesToRead);

			remaining-= bytesRead;
			totalBytesRead += bytesRead;
			addr += bytesRead;
			readBuff += bytesRead;
		}
		while(remaining > 0);

	}
	else
		throw UlException(ERR_BAD_MEM_TYPE);


	return totalBytesRead;
}
} /* namespace ul */
