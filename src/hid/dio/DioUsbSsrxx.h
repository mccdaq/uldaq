/*
 * DioUsbSsrxx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DIOUSBSSRXX_H_
#define HID_DIO_DIOUSBSSRXX_H_

#include "DioHidBase.h"

namespace ul
{

class UL_LOCAL DioUsbSsrxx: public DioHidBase
{
public:
	DioUsbSsrxx(const HidDaqDevice& daqDevice);
	virtual ~DioUsbSsrxx();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);
	virtual void dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	virtual void dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual unsigned long long getCfg_PortLogic(unsigned int portNum);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;

private:
	enum {CMD_DIN = 0x03, CMD_DOUT = 0x04, CMD_DBITIN = 0x05, CMD_DBITOUT = 0x06, CMD_DOUT_MULTIPLE = 0x0C,
		  CMD_STATUS = 0x44, CMD_GET_ALL = 0x46};

	unsigned char mPortOffset;
	bool mNewMicro;
};

} /* namespace ul */

#endif /* HID_DIO_DIOUSBSSRXX_H_ */
