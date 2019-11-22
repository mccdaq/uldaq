/*
 * DaqDeviceConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQDEVICECONFIG_H_
#define DAQDEVICECONFIG_H_

#include "interfaces/UlDaqDeviceConfig.h"
#include "ul_internal.h"

namespace ul
{
class DaqDevice;
class UL_LOCAL DaqDeviceConfig: public UlDaqDeviceConfig
{
public:
	virtual ~DaqDeviceConfig();
	DaqDeviceConfig(DaqDevice& daqDevice);

	virtual void getVersionStr(DevVersionType verType, char* verStr, unsigned int* maxStrLen);
	virtual bool hasExp();

private:
	DaqDevice& mDaqDevice;
};

} /* namespace ul */

#endif /* DAQDEVICECONFIG_H_ */
