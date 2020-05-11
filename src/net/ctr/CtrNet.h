/*
 * CtrNet.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_CTR_CTRNET_H_
#define NET_CTR_CTRNET_H_

#include "../NetDaqDevice.h"
#include "../../CtrDevice.h"


namespace ul
{

class UL_LOCAL CtrNet: public CtrDevice
{
public:
	CtrNet(const NetDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrNet();

	const NetDaqDevice& daqDev() const {return mNetDevice;}

	virtual unsigned long long cIn(int ctrNum);
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);

private:
	const NetDaqDevice&  mNetDevice;

	enum { CMD_CTR_R = 0x30, CMD_CTR_W = 0x31 };
};

} /* namespace ul */

#endif /* NET_CTR_CTRNET_H_ */
