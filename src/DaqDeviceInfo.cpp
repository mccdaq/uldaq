/*
 * DaqDeviceInfo.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "DaqDeviceInfo.h"

namespace ul
{

DaqDeviceInfo::DaqDeviceInfo():mProductId(0), mHasAiDevice(false), mHasAoDevice(false), mHasDioDevice(false), mHasCioDevice(false), mHasTmrDevice(false),mHasDaqIDevice(false),mHasDaqODevice(false), mClockFreq(0.0)
{
	mEventTypes = DE_NONE;
	mMemInfo =  new DevMemInfo();
}

DaqDeviceInfo::~DaqDeviceInfo()
{
	if(mMemInfo)
		delete mMemInfo;
}


} /* namespace ul */
