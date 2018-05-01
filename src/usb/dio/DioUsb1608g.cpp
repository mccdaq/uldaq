/*
 * DioUsb1608g.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsb1608g.h"

namespace ul
{

DioUsb1608g::DioUsb1608g(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT, 8, DPIOT_BITIO);
}

DioUsb1608g::~DioUsb1608g()
{

}

void DioUsb1608g::initialize()
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

void DioUsb1608g::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 0x00ff;

	daqDev().sendCmd(CMD_DTRISTATE, dir, 0, NULL, 0);

	setPortDirection(portType, direction);
}

void DioUsb1608g::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
		portDir.set(bitNum);

	unsigned char dir = portDir.to_ulong();

	daqDev().sendCmd(CMD_DTRISTATE, dir, 0, NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioUsb1608g::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(CMD_DPORT, 0, 0, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb1608g::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short val = data;

	daqDev().sendCmd(CMD_DLATCH, val, 0, NULL, 0);
}

unsigned long DioUsb1608g::readPortDirMask(unsigned int portNum) const
{
	unsigned char dirMask;

	daqDev().queryCmd(CMD_DTRISTATE, 0, 0, &dirMask, sizeof(dirMask));

	std::bitset<8> mask(dirMask);

	return mask.to_ulong();
}


bool DioUsb1608g::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb1608g::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char portValue = 0;

	daqDev().queryCmd(CMD_DLATCH, 0, 0, &portValue, sizeof(portValue));

	std::bitset<8> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	daqDev().sendCmd(CMD_DLATCH, portValue, 0, NULL, 0);
}

} /* namespace ul */
