/*
 * DaqDeviceManager.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqDeviceManager.h"
#include "./utility/FnLog.h"
#include "./utility/UlLock.h"
#include "./usb/UsbDaqDevice.h"

#include <algorithm>
#include <cstring>

namespace ul
{

// never freed, so they are still valid when lib_unload runs after static destructors (as it does when loaded with dlopen)
std::map<int, std::string>& DaqDeviceManager::mSupportedDevices = *new std::map<int, std::string>;
std::map<int, std::string>& DaqDeviceManager::mSupportedDtDevices = *new std::map<int, std::string>;
std::map<long long, DaqDevice*>& DaqDeviceManager::mCreatedDevicesMap = *new std::map<long long, DaqDevice*>;
pthread_mutex_t DaqDeviceManager::mCreatedDevicesMutex = PTHREAD_MUTEX_INITIALIZER;

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

	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_QUAD08, "USB-QUAD08"));

	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2001_TC, "USB-2001-TC"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2416, "USB-2416"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2416_4AO, "USB-2416-4AO"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2408, "USB-2408"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2408_2AO, "USB-2408-2AO"));

	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_2020, "USB-2020"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608HS, "USB-1608HS"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_1608HS_2AO, "USB-1608HS-2AO"));

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
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::USB_TC_32, "TC32"));

	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::E_1608, "E-1608"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::E_DIO24, "E-DIO24"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::E_TC, "E-TC"));
	mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::E_TC_32, "TC32"));



	//mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::E_1808, "E-1808"));
	//mSupportedDevices.insert(std::pair<int, std::string>(DaqDeviceId::E_1808X, "E-1808X"));


	// DT devices
	mSupportedDtDevices.insert(std::pair<int, std::string>(DaqDeviceId::DT9837_ABC, "DT9837ABC"));

	// Virtual DT devices
	mSupportedDtDevices.insert(std::pair<int, std::string>(DaqDeviceId::UL_DT9837_A, "DT9837A"));
	mSupportedDtDevices.insert(std::pair<int, std::string>(DaqDeviceId::UL_DT9837_B, "DT9837B"));
	mSupportedDtDevices.insert(std::pair<int, std::string>(DaqDeviceId::UL_DT9837_C, "DT9837C"));
}

bool DaqDeviceManager::isDaqDeviceSupported(int productId, int vendorId)
{
	bool supported = false;

	if(vendorId == UsbDaqDevice::DT_USB_VID)
	{
		std::map<int, std::basic_string<char> >::iterator itr = mSupportedDtDevices.find(productId);
		if(itr != mSupportedDtDevices.end())
			supported = true;
	}
	else
	{
		std::map<int, std::basic_string<char> >::iterator itr = mSupportedDevices.find(productId);
		if(itr != mSupportedDevices.end())
			supported = true;
	}

	return supported;
}

std::string DaqDeviceManager::getDeviceName(int productId, int vendorId)
{
	std::string deviceName;
	std::map<int, std::string>::iterator itr;

	if(vendorId == UsbDaqDevice::DT_USB_VID)
	{
		itr = mSupportedDtDevices.find(productId);

		if(itr != mSupportedDtDevices.end())
			deviceName = itr->second;
	}
	else
	{
		itr = mSupportedDevices.find(productId);

		if(itr != mSupportedDevices.end())
			deviceName = itr->second;
	}

	return deviceName;
}

void DaqDeviceManager::addToCreatedList(DaqDevice* daqDevice)
{
	FnLog log("DaqDeviceManager::addToCreatedList");

	UlLock lock(mCreatedDevicesMutex);

	mCreatedDevicesMap.insert(std::pair<long long, DaqDevice*>(daqDevice->getDeviceNumber(), daqDevice));
}

void DaqDeviceManager::removeFromCreatedList(long long deviceNumber)//DaqDevice* daqDevice)
{
	FnLog log("DaqDeviceManager::removeFromCreatedList");

	UlLock lock(mCreatedDevicesMutex);

	std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.find(deviceNumber);
	if(itr != mCreatedDevicesMap.end())
	{
		mCreatedDevicesMap.erase(itr);
	}
}

void DaqDeviceManager::releaseDevice(long long deviceNumber)
{
	FnLog log("DaqDeviceManager::releaseDevice");

	DaqDevice* daqDevice = NULL;

	{
		UlLock lock(mCreatedDevicesMutex);

		std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.find(deviceNumber);
		if(itr != mCreatedDevicesMap.end())
		{
			daqDevice = itr->second;
			mCreatedDevicesMap.erase(itr);
		}
	}

	// the destructor joins the device threads, whose callbacks can call into the library, so delete outside the lock
	if(daqDevice != NULL)
		delete daqDevice;
}

DaqDevice* DaqDeviceManager::getActualDeviceHandle(long long deviceNumber)
{
	//FnLog log("DaqDeviceManager::getActualDeviceHandle");

	DaqDevice* daqDevice = NULL;

	UlLock lock(mCreatedDevicesMutex);

	std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.find(deviceNumber);
	if(itr != mCreatedDevicesMap.end())
		daqDevice = itr->second;

	return daqDevice;
}

DaqDevice* DaqDeviceManager::getDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor)
{

	DaqDevice *daqDevice = NULL;
	DaqDeviceDescriptor desc;

	UlLock lock(mCreatedDevicesMutex);

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

	std::vector<DaqDevice*> createdDevList;

	{
		UlLock lock(mCreatedDevicesMutex);

		if(mCreatedDevicesMap.size())
			UL_LOG("#### " << mCreatedDevicesMap.size() << " lingering device(s) being released: ");
		else
			UL_LOG("No lingering device found");

		for(std::map<long long, DaqDevice*>::iterator itr = mCreatedDevicesMap.begin(); itr != mCreatedDevicesMap.end(); itr++)
			createdDevList.push_back(itr->second);

		mCreatedDevicesMap.clear();
	}

	for(unsigned int i = 0; i < createdDevList.size(); i++)
		delete createdDevList[i];

	createdDevList.clear();
}

} /* namespace ul */
