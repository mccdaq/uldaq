/*
 * CtrHidBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_CTR_CTRHIDBASE_H_
#define HID_CTR_CTRHIDBASE_H_

#include "../HidDaqDevice.h"
#include "../../CtrDevice.h"

namespace ul
{

class UL_LOCAL CtrHidBase: public CtrDevice
{
public:
	CtrHidBase(const HidDaqDevice& daqDevice);
	virtual ~CtrHidBase();

	const HidDaqDevice& daqDev() const {return mHidDevice;}

private:
	const HidDaqDevice&  mHidDevice;
};

} /* namespace ul */

#endif /* HID_CTR_CTRHIDBASE_H_ */
