/*
 * UlAoConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULAOCONFIG_H_
#define INTERFACES_ULAOCONFIG_H_

#include "../uldaq.h"

namespace ul
{

class UlAoConfig
{
public:
	virtual ~UlAoConfig() {};

	virtual void setSyncMode(AOutSyncMode mode) = 0;
	virtual AOutSyncMode getSyncMode() = 0;

	virtual void setSenseMode(int channel, AOutSenseMode mode) = 0;
	virtual AOutSenseMode getSenseMode(int channel) const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULAOCONFIG_H_ */
