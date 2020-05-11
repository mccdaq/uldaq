/*
 * AoNetBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AO_AONETBASE_H_
#define NET_AO_AONETBASE_H_

#include "../NetDaqDevice.h"
#include "../../AoDevice.h"

namespace ul
{

class UL_LOCAL AoNetBase: public AoDevice
{
public:
	AoNetBase(const NetDaqDevice& daqDevice);
	virtual ~AoNetBase();

	const NetDaqDevice& daqDev() const {return mNetDevice;}

protected:
	virtual void loadDacCoefficients();
	virtual void readCalDate();


private:
	const NetDaqDevice&  mNetDevice;
};

} /* namespace ul */

#endif /* NET_AO_AONETBASE_H_ */
