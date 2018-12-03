/*
 * DioUsbDio96h.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_DIO_DIOUSBDIO96H_H_
#define HID_DIO_DIOUSBDIO96H_H_

#include "DioHidBase.h"

namespace ul
{

class UL_LOCAL DioUsbDio96h: public DioHidBase
{
public:
	DioUsbDio96h(const HidDaqDevice& daqDevice);
	virtual ~DioUsbDio96h();

	virtual void initialize();

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);
	virtual void dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	virtual void dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;

private:
	void addPorts();

private:
	enum {CMD_DCONFIG_PORT = 0x01, CMD_DIN = 0x03, CMD_DOUT = 0x04, CMD_DBITIN = 0x05, CMD_DBITOUT = 0x06,
		  CMD_DCONFIG_PORT_R = 0x09, CMD_DOUT_MULTIPLE = 0x0C, CMD_GET_ALL = 0x46};

	bool mNewMicro;
};

} /* namespace ul */

#endif /* HID_DIO_DIOUSBDIO96H_H_ */
