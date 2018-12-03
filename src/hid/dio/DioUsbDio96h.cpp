/*
 * DioUsbDio96h.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbDio96h.h"

namespace ul
{

DioUsbDio96h::DioUsbDio96h(const HidDaqDevice& daqDevice) : DioHidBase(daqDevice)
{
	mNewMicro = false;
	addPorts();
}

DioUsbDio96h::~DioUsbDio96h()
{

}

void DioUsbDio96h::initialize()
{
	unsigned short rawFwVer = daqDev().getRawFwVer();

	if(rawFwVer < 0x200)
		mNewMicro = false;
	else
		mNewMicro = true;

	try
	{
		initPortsDirectionMask();

		// set the ports to input mode if fw version is less than 2.0
		if(!mNewMicro)
		{
			for(unsigned int portNum = 0; portNum < mDioInfo.getNumPorts(); portNum++)
				dConfigPort(mDioInfo.getPortType(portNum), DD_INPUT);
		}
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void DioUsbDio96h::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned char portNum = mDioInfo.getPortNum(portType);

	unsigned char dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 1;

	daqDev().sendCmd(CMD_DCONFIG_PORT, portNum, dir);

	setPortDirection(portType, direction);
}

unsigned long long DioUsbDio96h::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned char portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DIN, portNum, &portValue);

	return portValue;
}

void DioUsbDio96h::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned char val = data;

	daqDev().sendCmd(CMD_DOUT, portNum, val);
}

void DioUsbDio96h::dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DInArray_Args(lowPort, highPort, data);

	unsigned char portVals[16];

	daqDev().queryCmd(CMD_GET_ALL, portVals, sizeof(portVals), 2000);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int i = 0;
	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		data[i] = portVals[portNum];
		i++;
	}
}

void DioUsbDio96h::dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DOutArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	if(mNewMicro)
	{
		unsigned char portVals[16] = {0};
		unsigned short portsMask = 0;

		int i = 0;
		for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
		{
			portVals[portNum] = data[i];
			portsMask |= 1 << portNum;
			i++;
		}

		daqDev().sendCmd(CMD_DOUT_MULTIPLE, portsMask, portVals, sizeof(portVals));
	}
	else
	{
		int i = 0;
		for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
		{
			dOut(mDioInfo.getPortType(portNum), data[i]);
			i++;
		}
	}
}


bool DioUsbDio96h::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;

	unsigned char portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DBITIN, portNum, bitNum, &bitValue);

	return bitValue? true : false;
}

void DioUsbDio96h::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char portNum = mDioInfo.getPortNum(portType);

	unsigned char bitVal = bitValue ? 1 : 0;

	daqDev().sendCmd(CMD_DBITOUT, portNum, (unsigned char) bitNum, bitVal);
}




unsigned long DioUsbDio96h::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;
	mask.set();

	if(mNewMicro)
	{
		unsigned char cmd = CMD_DCONFIG_PORT_R;
		unsigned short portsDir = 0;

		daqDev().queryCmd(cmd, &portsDir);

		std::bitset<16> portsDirMask(portsDir);

		if(portsDirMask[portNum])
			mask.set();
		else
			mask.reset();
	}

	return mask.to_ulong();
}

void DioUsbDio96h::addPorts()
{
	mDioInfo.addPort(0, FIRSTPORTA, 8, DPIOT_IO);
	mDioInfo.addPort(1, FIRSTPORTB, 8, DPIOT_IO);
	mDioInfo.addPort(2, FIRSTPORTCL, 4, DPIOT_IO);
	mDioInfo.addPort(3, FIRSTPORTCH, 4, DPIOT_IO);

	mDioInfo.addPort(4, SECONDPORTA, 8, DPIOT_IO);
	mDioInfo.addPort(5, SECONDPORTB, 8, DPIOT_IO);
	mDioInfo.addPort(6, SECONDPORTCL, 4, DPIOT_IO);
	mDioInfo.addPort(7, SECONDPORTCH, 4, DPIOT_IO);

	mDioInfo.addPort(8, THIRDPORTA, 8, DPIOT_IO);
	mDioInfo.addPort(9, THIRDPORTB, 8, DPIOT_IO);
	mDioInfo.addPort(10, THIRDPORTCL, 4, DPIOT_IO);
	mDioInfo.addPort(11, THIRDPORTCH, 4, DPIOT_IO);

	mDioInfo.addPort(12, FOURTHPORTA, 8, DPIOT_IO);
	mDioInfo.addPort(13, FOURTHPORTB, 8, DPIOT_IO);
	mDioInfo.addPort(14, FOURTHPORTCL, 4, DPIOT_IO);
	mDioInfo.addPort(15, FOURTHPORTCH, 4, DPIOT_IO);
}

} /* namespace ul */
