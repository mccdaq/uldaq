/*
 * AoHidBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_AO_AOHIDBASE_H_
#define HID_AO_AOHIDBASE_H_

#include "../HidDaqDevice.h"
#include "../../AoDevice.h"

namespace ul
{

class UL_LOCAL AoHidBase: public AoDevice
{
public:
	AoHidBase(const HidDaqDevice& daqDevice);
	virtual ~AoHidBase();

	const HidDaqDevice& daqDev() const {return mHidDevice;}

private:
	const HidDaqDevice&  mHidDevice;
};

} /* namespace ul */

#endif /* HID_AO_AOHIDBASE_H_ */
