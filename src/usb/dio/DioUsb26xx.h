/*
 * DioUsb26xx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSB26XX_H_
#define USB_DIO_DIOUSB26XX_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsb26xx: public DioUsbBase
{
public:
	DioUsb26xx(const UsbDaqDevice& daqDevice);
	virtual ~DioUsb26xx();

	virtual void initialize();

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);
	virtual void dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction);

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;

private:
	enum {CMD_DTRISTATE = 0x00, CMD_DPORT = 0x01, CMD_DLATCH = 0x02};
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSB26XX_H_ */
