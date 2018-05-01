/*
 * AoUsb1608g.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB1608G_H_
#define USB_AO_AOUSB1608G_H_

#include "AoUsb1208hs.h"

namespace ul
{

class UL_LOCAL AoUsb1608g: public AoUsb1208hs
{
public:
	AoUsb1608g(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb1608g();

protected:
	virtual void readCalDate();

private:
	enum { FIFO_SIZE = 4 * 1024 };
};

} /* namespace ul */

#endif /* USB_AO_AOUSB1608G_H_ */
