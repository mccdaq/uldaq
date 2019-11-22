/*
 * DioUsb1608hs.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "DioUsb1608hs.h"

namespace ul
{
DioUsb1608hs::DioUsb1608hs(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT0, 8, DPIOT_IN);
	mDioInfo.addPort(1, AUXPORT1, 8, DPIOT_OUT);
}

DioUsb1608hs::~DioUsb1608hs()
{

}

void DioUsb1608hs::initialize()
{
	try
	{
		initPortsDirectionMask();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

unsigned long long DioUsb1608hs::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned char cmd = CMD_DIN;

	if(portType == AUXPORT1)
		cmd = CMD_DOUT;

	daqDev().queryCmd(cmd, 0, 0, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb1608hs::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned char val = data;

	daqDev().sendCmd(CMD_DOUT, 0, 0, &val, sizeof(val));
}

bool DioUsb1608hs::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;
	unsigned char cmd = CMD_DBITIN;

	if(portType == AUXPORT1)
		cmd = CMD_DBITOUT;

	daqDev().queryCmd(cmd, (unsigned short) bitNum, 0, &bitValue, sizeof(bitValue));

	return bitValue;
}

void DioUsb1608hs::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char params[2];
	params[0] = (unsigned char) bitNum;
	params[1] = (unsigned char) (bitValue ? 1 : 0);

	daqDev().sendCmd(CMD_DBITOUT, 0, 0, params, sizeof(params));
}

unsigned long DioUsb1608hs::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;

	if(portNum == 0)
		mask.set();
	else
		mask.reset();

	return mask.to_ulong();
}
} /* namespace ul */
