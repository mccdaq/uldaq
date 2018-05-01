/*
 * DioUsb1208hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsb1208hs.h"

namespace ul
{

DioUsb1208hs::DioUsb1208hs(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT, 16, DPIOT_BITIO);
}

DioUsb1208hs::~DioUsb1208hs()
{

}

void DioUsb1208hs::initialize()
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

void DioUsb1208hs::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 0xffff;

	daqDev().sendCmd(CMD_DTRISTATE, dir, 0, NULL, 0);

	setPortDirection(portType, direction);
}

void DioUsb1208hs::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
		portDir.set(bitNum);

	unsigned short dir = portDir.to_ulong();

	daqDev().sendCmd(CMD_DTRISTATE, dir, 0, NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioUsb1208hs::dIn(DigitalPortType portType)
{
	unsigned short portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(CMD_DPORT, 0, 0, (unsigned char*) &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb1208hs::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short val = data;

	daqDev().sendCmd(CMD_DLATCH, val, 0, NULL, 0);
}

unsigned long DioUsb1208hs::readPortDirMask(unsigned int portNum) const
{
	unsigned short dirMask;

	daqDev().queryCmd(CMD_DTRISTATE, 0, 0, (unsigned char*) &dirMask, sizeof(dirMask));

	std::bitset<16> mask(dirMask);

	return mask.to_ulong();
}


bool DioUsb1208hs::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned short portValue = dIn(portType);

	std::bitset<16> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb1208hs::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned short portValue = 0;

	daqDev().queryCmd(CMD_DLATCH, 0, 0, (unsigned char*)&portValue, sizeof(portValue));

	std::bitset<16> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	daqDev().sendCmd(CMD_DLATCH, portValue, 0, NULL, 0);
}

} /* namespace ul */
