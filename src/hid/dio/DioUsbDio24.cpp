/*
 * DioUsbDio24.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbDio24.h"
#include "../UsbDio24.h"

namespace ul
{

DioUsbDio24::DioUsbDio24(const HidDaqDevice& daqDevice) : DioHidBase(daqDevice)
{
	mDioInfo.addPort(0, FIRSTPORTA, 8, DPIOT_IO);
	mDioInfo.addPort(1, FIRSTPORTB, 8, DPIOT_IO);
	mDioInfo.addPort(2, FIRSTPORTCL, 4, DPIOT_IO);
	mDioInfo.addPort(3, FIRSTPORTCH, 4, DPIOT_IO);

	mPortCLVal = 0;
	mPortCHVal = 0;
}

DioUsbDio24::~DioUsbDio24()
{

}

void DioUsbDio24::initialize()
{
	try
	{
		initPortsDirectionMask();

		for(unsigned int portNum = 0; portNum < mDioInfo.getNumPorts(); portNum++)
			dConfigPort(mDioInfo.getPortType(portNum), DD_INPUT);

		mPortCLVal = 0;
		mPortCHVal = 0;
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void DioUsbDio24::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned char buffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	buffer[1] = CMD_DCONFIG;
	buffer[2] = getPortCode(portType);
	buffer[3] = (direction == DD_INPUT)? 1 : 0;

	size_t length = sizeof(buffer);

	daqDev().sendRawCmd(buffer, &length);

	setPortDirection(portType, direction);

	if(direction == DD_INPUT)
	{
		if(portType == FIRSTPORTCL)
			mPortCLVal = 0;
		else if(portType == FIRSTPORTCH)
			mPortCHVal = 0;
	}
}

unsigned long long DioUsbDio24::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	unsigned char outBuffer[UsbDio24::MAX_PACKET_SIZE] = {0};
	unsigned char inBuffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	outBuffer[1] = CMD_DIN;
	outBuffer[2] = getPortCode(portType);

	size_t outBufLen = sizeof(outBuffer);
	size_t inBufLen = sizeof(inBuffer);

	daqDev().queryRawCmd(outBuffer, outBufLen, inBuffer, &inBufLen);

	portValue = inBuffer[0];

	if(portType == FIRSTPORTCL)
		portValue &= 0x0f;
	else if(portType == FIRSTPORTCH)
		portValue >>= 4;

	return portValue;
}

void DioUsbDio24::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned char val = data & 0xff;

	if(portType == FIRSTPORTCL)
	{
		mPortCLVal = val & 0x0f;
		val = val | (mPortCHVal << 4);
	}
	else if(portType == FIRSTPORTCH)
	{
		mPortCHVal = val & 0x0f;
		val = (val << 4) | mPortCLVal;
	}

	unsigned char buffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	buffer[1] = CMD_DOUT;
	buffer[2] = getPortCode(portType);
	buffer[3] = val;

	size_t length = sizeof(buffer);

	daqDev().sendRawCmd(buffer, &length);
}

bool DioUsbDio24::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char bitValue = 0;

	unsigned char outBuffer[UsbDio24::MAX_PACKET_SIZE] = {0};
	unsigned char inBuffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	unsigned char bit = bitNum;
	if(portType == FIRSTPORTCH)
		bit +=4;

	outBuffer[1] = CMD_BITIN;
	outBuffer[2] = getPortCode(portType);
	outBuffer[3] = bit;

	size_t outBufLen = sizeof(outBuffer);
	size_t inBufLen = sizeof(inBuffer);

	daqDev().queryRawCmd(outBuffer, outBufLen, inBuffer, &inBufLen);

	bitValue = inBuffer[0] & 0x01;

	return bitValue? true : false;
}


void DioUsbDio24::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char bit = bitNum;
	if(portType == FIRSTPORTCH)
		bit +=4;

	unsigned char bitVal = bitValue ? 1 : 0;

	unsigned char buffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	buffer[1] = CMD_BITOUT;
	buffer[2] = getPortCode(portType);
	buffer[3] = bit;
	buffer[4] = bitVal;

	size_t length = sizeof(buffer);

	daqDev().sendRawCmd(buffer, &length);
}

unsigned char DioUsbDio24::getPortCode(DigitalPortType portType) const
{
	unsigned char code = 0;

	switch(portType)
	{
	case FIRSTPORTA:
		code = 1;
		break;
	case FIRSTPORTB:
		code = 4;
		break;
	case FIRSTPORTCL:
		code = 8;
		break;
	case FIRSTPORTCH:
		code = 2;
		break;
	default:
		code = 0;
		break;
	}

	return code;
}

void DioUsbDio24::setCfg_PortInitialOutputVal(unsigned int portNum, unsigned long long val)
{
	if(daqDev().getDeviceType() == DaqDeviceId::USB_1024LS || daqDev().getDeviceType() == DaqDeviceId::USB_DIO24)
		throw UlException(ERR_CONFIG_NOT_SUPPORTED);

	DioDevice::setCfg_PortInitialOutputVal(portNum, val);
}

unsigned long DioUsbDio24::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;
	mask.set();

	return mask.to_ulong();
}

} /* namespace ul */
