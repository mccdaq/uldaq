/*
 * AoConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AOCONFIG_H_
#define AOCONFIG_H_

#include "interfaces/UlAoConfig.h"
#include "ul_internal.h"

namespace ul
{
class AoDevice;

class UL_LOCAL AoConfig: public UlAoConfig
{
public:
	AoConfig(AoDevice& aoDevice);
	virtual ~AoConfig();

	virtual void setSyncMode(AOutSyncMode mode);
	virtual AOutSyncMode getSyncMode();

	virtual void setSenseMode(int channel, AOutSenseMode mode);
	virtual AOutSenseMode getSenseMode(int channel) const;

private:
	AoDevice& mAoDevice;
};

} /* namespace ul */

#endif /* AOCONFIG_H_ */
