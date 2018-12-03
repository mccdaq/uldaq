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

void DioConfig::setPortInitialOutputVal(int portNum, unsigned long long val)
{
	mDioDevice.setCfg_PortInitialOutputVal(portNum, val);
}

void DioConfig::setPortIsoMask(int portNum, unsigned long long mask)
{
	mDioDevice.setCfg_PortIsoMask(portNum, mask);
}

unsigned long long DioConfig::getPortIsoMask(int portNum)
{
	return mDioDevice.getCfg_PortIsoMask(portNum);
}

unsigned long long DioConfig::getPortLogic(int portNum)
{
	return mDioDevice.getCfg_PortLogic(portNum);
}

} /* namespace ul */
