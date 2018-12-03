/*
 * AiHidBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_AI_AIHIDBASE_H_
#define HID_AI_AIHIDBASE_H_

#include "../HidDaqDevice.h"
#include "../../AiDevice.h"

namespace ul
{

class UL_LOCAL AiHidBase: public AiDevice
{
public:
	AiHidBase(const HidDaqDevice& daqDevice);
	virtual ~AiHidBase();

	const HidDaqDevice& daqDev() const {return mHidDevice;}

protected:
	virtual void readCalDate();

private:
	const HidDaqDevice&  mHidDevice;
};

} /* namespace ul */

#endif /* HID_AI_AIHIDBASE_H_ */
