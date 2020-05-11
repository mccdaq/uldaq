/*
 * DioEDio24.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_DIO_DIOEDIO24_H_
#define NET_DIO_DIOEDIO24_H_

#include "DioNetBase.h"

namespace ul
{

class UL_LOCAL DioEDio24: public DioNetBase
{
public:
	DioEDio24(const NetDaqDevice& daqDevice);
	virtual ~DioEDio24();

	virtual void initialize();

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);
	virtual void dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction);

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);
	virtual void dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	virtual void dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;
	void readAlarmMask();

private:
	enum {CMD_DIN_R = 0x00, CMD_DOUT_R = 0x02, CMD_DOUT_W = 0x03 ,CMD_DCONFIG_R = 0x04, CMD_DCONFIG_W = 0x05};

	std::bitset<8> mAlarmMask[3];
};
} /* namespace ul */

#endif /* NET_DIO_DIOEDIO24_H_ */
