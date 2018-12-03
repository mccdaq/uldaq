/*
 * DioUsbDio24.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DIOUSBDIO24_H_
#define HID_DIO_DIOUSBDIO24_H_

#include "DioHidBase.h"

namespace ul
{

class UL_LOCAL DioUsbDio24: public DioHidBase
{
public:
	DioUsbDio24(const HidDaqDevice& daqDevice);
	virtual ~DioUsbDio24();

	virtual void initialize();

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);
	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_PortInitialOutputVal(unsigned int portNum, unsigned long long val);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;
	unsigned char getPortCode(DigitalPortType portType) const;

private:
	enum {CMD_DIN = 0, CMD_DOUT = 1, CMD_BITIN = 0x02, CMD_BITOUT = 0x03, CMD_DCONFIG = 0x0D};
	unsigned char mPortCLVal;
	unsigned char mPortCHVal;
};

} /* namespace ul */

#endif /* HID_DIO_DIOUSBDIO24_H_ */
