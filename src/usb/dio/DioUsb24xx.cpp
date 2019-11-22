/*
 * DioUsb24xx.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "DioUsb24xx.h"


namespace ul
{

DioUsb24xx::DioUsb24xx(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT0, 8, DPIOT_NONCONFIG);
	mDioInfo.addPort(0, AUXPORT1, 8, DPIOT_NONCONFIG);
	mDioInfo.addPort(0, AUXPORT2, 8, DPIOT_NONCONFIG);
}

DioUsb24xx::~DioUsb24xx()
{

}

void DioUsb24xx::initialize()
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

unsigned long long DioUsb24xx::dIn(DigitalPortType portType)
{
	DigitalPortType portTypeCheck = portType;

	if(!daqDev().hasExp() && (portType == AUXPORT1 || portType == AUXPORT2))
		throw UlException(ERR_BAD_PORT_TYPE);

	check_DIn_Args(portTypeCheck);

	unsigned char portValue = 0;

	unsigned short portNum = portType - AUXPORT0;

	daqDev().queryCmd(CMD_DIN, portNum, 0, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb24xx::dOut(DigitalPortType portType, unsigned long long data)
{
	DigitalPortType portTypeCheck = portType;

	if(!daqDev().hasExp() && (portType == AUXPORT1 || portType == AUXPORT2))
		throw UlException(ERR_BAD_PORT_TYPE);

	check_DOut_Args(portTypeCheck, data);

	unsigned char portNum = portType - AUXPORT0;

	unsigned char val = data;

	unsigned char buf[2] = { portNum, val};

	daqDev().sendCmd(CMD_DOUT, 0, 0, buf, sizeof(buf));
}

bool DioUsb24xx::dBitIn(DigitalPortType portType, int bitNum)
{
	DigitalPortType portTypeCheck = portType;

	if(!daqDev().hasExp() && (portType == AUXPORT1 || portType == AUXPORT2))
		throw UlException(ERR_BAD_PORT_TYPE);

	check_DBitIn_Args(portTypeCheck, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb24xx::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	DigitalPortType portTypeCheck = portType;

	if(!daqDev().hasExp() && (portType == AUXPORT1 || portType == AUXPORT2))
		throw UlException(ERR_BAD_PORT_TYPE);

	check_DBitOut_Args(portTypeCheck, bitNum);

	unsigned char portValue = 0;

	unsigned short portNum = portType - AUXPORT0;

	daqDev().queryCmd(CMD_DOUT, portNum, 0, &portValue, sizeof(portValue));

	std::bitset<8> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	dOut(portType, portValue);
}

unsigned long long DioUsb24xx::getCfg_PortDirectionMask(unsigned int portNum) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}




} /* namespace ul */

