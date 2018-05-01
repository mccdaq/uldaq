/*
 * DaqDeviceConfig.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqDevice.h"
#include "DaqDeviceConfig.h"

namespace ul
{

DaqDeviceConfig::DaqDeviceConfig(DaqDevice& daqDevice) : mDaqDevice(daqDevice)
{

}

DaqDeviceConfig::~DaqDeviceConfig()
{

}

void DaqDeviceConfig::getVersionStr(DevVersionType verType, char* verStr, unsigned int* maxStrLen)
{
	switch(verType)
	{
		case DEV_VER_FW_MAIN:
			mDaqDevice.getCfg_FwVersionStr(verStr, maxStrLen);
		break;
		case DEV_VER_FPGA:
			mDaqDevice.getCfg_FpgaVersionStr(verStr, maxStrLen);
		break;
		case DEV_VER_RADIO:
			mDaqDevice.getCfg_RadioVersionStr(verStr, maxStrLen);
		break;

		default:
			break;
	}
}

} /* namespace ul */
