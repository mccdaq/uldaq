/*
 * DioUsbQuad08.h
 *
 * Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSBQUAD08_H_
#define USB_DIO_DIOUSBQUAD08_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsbQuad08: public DioUsbBase
{
public:
	DioUsbQuad08(const UsbDaqDevice& daqDevice);
	virtual ~DioUsbQuad08();

	virtual void initialize();

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);
	virtual void dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction);

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSBQUAD08_H_ */
