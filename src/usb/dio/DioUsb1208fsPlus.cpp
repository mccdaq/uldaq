/*
 * DioUsb1208fsPlus.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsb1208fsPlus.h"

namespace ul
{
DioUsb1208fs_Plus::DioUsb1208fs_Plus(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, FIRSTPORTA, 8, DPIOT_IO);
	mDioInfo.addPort(1, FIRSTPORTB, 8, DPIOT_IO);
}

DioUsb1208fs_Plus::~DioUsb1208fs_Plus()
{

}

void DioUsb1208fs_Plus::initialize()
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

void DioUsb1208fs_Plus::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 1;

	daqDev().sendCmd(CMD_DTRISTATE, dir, portNum, NULL, 0);

	setPortDirection(portType, direction);
}

unsigned long long DioUsb1208fs_Plus::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DPORT, 0, portNum, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb1208fs_Plus::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned short val = data;

	daqDev().sendCmd(CMD_DLATCH, val, portNum, NULL, 0);
}

unsigned long DioUsb1208fs_Plus::readPortDirMask(unsigned int portNum) const
{
	unsigned char dir;

	daqDev().queryCmd(CMD_DTRISTATE, 0, portNum, &dir, sizeof(dir));

	std::bitset<8> mask;

	if(dir)
		mask.set();
	else
		mask.reset();

	return mask.to_ulong();
}


bool DioUsb1208fs_Plus::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb1208fs_Plus::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
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
