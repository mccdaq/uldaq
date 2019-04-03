
/*
 * DioUsbQuad08.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbQuad08.h"
#include "../UsbIotech.h"

namespace ul
{

DioUsbQuad08::DioUsbQuad08(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	mDioInfo.addPort(0, AUXPORT, 8, DPIOT_BITIO);
}

DioUsbQuad08::~DioUsbQuad08()
{

}

void DioUsbQuad08::initialize()
{
	try
	{
		initPortsDirectionMask();

		for(unsigned int portNum = 0; portNum < mDioInfo.getNumPorts(); portNum++)
			dConfigPort(mDioInfo.getPortType(portNum), DD_INPUT);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void DioUsbQuad08::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0x00ff;
	else
		dir = 0;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, p2Is82c55, UsbIotech::HWRegDioControl, NULL, 0);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, dir, UsbIotech::HWRegDioP2IO8Bit + 3, NULL, 0);

	setPortDirection(portType, direction);
}

void DioUsbQuad08::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
		portDir.set(bitNum);

	unsigned char dir = ~portDir.to_ulong();

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, p2Is82c55, UsbIotech::HWRegDioControl, NULL, 0);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, dir, UsbIotech::HWRegDioP2IO8Bit + 3, NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioUsbQuad08::dIn(DigitalPortType portType)
{
	unsigned short portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegDioP2IO8Bit + 2, (unsigned char*) &portValue, sizeof(portValue));

	return portValue;
}

void DioUsbQuad08::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portValue = (data & 0x00ff);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, portValue, UsbIotech::HWRegDioP2IO8Bit + 2, NULL, 0);
}

bool DioUsbQuad08::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	return bitset[bitNum];
}

void DioUsbQuad08::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned short portValue = dIn(portType);

	std::bitset<8> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, portValue, UsbIotech::HWRegDioP2IO8Bit + 2, NULL, 0);
}

unsigned long DioUsbQuad08::readPortDirMask(unsigned int portNum) const
{
	std::bitset<8> mask;
	mask.set();

	return mask.to_ulong();
}

} /* namespace ul */
