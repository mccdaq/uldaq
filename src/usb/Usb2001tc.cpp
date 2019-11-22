/*
 * Usb2001tc.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "Usb2001tc.h"
#include "./ai/AiUsb2001tc.h"

#include <iostream>
#include <sstream>

namespace ul
{
Usb2001tc::Usb2001tc(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	FnLog log("Usb2001tc::Usb2001tc");

	setAiDevice(new AiUsb2001tc(*this));

	//setMultiCmdMem(true);

	//addMemRegion(MR_CAL, 0, 768, MA_READ);*/
	//addMemRegion(MR_USER, 0x100, 240, MA_READ | MA_WRITE);
}

Usb2001tc::~Usb2001tc()
{
	FnLog log("Usb2001tc::~Usb2001tc");

}

void Usb2001tc::flashLed(int flashCount) const
{
	pthread_mutex_t& devMutex = getDeviceMutex();
	UlLock lock(devMutex);

	uint8_t cmd = 0x80;
	char reply[64];

	std::ostringstream msg;
	msg << "DEV:FLASHLED/" <<  flashCount;

	sendCmd(cmd, 0, 0, (unsigned char*) msg.str().c_str(), msg.str().length(), 2000);
	queryCmd(cmd, 0, 0, (unsigned char*) &reply, sizeof(reply), 2000, false);
}
} /* namespace ul */
