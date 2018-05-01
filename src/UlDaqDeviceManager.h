/*
 * UlDaqDeviceManager.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef ULDAQDEVICEMANAGER_H_
#define ULDAQDEVICEMANAGER_H_

#include <vector>

#include "./interfaces/UlDaqDevice.h"
#include "uldaq.h"


namespace ul
{

class UlDaqDeviceManager
{
public:
	UlDaqDeviceManager();
	virtual ~UlDaqDeviceManager();

	static std::vector<DaqDeviceDescriptor> getDaqDeviceInventory(DaqDeviceInterface InterfaceType);
	static UlDaqDevice& createDaqDevice(DaqDeviceDescriptor daqDevDescriptor);
	static void releaseDaqDevice(UlDaqDevice& daqDevice);
};

} /* namespace ul */

#endif /* UlDaqDeviceManager_H_ */
