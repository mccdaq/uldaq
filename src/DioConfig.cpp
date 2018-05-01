/*
 * DioConfig.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioDevice.h"
#include "DioConfig.h"
#include "uldaq.h"

namespace ul
{

DioConfig::DioConfig(DioDevice& dioDevice) : mDioDevice(dioDevice)
{

}

DioConfig::~DioConfig()
{

}

unsigned long long DioConfig::getPortDirectionMask(int portNum)
{
	return mDioDevice.getCfg_PortDirectionMask(portNum);
}

} /* namespace ul */
