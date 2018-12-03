/*
 * DioHidAux.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioHidAux.h"

namespace ul
{

DioHidAux::DioHidAux(const HidDaqDevice& daqDevice) : DioHidBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT0, 8, DPIOT_BITIO);
}

DioHidAux::~DioHidAux()
{

}

void DioHidAux::initialize()
{
	try
	{
		initPortsDirectionMask();

		dConfigPort(AUXPORT0, DD_INPUT);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void DioHidAux::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned char dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 1;

	daqDev().sendCmd(CMD_DCONFIG_PORT, dir);

	setPortDirection(portType, direction);
}

void DioHidAux::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	unsigned char dir;

	if(direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 1;

	daqDev().sendCmd(CMD_DCONFIG_BIT, (unsigned char) bitNum, dir);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioHidAux::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(CMD_DIN, &portValue);

	return portValue;
}

void DioHidAux::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned char val = data;

	daqDev().sendCmd(CMD_DOUT, val);
}

bool DioHidAux::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;

	daqDev().queryCmd(CMD_DBITIN, (unsigned char) bitNum, &bitValue);

	return bitValue? true : false;
}

void DioHidAux::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char bitVal = bitValue ? 1 : 0;

	daqDev().sendCmd(CMD_DBITOUT, (unsigned char) bitNum, bitVal);
}

unsigned long DioHidAux::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;

	mask.set();

	return mask.to_ulong();
}

} /* namespace ul */
