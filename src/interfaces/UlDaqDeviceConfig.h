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

	virtual void getVersionStr(DevVersionType verType, char* verStr, unsigned int* maxStrLen) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDAQDEVICECONFIG_H_ */
