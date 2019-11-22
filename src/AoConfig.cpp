/*
 * AoConfig.cpp
 *
 *     Author: Measurement Computing Corporation
 */
#include "AoDevice.h"
#include "AoConfig.h"

namespace ul
{

AoConfig::AoConfig(AoDevice& aoDevice) : mAoDevice(aoDevice)
{

}


AoConfig::~AoConfig()
{

}

void AoConfig::setSyncMode(AOutSyncMode mode)
{
	mAoDevice.setCfg_SyncMode(mode);
}

AOutSyncMode AoConfig::getSyncMode()
{
	return mAoDevice.getCfg_SyncMode();
}

void AoConfig::setSenseMode(int channel, AOutSenseMode mode)
{
	mAoDevice.setCfg_SenseMode(channel, mode);
}

AOutSenseMode AoConfig::getSenseMode(int channel) const
{
	return mAoDevice.getCfg_SenseMode(channel);
}

} /* namespace ul */
