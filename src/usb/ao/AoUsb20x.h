/*
 * AoUsb20x.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB20X_H_
#define USB_AO_AOUSB20X_H_

#include "AoUsbBase.h"

namespace ul
{

class UL_LOCAL AoUsb20x: public AoUsbBase
{
public:
	AoUsb20x(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb20x();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) { return ERR_BAD_DEV_TYPE; }
	virtual void stopBackground() { throw UlException(ERR_BAD_DEV_TYPE); }

protected:
	virtual int getCalCoefIndex(int channel, Range range) const { return -1;}

private:
	enum { CMD_AOUT = 0x18};
};

} /* namespace ul */

#endif /* USB_AO_AOUSB20X_H_ */
