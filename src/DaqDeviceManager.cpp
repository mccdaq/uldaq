/*
 * DaqDeviceManager.cpp
 *
 *  Created on: Jul 30, 2015
 *     Author: Measurement Computing Corporation
 */

#include "DaqDeviceManager.h"
#include "./utility/FnLog.h"

#include <algorithm>
#include <cstring>

namespace ul
{

std::map<int, std::string> DaqDeviceManager::mSupportedDevices;
std::map<long long, DaqDevice*> DaqDeviceManager::mCreatedDevicesMap;

DaqDeviceManager::DaqDeviceManager()
{

}

DaqDeviceManager::~DaqDeviceManager()
{

}

void DaqDeviceManager::addSupportedDaqDevice()
{
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1208FS_PLUS, "USB-1208FS-Plus"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1408FS_PLUS, "USB-1408FS-Plus"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608FS_PLUS, "USB-1608FS-Plus"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_201, "USB-201"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_202, "USB-202"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_204, "USB-204"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_205, "USB-205"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1208HS, "USB-1208HS"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1208HS_2AO, "USB-1208HS-2AO"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1208HS_4AO, "USB-1208HS-4AO"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608G, "USB-1608G"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608G_2, "USB-1608G"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608GX, "USB-1608GX"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608GX_2, "USB-1608GX"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608GX_2AO, "USB-1608GX-2AO"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608GX_2AO_2, "USB-1608GX-2AO"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1808, "USB-1808"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1808X, "USB-1808X"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2623, "USB-2623"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2627, "USB-2627"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2633, "USB-2633"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2637, "USB-2637"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_DIO32HS, "USB-DIO32HS"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_CTR04, "USB-CTR04"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_CTR08, "USB-CTR08"));

	// HID devices
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_DIO96H, "USB-DIO96H"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_DIO96H_50, "USB-DIO96H/50"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1024LS, "USB-1024LS"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1024HLS, "USB-1024HLS"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_DIO24, "USB-DIO24/37"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_DIO24H, "USB-DIO24H/37"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_PDISO8, "USB-PDISO8"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_PDISO8_40, "USB-PDISO8/40"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_SSR24, "USB-SSR24"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_SSR08, "USB-SSR08"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_ERB24, "USB-ERB24"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_ERB08, "USB-ERB08"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3101, "USB-3101"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3102, "USB-3102"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3103, "USB-3103"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3104, "USB-3104"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3105, "USB-3105"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3106, "USB-3106"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3110, "USB-3110"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3112, "USB-3112"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_3114, "USB-3114"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_TEMP, "USB-TEMP"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_TC, "USB-TC"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_TEMP_AI, "USB-TEMP_AI"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_TC_AI, "USB-TC_AI"));
}

bool DaqDeviceManager::isDaqDeviceSupported(int productId)
{
	bool supported = false;

	if(mSupportedDevices.empty())
		addSupportedDaqDevice();

	std::map<int, std::basic_string<char> >::iterator itr = mSupportedDevices.find(productId);
	if(itr != mSupportedDevices.end())
		supported = true;

	return supported;
}

std::string DaqDeviceManager::getDeviceName(int productId)
{
	std::string deviceName;

	std::map<int, std::string>::iterator itr = mSupportedDevices.find(productId);

	if(itr != mSupportedDevices.end())
		deviceName = itr->second;

	return deviceName;
}

void DaqDeviceManager::addToCreatedList(DaqDevice* daqDevice)
{
	FnLog log("DaqDeviceManager::addToCreatedList");

	mCreatedDevicesMap.insert(std::pair<long long, DaqDevice*>(daqDevice->getDeviceNumber(), daqDevice));
}

void DaqDeviceManager::removeFromCreatedList(long long deviceNumber)//DaqDevice* daqDevice)
{
	FnLog log("DaqDeviceManager::removeFromCreatedList");

	std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.find(deviceNumber);
	if(itr != mCreatedDevicesMap.end())
	{
		mCreatedDevicesMap.erase(itr);
	}
}

void DaqDeviceManager::releaseDevice(long long deviceNumber)
{
	FnLog log("DaqDeviceManager::releaseDevice");

	std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.find(deviceNumber);
	if(itr != mCreatedDevicesMap.end())
	{
		delete (itr->second);
	}

	// when a device object is deleted, it removes itself from mCreatedDevices list in the DaqDevice destructor
	// so there is no need to removed it from the list here. This approach prevents deleting the device again if the users used delete
	// instead of DaqDeviceManager::releaseDaqDevice to free DaqDevice objects.
}

DaqDevice* DaqDeviceManager::getActualDeviceHandle(long long deviceNumber)
{
	FnLog log("DaqDeviceManager::getActualDeviceHandle");

	DaqDevice* daqDevice = NULL;

	std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.find(deviceNumber);
	if(itr != mCreatedDevicesMap.end())
		daqDevice = itr->second;

	return daqDevice;
}

DaqDevice* DaqDeviceManager::getDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor)
{

	DaqDevice *daqDevice = NULL;
	DaqDeviceDescriptor desc;

	for(std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.begin(); itr != mCreatedDevicesMap.end(); itr++)
	{
		desc = itr->second->getDescriptor();

		if(desc.productId == daqDeviceDescriptor.productId)
		{
			if(std::memcmp(desc.uniqueId, daqDeviceDescriptor.uniqueId, sizeof(desc.uniqueId)) == 0)
			{
				daqDevice = itr->second;
				break;
			}
		}
	}

	return daqDevice;
}

void DaqDeviceManager::releaseDevices()
{
	FnLog log("DaqDeviceManager::releaseDevices");

	if(mCreatedDevicesMap.size())
		UL_LOG("#### " << mCreatedDevices.size() << " lingering device(s) being released: ");
	else
		UL_LOG("No lingering device found");

	//for(unsigned int i = 0; i < mCreatedDevicesMap.size(); i++)
	//	delete mCreatedDevicesMap[i];

	for(std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.begin(); itr != mCreatedDevicesMap.end(); itr++)
		delete itr->second;

	// when a device object is deleted, it removes itself from mCreatedDevices list in the DaqDevice destructor
	// so there is no need to removed it from the list here. This approach prevents deleting the device again if the users used delete
	// instead of DaqDeviceManager::releaseDaqDevice to free DaqDevice objects.2
}

} /* namespace ul */
