/*
 * DioUsbErbxx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DIOUSBERBXX_H_
#define HID_DIO_DIOUSBERBXX_H_

#include "DioHidBase.h"

namespace ul
{

class UL_LOCAL DioUsbErbxx: public DioHidBase
{
public:
	DioUsbErbxx(const HidDaqDevice& daqDevice);
	virtual ~DioUsbErbxx();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual unsigned long long getCfg_PortLogic(unsigned int portNum);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;

private:
	enum {CMD_DIN = 0x03, CMD_DOUT = 0x04, CMD_DBITIN = 0x05, CMD_DBITOUT = 0x06, CMD_STATUS = 0x44};

	unsigned char mPortOffset;
};

} /* namespace ul */

#endif /* HID_DIO_DIOUSBERBXX_H_ */
