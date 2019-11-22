/*
 * CtrConfig.cpp
 *
 *      Author: Measurement Computing Corporation
 */
#include "CtrDevice.h"
#include "CtrConfig.h"

namespace ul
{
CtrConfig::~CtrConfig()
{
}

CtrConfig::CtrConfig(CtrDevice& ctrDevice) : mCtrDevice(ctrDevice)
{

}

void CtrConfig::setCtrCfgReg(int ctrNum, long long regVal)
{
	mCtrDevice.setCfg_CtrReg(ctrNum, regVal);
}

long long CtrConfig::getCtrCfgReg(int ctrNum)
{
	return mCtrDevice.getCfg_CtrReg(ctrNum);
}

} /* namespace ul */
