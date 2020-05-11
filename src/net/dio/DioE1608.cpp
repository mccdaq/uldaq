/*
 * DioE1608.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioE1608.h"


namespace ul
{

DioE1608::DioE1608(const NetDaqDevice& daqDevice) : DioNetBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT, 8, DPIOT_BITIO);
}

DioE1608::~DioE1608()
{

}

void DioE1608::initialize()
{
	try
	{
		initPortsDirectionMask();

		readAlarmMask();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void DioE1608::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned char dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
	{
		if(mAlarmMask.any())
			throw UlException(ERR_PORT_USED_FOR_ALARM);

		dir = 0x00ff;
	}

	daqDev().queryCmd(CMD_DCONFIG_W, &dir, sizeof(dir), NULL, 0);

	setPortDirection(portType, direction);
}

void DioE1608::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
	{
		if(mAlarmMask[bitNum])
			throw UlException(ERR_BIT_USED_FOR_ALARM);

		portDir.set(bitNum);
	}

	unsigned char dir = portDir.to_ulong();

	daqDev().queryCmd(CMD_DCONFIG_W, &dir, sizeof(dir), NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioE1608::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(CMD_DIN_R, 0, 0, &portValue, sizeof(portValue));

	return portValue;
}

void DioE1608::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	if(mAlarmMask.any())
		throw UlException(ERR_PORT_USED_FOR_ALARM);

	unsigned char val = data;

	daqDev().queryCmd(CMD_DOUT_W, &val, sizeof(val));
}

unsigned long DioE1608::readPortDirMask(unsigned int portNum) const
{
	unsigned char dirMask;

	daqDev().queryCmd(CMD_DCONFIG_R, NULL, 0, &dirMask, sizeof(dirMask));

	std::bitset<8> mask(dirMask);

	return mask.to_ulong();
}


bool DioE1608::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioE1608::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	if(mAlarmMask[bitNum])
		throw UlException(ERR_BIT_USED_FOR_ALARM);

	unsigned char portValue = 0;

	daqDev().queryCmd(CMD_DOUT_R, 0, 0, &portValue, sizeof(portValue));

	std::bitset<8> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	daqDev().queryCmd(CMD_DOUT_W, &portValue, sizeof(portValue));
}

void DioE1608::readAlarmMask()
{
	unsigned char enabled = 0;
	unsigned char mask = 0xff;
	daqDev().memRead(MT_EEPROM, MR_SETTINGS, 0x16, &enabled, sizeof(enabled));

	if(enabled)
	{
		daqDev().memRead(MT_EEPROM, MR_SETTINGS, 0x17, &mask, sizeof(mask));
	}

	mAlarmMask = ~mask;
}

} /* namespace ul */
