/*
 * DioUsb26xx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsb26xx.h"

namespace ul
{
DioUsb26xx::DioUsb26xx(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, FIRSTPORTA, 8, DPIOT_BITIO);
	mDioInfo.addPort(1, FIRSTPORTB, 8, DPIOT_BITIO);
	mDioInfo.addPort(2, FIRSTPORTC, 8, DPIOT_BITIO);
}

DioUsb26xx::~DioUsb26xx()
{

}

void DioUsb26xx::initialize()
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

void DioUsb26xx::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 0x00ff;

	daqDev().sendCmd(CMD_DTRISTATE, dir, portNum, NULL, 0);

	setPortDirection(portType, direction);
}

void DioUsb26xx::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
		portDir.set(bitNum);

	unsigned char dir = portDir.to_ulong();

	daqDev().sendCmd(CMD_DTRISTATE, dir, portNum, NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioUsb26xx::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DPORT, 0, portNum, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb26xx::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned short val = data;

	daqDev().sendCmd(CMD_DLATCH, val, portNum, NULL, 0);
}

unsigned long DioUsb26xx::readPortDirMask(unsigned int portNum) const
{
	unsigned char dirMask;

	daqDev().queryCmd(CMD_DTRISTATE, 0, portNum, &dirMask, sizeof(dirMask));

	std::bitset<8> mask(dirMask);

	return mask.to_ulong();
}


bool DioUsb26xx::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb26xx::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned char portValue = 0;

	daqDev().queryCmd(CMD_DLATCH, 0, portNum, &portValue, sizeof(portValue));

	std::bitset<8> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	daqDev().sendCmd(CMD_DLATCH, portValue, portNum, NULL, 0);
}

} /* namespace ul */
