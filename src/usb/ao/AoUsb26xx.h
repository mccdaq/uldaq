/*
 * AoUsb26xx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB26XX_H_
#define USB_AO_AOUSB26XX_H_

#include "AoUsb1208hs.h"

namespace ul
{

class UL_LOCAL AoUsb26xx: public AoUsb1208hs
{
public:
	AoUsb26xx(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb26xx();

private:
	enum { FIFO_SIZE = 4 * 1024 };
};

} /* namespace ul */

#endif /* USB_AO_AOUSB26XX_H_ */
