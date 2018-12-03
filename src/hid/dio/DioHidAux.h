/*
 * DioHidAux.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DioHidAux_H_
#define HID_DIO_DioHidAux_H_

#include "DioHidBase.h"

namespace ul
{

class UL_LOCAL DioHidAux: public DioHidBase
{
public:
	DioHidAux(const HidDaqDevice& daqDevice);
	virtual ~DioHidAux();

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
	enum {CMD_DCONFIG_PORT = 0x01, CMD_DCONFIG_BIT = 0x02, CMD_DIN = 0x03, CMD_DOUT = 0x04, CMD_DBITIN = 0x05, CMD_DBITOUT = 0x06};
};

} /* namespace ul */

#endif /* HID_DIO_DioHidAux_H_ */
