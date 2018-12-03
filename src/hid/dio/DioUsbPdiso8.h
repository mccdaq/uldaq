/*
 * DioUsbPdiso8.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DIOUSBPDISO8_H_
#define HID_DIO_DIOUSBPDISO8_H_

#include "DioHidBase.h"

namespace ul
{

class UL_LOCAL DioUsbPdiso8: public DioHidBase
{
public:
	DioUsbPdiso8(const HidDaqDevice& daqDevice);
	virtual ~DioUsbPdiso8();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual unsigned long long getCfg_PortIsoMask(unsigned int portNum);
	virtual void setCfg_PortIsoMask(unsigned int portNum, unsigned long long mask);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;

private:
	enum {RELAY_PORT = 0x00, INPUT_PORT = 0x01, FILTER_PORT = 0x02};
	enum {CMD_DIN = 0x03, CMD_DOUT = 0x04, CMD_DBITIN = 0x05, CMD_DBITOUT = 0x06};
};

} /* namespace ul */

#endif /* HID_DIO_DIOUSBPDISO8_H_ */
