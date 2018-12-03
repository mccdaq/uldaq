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

} /* namespace ul */
