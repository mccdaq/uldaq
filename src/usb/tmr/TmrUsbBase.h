/*
 * TmrUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_TMR_TMRUSBBASE_H_
#define USB_TMR_TMRUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../TmrDevice.h"

namespace ul
{

class UL_LOCAL TmrUsbBase : public TmrDevice
{
public:
	TmrUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~TmrUsbBase();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

private:
	const UsbDaqDevice&  mUsbDevice;
};

} /* namespace ul */

#endif /* USB_TMR_TMRUSBBASE_H_ */
