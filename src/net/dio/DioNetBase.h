/*
 * DioNetBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_DIO_DIONETBASE_H_
#define NET_DIO_DIONETBASE_H_


#include "../NetDaqDevice.h"
#include "../../DioDevice.h"

namespace ul
{

class UL_LOCAL DioNetBase: public DioDevice
{
public:
	DioNetBase(const NetDaqDevice& daqDevice);
	virtual ~DioNetBase();

	const NetDaqDevice& daqDev() const {return mNetDevice;}

private:
	const NetDaqDevice&  mNetDevice;
};

} /* namespace ul */

#endif /* NET_DIO_DIONETBASE_H_ */
