/*
 * DioUsbErbxx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbErbxx.h"

namespace ul
{

DioUsbErbxx::DioUsbErbxx(const HidDaqDevice& daqDevice) : DioHidBase(daqDevice)
{
	if(daqDevice.getDeviceType() == DaqDeviceId::USB_ERB08)
	{
		mDioInfo.addPort(0, FIRSTPORTCL, 4, DPIOT_OUT);
		mDioInfo.addPort(1, FIRSTPORTCH, 4, DPIOT_OUT);

		mPortOffset = 2;
	}
	else
	{
		mDioInfo.addPort(0, FIRSTPORTA, 8, DPIOT_OUT);
		mDioInfo.addPort(1, FIRSTPORTB, 8, DPIOT_OUT);
		mDioInfo.addPort(2, FIRSTPORTCL, 4, DPIOT_OUT);
		mDioInfo.addPort(3, FIRSTPORTCH, 4, DPIOT_OUT);

		mPortOffset = 0;
	}

}

DioUsbErbxx::~DioUsbErbxx()
{

}

void DioUsbErbxx::initialize()
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

unsigned long long DioUsbErbxx::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned char portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	daqDev().queryCmd(CMD_DIN, portNum, &portValue);

	return portValue;
}

void DioUsbErbxx::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	unsigned char val = data;

	daqDev().sendCmd(CMD_DOUT, portNum, val);
}

bool DioUsbErbxx::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;

	unsigned char portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	daqDev().queryCmd(CMD_DBITIN, portNum, bitNum, &bitValue);

	return bitValue? true : false;
}

void DioUsbErbxx::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char portNum = mDioInfo.getPortNum(portType) + mPortOffset;

	unsigned char bitVal = bitValue ? 1 : 0;

	daqDev().sendCmd(CMD_DBITOUT, portNum, (unsigned char) bitNum, bitVal);
}

unsigned long long DioUsbErbxx::getCfg_PortLogic(unsigned int portNum)
{
	if(portNum >= mDioInfo.getNumPorts())
		throw UlException(ERR_BAD_PORT_INDEX);

	unsigned long long logic = 0; // non-invert

	unsigned char cmd = CMD_STATUS;
	unsigned short status = 0;

	daqDev().queryCmd(cmd, &status);

	std::bitset<16> statusMask(status);

	int bitIdx = portNum + mPortOffset;

	if(statusMask[bitIdx] == 0)
		logic = 1;

	return logic;
}

unsigned long DioUsbErbxx::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;

	mask.reset();

	return mask.to_ulong();
}

} /* namespace ul */
