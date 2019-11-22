/*
 * DioUsb1608hs.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSB1608HS_H_
#define USB_DIO_DIOUSB1608HS_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsb1608hs: public DioUsbBase
{
public:
	DioUsb1608hs(const UsbDaqDevice& daqDevice);
	virtual ~DioUsb1608hs();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;

private:
	enum {CMD_DIN = 0x00, CMD_DBITIN= 0x01, CMD_DOUT = 0x08, CMD_DBITOUT = 0x09};
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSB1608HS_H_ */
