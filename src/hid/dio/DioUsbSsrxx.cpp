/*
 * DioUsbSsrxx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbSsrxx.h"

namespace ul
{

DioUsbSsrxx::DioUsbSsrxx(const HidDaqDevice& daqDevice) : DioHidBase(daqDevice)
{
	if(daqDevice.getDeviceType() == DaqDeviceId::USB_SSR08)
	{
		mDioInfo.addPort(0, FIRSTPORTCL, 4, DPIOT_NONCONFIG);
		mDioInfo.addPort(1, FIRSTPORTCH, 4, DPIOT_NONCONFIG);

		mPortOffset = 2;
	}
	else
	{
		mDioInfo.addPort(0, FIRSTPORTA, 8, DPIOT_NONCONFIG);
		mDioInfo.addPort(1, FIRSTPORTB, 8, DPIOT_NONCONFIG);
		mDioInfo.addPort(2, FIRSTPORTCL, 4, DPIOT_NONCONFIG);
		mDioInfo.addPort(3, FIRSTPORTCH, 4, DPIOT_NONCONFIG);

		mPortOffset = 0;
	}

	mNewMicro = false;

}

DioUsbSsrxx::~DioUsbSsrxx()
{

}

void DioUsbSsrxx::initialize()
{
	unsigned short rawFwVer = daqDev().getRawFwVer();

	if(rawFwVer < 0x200)
		mNewMicro = false;
	else
		mNewMicro = true;

	try
	{
		initPortsDirectionMask();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

unsigned long long DioUsbSsrxx::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned char portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	daqDev().queryCmd(CMD_DIN, portNum, &portValue);

	return portValue;
}

void DioUsbSsrxx::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	std::bitset<32> portDirectionMask = getPortDirection(portType);

	if(portDirectionMask.any())
		throw UlException(ERR_WRONG_DIG_CONFIG);

	unsigned short portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	unsigned char val = data;

	daqDev().sendCmd(CMD_DOUT, portNum, val);
}

void DioUsbSsrxx::dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DInArray_Args(lowPort, highPort, data);

	unsigned char portVals[4];

	daqDev().queryCmd(CMD_GET_ALL, portVals, sizeof(portVals), 2000);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int i = 0;
	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		data[i] = portVals[portNum + mPortOffset];
		i++;
	}
}

void DioUsbSsrxx::dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DOutArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	std::bitset<32> portDirectionMask;

	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		portDirectionMask = getPortDirection(mDioInfo.getPortType(portNum));

		if(portDirectionMask.any())
			throw UlException(ERR_WRONG_DIG_CONFIG);
	}

	if(mNewMicro)
	{
		unsigned char portVals[4] = {0};
		unsigned short portsMask = 0;

		int i = 0;
		for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
		{
			portVals[portNum + mPortOffset] = data[i];
			portsMask |= 1 << (portNum + mPortOffset);
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

bool DioUsbSsrxx::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;

	unsigned char portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	daqDev().queryCmd(CMD_DBITIN, portNum, bitNum, &bitValue);

	return bitValue? true : false;
}

void DioUsbSsrxx::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	std::bitset<32> portDirectionMask = getPortDirection(portType);

	if(portDirectionMask[bitNum])
		throw UlException(ERR_WRONG_DIG_CONFIG);

	unsigned char portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	unsigned char bitVal = bitValue ? 1 : 0;

	daqDev().sendCmd(CMD_DBITOUT, portNum, (unsigned char) bitNum, bitVal);
}

unsigned long long DioUsbSsrxx::getCfg_PortLogic(unsigned int portNum)
{
	if(portNum >= mDioInfo.getNumPorts())
		throw UlException(ERR_BAD_PORT_INDEX);

	unsigned long long logic = 0; // non-invert

	unsigned char cmd = CMD_STATUS;
	unsigned short status = 0;

	daqDev().queryCmd(cmd, &status);

	std::bitset<16> statusMask(status);

	int bitIdx = portNum + mPortOffset + 4;

	if(statusMask[bitIdx] == 0)
		logic = 1;

	return logic;
}

unsigned long DioUsbSsrxx::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;

	unsigned char cmd = CMD_STATUS;
	unsigned short status = 0;

	daqDev().queryCmd(cmd, &status);

	std::bitset<16> statusMask(status);

	int bitIdx = portNum + mPortOffset;

	if(statusMask[bitIdx])
		mask.set();
	else
		mask.reset();

	return mask.to_ulong();
}

} /* namespace ul */
