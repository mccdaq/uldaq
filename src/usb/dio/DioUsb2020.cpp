/*
 * DioUsb2020.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "DioUsb2020.h"

namespace ul
{

DioUsb2020::DioUsb2020(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT, 8, DPIOT_NONCONFIG);
}

DioUsb2020::~DioUsb2020()
{

}

void DioUsb2020::initialize()
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

unsigned long long DioUsb2020::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(CMD_DPORT, 0, 0, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb2020::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short val = data;

	daqDev().sendCmd(CMD_DLATCH, val, 0, NULL, 0);
}

bool DioUsb2020::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb2020::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
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

unsigned long long DioUsb2020::getCfg_PortDirectionMask(unsigned int portNum) const
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}


} /* namespace ul */

