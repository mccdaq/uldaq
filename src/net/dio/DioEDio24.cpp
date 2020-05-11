/*
 * DioEDio24.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioEDio24.h"

namespace ul
{
DioEDio24::DioEDio24(const NetDaqDevice& daqDevice) : DioNetBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT0, 8, DPIOT_BITIO);
	mDioInfo.addPort(1, AUXPORT1, 8, DPIOT_BITIO);
	mDioInfo.addPort(2, AUXPORT2, 8, DPIOT_BITIO);
}

DioEDio24::~DioEDio24()
{

}

void DioEDio24::initialize()
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

void DioEDio24::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned int portNum = mDioInfo.getPortNum(portType);

#pragma pack(1)
	struct
	{
		unsigned char mask[3];
		unsigned char dir[3];

	}cfg;
#pragma pack()

	memset(&cfg, 0, sizeof(cfg));

	cfg.mask[portNum] = 0xff;

	if (direction == DD_OUTPUT)
		cfg.dir[portNum] = 0;
	else
	{
		if(mAlarmMask[portNum].any())
			throw UlException(ERR_PORT_USED_FOR_ALARM);

		cfg.dir[portNum] = 0x00ff;
	}

	daqDev().queryCmd(CMD_DCONFIG_W, (unsigned char*) &cfg, sizeof(cfg), NULL, 0);

	setPortDirection(portType, direction);
}

void DioEDio24::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	unsigned int portNum = mDioInfo.getPortNum(portType);

#pragma pack(1)
	struct
	{
		unsigned char mask[3];
		unsigned char dir[3];

	}cfg;
#pragma pack()

	memset(&cfg, 0, sizeof(cfg));

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
	{
		if(mAlarmMask[portNum][bitNum])
			throw UlException(ERR_BIT_USED_FOR_ALARM);

		portDir.set(bitNum);
	}

	cfg.mask[portNum] = 1 << bitNum;
	cfg.dir[portNum] = portDir.to_ulong();

	daqDev().queryCmd(CMD_DCONFIG_W, (unsigned char*) &cfg, sizeof(cfg), NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioEDio24::dIn(DigitalPortType portType)
{
	check_DIn_Args(portType);

	unsigned char portVals[3] = {0};

	unsigned int portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DIN_R, 0, 0, portVals, sizeof(portVals));

	return portVals[portNum];
}

void DioEDio24::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned int portNum = mDioInfo.getPortNum(portType);

	if(mAlarmMask[portNum].any())
		throw UlException(ERR_PORT_USED_FOR_ALARM);

#pragma pack(1)
	struct
	{
		unsigned char mask[3];
		unsigned char val[3];
	}cfg;
#pragma pack()

	memset(&cfg, 0, sizeof(cfg));

	cfg.mask[portNum] = 0xff;
	cfg.val[portNum] = data;

	daqDev().queryCmd(CMD_DOUT_W, (unsigned char*) &cfg, sizeof(cfg));
}

void DioEDio24::dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DInArray_Args(lowPort, highPort, data);

	unsigned char portVals[3] = {0};
	daqDev().queryCmd(CMD_DIN_R, 0, 0, portVals, sizeof(portVals));

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int i = 0;
	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		data[i] = portVals[portNum];
		i++;
	}
}

void DioEDio24::dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DOutArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

#pragma pack(1)
	struct
	{
		unsigned char mask[3];
		unsigned char val[3];

	}cfg;
#pragma pack()

	memset(&cfg, 0, sizeof(cfg));

	int i = 0;

	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		if(mAlarmMask[portNum].any())
			throw UlException(ERR_PORT_USED_FOR_ALARM);

		cfg.mask[portNum] = 0xff;
		cfg.val[portNum] = data[i];
		i++;
	}

	daqDev().queryCmd(CMD_DOUT_W, (unsigned char*) &cfg, sizeof(cfg));
}

unsigned long DioEDio24::readPortDirMask(unsigned int portNum) const
{
	unsigned char dirMask[3];

	daqDev().queryCmd(CMD_DCONFIG_R, NULL, 0, dirMask, sizeof(dirMask));

	std::bitset<8> mask(dirMask[portNum]);

	return mask.to_ulong();
}


bool DioEDio24::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioEDio24::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned int portNum = mDioInfo.getPortNum(portType);

	if(mAlarmMask[portNum][bitNum])
		throw UlException(ERR_BIT_USED_FOR_ALARM);

#pragma pack(1)
	struct
	{
		unsigned char mask[3];
		unsigned char val[3];
	}cfg;
#pragma pack()

	memset(&cfg, 0, sizeof(cfg));

	cfg.mask[portNum] = 1 << bitNum;
	cfg.val[portNum] = bitValue ? 1 << bitNum : 0;

	daqDev().queryCmd(CMD_DOUT_W, (unsigned char*) &cfg, sizeof(cfg));
}

void DioEDio24::readAlarmMask()
{
	unsigned char enabled = 0;
	unsigned char masks[3] = {0xff, 0xff, 0xff};
	daqDev().memRead(MT_EEPROM, MR_SETTINGS, 0x16, &enabled, sizeof(enabled));

	if(enabled)
	{
		daqDev().memRead(MT_EEPROM, MR_SETTINGS, 0x18, masks, sizeof(masks));
	}

	for(int i = 0; i < 3; i++)
	{
		mAlarmMask[i] = ~masks[i];
	}
}
} /* namespace ul */
