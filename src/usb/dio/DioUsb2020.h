/*
 * DioUsb2020.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSB2020_H_
#define USB_DIO_DIOUSB2020_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsb2020: public DioUsbBase
{
public:
	DioUsb2020(const UsbDaqDevice& daqDevice);
	virtual ~DioUsb2020();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual unsigned long long getCfg_PortDirectionMask(unsigned int portNum) const;

private:
	enum {CMD_DPORT = 0x01, CMD_DLATCH = 0x02};
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSB2020_H_ */
