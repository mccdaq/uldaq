/*
 * CtrConfig.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef CTRCONFIG_H_
#define CTRCONFIG_H_

#include "interfaces/UlCtrConfig.h"
#include "ul_internal.h"

namespace ul
{
class CtrDevice;

class UL_LOCAL CtrConfig: public UlCtrConfig
{
public:
	virtual ~CtrConfig();
	CtrConfig(CtrDevice& ctrDevice);

	virtual void setCtrCfgReg(int ctrNum, long long regVal);
	virtual long long getCtrCfgReg(int ctrNum);

private:
	CtrDevice& mCtrDevice;
};

} /* namespace ul */

#endif /* CTRCONFIG_H_ */
