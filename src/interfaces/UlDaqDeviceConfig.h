/*
 * UlDaqDeviceConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDAQDEVICECONFIG_H_
#define INTERFACES_ULDAQDEVICECONFIG_H_

#include "../uldaq.h"

namespace ul
{

class UlDaqDeviceConfig
{
public:
	virtual ~UlDaqDeviceConfig() {};

	virtual void setConnectionCode(long long code) = 0;
	virtual long long getConnectionCode() = 0;
	virtual void setMemUnlockCode(long long code) = 0;
	virtual long long getMemUnlockCode() = 0;
	virtual void reset() = 0;


	virtual void getVersionStr(DevVersionType verType, char* verStr, unsigned int* maxStrLen) = 0;
	virtual bool hasExp() = 0;
	virtual void getIpAddressStr(char* address, unsigned int* maxStrLen) = 0;
	virtual void getNetIfcNameStr(char* ifcName, unsigned int* maxStrLen) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQDEVICECONFIG_H_ */
