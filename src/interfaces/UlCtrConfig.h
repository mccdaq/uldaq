/*
 * UlCtrConfig.h
 *
 *  Created on: Jan 16, 2019
 *      Author: mcc
 */

#ifndef INTERFACES_ULCTRCONFIG_H_
#define INTERFACES_ULCTRCONFIG_H_

namespace ul
{

class UlCtrConfig
{
public:
	virtual ~UlCtrConfig() {};

	virtual void setCtrCfgReg(int ctrNum, long long) = 0;
	virtual long long getCtrCfgReg(int ctrNum) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULCTRCONFIG_H_ */
