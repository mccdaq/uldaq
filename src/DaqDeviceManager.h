/*
 * DaqDeviceManager.h
 *
 *  Created on: Jul 30, 2015
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQDEVICEMANAGER_H_
#define DAQDEVICEMANAGER_H_

#include "ul_internal.h"
#include "DaqDevice.h"
#include <string>
#include <map>
#include <vector>

namespace ul
{

class UL_LOCAL DaqDeviceManager
{
public:
	DaqDeviceManager();
	virtual ~DaqDeviceManager();

	static bool isDaqDeviceSupported(int productId);
	static std::string getDeviceName(int productId);

	static void addToCreatedList(DaqDevice* daqDevice);
	static void removeFromCreatedList(long long deviceNumber);
	static void releaseDevice(long long deviceNumber);
	static void releaseDevices();
	static DaqDevice* getActualDeviceHandle(long long deviceNumber);
	static DaqDevice* getDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);

private:
	static void addSupportedDaqDevice();

private:
	static std::map<int, std::string> mSupportedDevices;
	static std::map<long long, DaqDevice*> mCreatedDevicesMap;
};

} /* namespace ul */

#endif /* DaqDeviceManager_H_ */
