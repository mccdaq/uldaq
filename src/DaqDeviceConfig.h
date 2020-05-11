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

	virtual void setConnectionCode(long long code);
	virtual long long getConnectionCode();
	virtual void setMemUnlockCode(long long code);
	virtual long long getMemUnlockCode();
	virtual void reset();

	virtual void getVersionStr(DevVersionType verType, char* verStr, unsigned int* maxStrLen);
	virtual bool hasExp();
	virtual void getIpAddressStr(char* address, unsigned int* maxStrLen);
	virtual void getNetIfcNameStr(char* ifcName, unsigned int* maxStrLen);

private:
	DaqDevice& mDaqDevice;
};

} /* namespace ul */

#endif /* DAQDEVICECONFIG_H_ */
