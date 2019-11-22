/*
 * DioUsbPdiso8.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbPdiso8.h"

namespace ul
{

DioUsbPdiso8::DioUsbPdiso8(const HidDaqDevice& daqDevice) : DioHidBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT0, 8, DPIOT_OUT);
	mDioInfo.addPort(1, AUXPORT1, 8, DPIOT_IN);

}

DioUsbPdiso8::~DioUsbPdiso8()
{

}

void DioUsbPdiso8::initialize()
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

unsigned long long DioUsbPdiso8::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned char portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DIN, portNum, &portValue);

	return portValue;
}

void DioUsbPdiso8::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned char val = data;

	daqDev().sendCmd(CMD_DOUT, portNum, val);
}

bool DioUsbPdiso8::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;

	unsigned char portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DBITIN, portNum, bitNum, &bitValue);

	return bitValue? true : false;
}

void DioUsbPdiso8::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char portNum = mDioInfo.getPortNum(portType);

	unsigned char bitVal = bitValue ? 1 : 0;

	daqDev().sendCmd(CMD_DBITOUT, portNum, (unsigned char) bitNum, bitVal);
}

unsigned long long DioUsbPdiso8::getCfg_PortIsoMask(unsigned int portNum)
{
	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

	unsigned char mask = 0;

	portNum = FILTER_PORT;

	daqDev().queryCmd(CMD_DIN, portNum, &mask);

	mask = ~mask;

	return mask;
}

void DioUsbPdiso8::DioUsbPdiso8::setCfg_PortIsoMask(unsigned int portNum, unsigned long long mask)
{
	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);

	portNum = FILTER_PORT;

	unsigned char val = ~mask;

	daqDev().sendCmd(CMD_DOUT, portNum, val);
}

unsigned long DioUsbPdiso8::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;

	if(portNum == 0)
		mask.reset();
	else
		mask.set();

	return mask.to_ulong();
}

} /* namespace ul */
