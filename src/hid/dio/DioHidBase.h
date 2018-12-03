/*
 * DioHidBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DIOHIDBASE_H_
#define HID_DIO_DIOHIDBASE_H_

#include "../HidDaqDevice.h"
#include "../../DioDevice.h"

namespace ul
{

class UL_LOCAL DioHidBase: public DioDevice
{
public:
	DioHidBase(const HidDaqDevice& daqDevice);
	virtual ~DioHidBase();

	const HidDaqDevice& daqDev() const {return mHidDevice;}

private:
	const HidDaqDevice&  mHidDevice;
};

} /* namespace ul */

#endif /* HID_DIO_DIOHIDBASE_H_ */
