/*
 * UsbQuadxx.cpp
 *
 *  Author: Measurement Computing Corporation
 */

#include <unistd.h>

#include "UsbQuad08.h"
#include "./dio/DioUsbQuad08.h"
#include "./ctr/CtrUsbQuad08.h"
#include "./tmr/TmrUsbQuad08.h"

namespace ul
{

UsbQuad08::UsbQuad08(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbIotech(daqDeviceDescriptor)
{
	FnLog log("UsbQuadxx::UsbQuadxx");

	mDaqDeviceInfo.setClockFreq(48000000);

	setDioDevice(new DioUsbQuad08(*this));
	setCtrDevice(new CtrUsbQuad08(*this, 8));
	setTmrDevice(new TmrUsbQuad08(*this, 2));
}

UsbQuad08::~UsbQuad08()
{
	FnLog log("UsbQuadxx::~UsbQuadxx");

}

void UsbQuad08::flashLed(int flashCount) const
{
	if(mDioDevice)
	{
		for(int i = 0; i < flashCount; i++)
		{
			mDioDevice->dIn(AUXPORT);

			if(i != (flashCount - 1))
				usleep(200000);
		}
	}
}
} /* namespace ul */
