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

void DaqDeviceConfig::setConnectionCode(long long code)
{
	mDaqDevice.setCfg_ConnectionCode(code);
}

long long DaqDeviceConfig::getConnectionCode()
{
	return mDaqDevice.getCfg_ConnectionCode();
}

void DaqDeviceConfig::setMemUnlockCode(long long code)
{
	mDaqDevice.setCfg_MemUnlockCode(code);
}

long long DaqDeviceConfig::getMemUnlockCode()
{
	return mDaqDevice.getCfg_MemUnlockCode();
}

void DaqDeviceConfig::reset()
{
	mDaqDevice.setCfg_Reset();
}


void DaqDeviceConfig::getVersionStr(DevVersionType verType, char* verStr, unsigned int* maxStrLen)
{
	switch(verType)
	{
		case DEV_VER_FW_MAIN:
		case DEV_VER_FW_MEASUREMENT:
		case DEV_VER_FW_MEASUREMENT_EXP:
			mDaqDevice.getCfg_FwVersionStr(verType, verStr, maxStrLen);
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

bool DaqDeviceConfig::hasExp()
{
	return mDaqDevice.hasExp();
}

void DaqDeviceConfig::getIpAddressStr(char* address, unsigned int* maxStrLen)
{
	mDaqDevice.getCfg_IpAddress(address, maxStrLen);
}
void DaqDeviceConfig::getNetIfcNameStr(char* ifcName, unsigned int* maxStrLen)
{
	mDaqDevice.getCfg_NetIfcName(ifcName, maxStrLen);
}

} /* namespace ul */
