/*
 * DioUsb24xx.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSB24XX_H_
#define USB_DIO_DIOUSB24XX_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsb24xx: public DioUsbBase
{
public:
	DioUsb24xx(const UsbDaqDevice& daqDevice);
	virtual ~DioUsb24xx();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual unsigned long long getCfg_PortDirectionMask(unsigned int portNum) const;

private:
	enum {CMD_DIN = 0x00, CMD_DOUT = 0x01};
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSB24XX_H_ */
