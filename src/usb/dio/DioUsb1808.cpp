/*
 * DioUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsb1808.h"
#include "../daqi/DaqIUsb1808.h"
#include "../daqo/DaqOUsb1808.h"

namespace ul
{

DioUsb1808::DioUsb1808(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;
	mDioInfo.hasPacer(DD_INPUT, true);
	mDioInfo.hasPacer(DD_OUTPUT, true);
	mDioInfo.addPort(0, AUXPORT, 4, DPIOT_BITIO);

	mDioInfo.setScanFlags(DD_INPUT, 0);
	mDioInfo.setScanFlags(DD_OUTPUT, 0);

	mDioInfo.setScanOptions(DD_INPUT, SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mDioInfo.setScanOptions(DD_OUTPUT, SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mDioInfo.setTriggerTypes(DD_INPUT, TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);
	mDioInfo.setTriggerTypes(DD_OUTPUT, TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mDioInfo.setMinScanRate(DD_INPUT, minRate);
	mDioInfo.setMinScanRate(DD_OUTPUT, minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1808X)
	{
		mDioInfo.setMaxScanRate(DD_INPUT, 200000);
		mDioInfo.setMaxThroughput(DD_INPUT,200000);

		mDioInfo.setMaxScanRate(DD_OUTPUT, 500000);
		mDioInfo.setMaxThroughput(DD_OUTPUT, 500000);
	}
	else
	{
		mDioInfo.setMaxScanRate(DD_INPUT, 50000);
		mDioInfo.setMaxThroughput(DD_INPUT, 50000);

		mDioInfo.setMaxScanRate(DD_OUTPUT, 125000);
		mDioInfo.setMaxThroughput(DD_OUTPUT, 125000);
	}

	mDioInfo.setFifoSize(DD_INPUT, FIFO_SIZE_IN);
	mDioInfo.setFifoSize(DD_OUTPUT, FIFO_SIZE_OUT);
}

DioUsb1808::~DioUsb1808()
{

}

void DioUsb1808::initialize()
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

void DioUsb1808::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 0x000f;

	daqDev().sendCmd(CMD_DTRISTATE, dir, 0, NULL, 0);

	setPortDirection(portType, direction);
}

void DioUsb1808::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
		portDir.set(bitNum);

	unsigned char dir = portDir.to_ulong();

	daqDev().sendCmd(CMD_DTRISTATE, dir, 0, NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioUsb1808::dIn(DigitalPortType portType)
{
	unsigned char portValue = 0;

	check_DIn_Args(portType);

	daqDev().queryCmd(CMD_DPORT, 0, 0, &portValue, sizeof(portValue));

	return portValue;
}

void DioUsb1808::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short val = data;

	daqDev().sendCmd(CMD_DLATCH, val, 0, NULL, 0);
}

unsigned long DioUsb1808::readPortDirMask(unsigned int portNum) const
{
	unsigned char dirMask;

	daqDev().queryCmd(CMD_DTRISTATE, 0, 0, &dirMask, sizeof(dirMask));

	std::bitset<4> mask(dirMask);

	return mask.to_ulong();
}


bool DioUsb1808::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned char portValue = dIn(portType);

	std::bitset<4> bitset(portValue);

	return bitset[bitNum];
}

void DioUsb1808::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned char portValue = 0;

	daqDev().queryCmd(CMD_DLATCH, 0, 0, &portValue, sizeof(portValue));

	std::bitset<4> bitset(portValue);

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue = bitset.to_ulong();

	daqDev().sendCmd(CMD_DLATCH, portValue, 0, NULL, 0);
}

double DioUsb1808::dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[])
{
	check_DInScan_Args(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsb1808* daqIDev = dynamic_cast<DaqIUsb1808*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		DaqInChanDescriptor chanDescriptors;

		chanDescriptors.channel = AUXPORT;
		chanDescriptors.type = DAQI_DIGITAL;

		actualRate =  daqIDev->daqInScan(FT_DI, &chanDescriptors, 1, samplesPerPort, rate, options, (DaqInScanFlag) flags, data);
	}

	return actualRate;
}

double DioUsb1808::dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[])
{
	check_DOutScan_Args(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	double actualRate = 0;

	DaqOUsb1808* daqODev = dynamic_cast<DaqOUsb1808*>(mDaqDevice.daqODevice());

	if(daqODev)
	{
		DaqOutChanDescriptor chanDescriptors;

		chanDescriptors.channel = AUXPORT;
		chanDescriptors.type = DAQO_DIGITAL;

		actualRate =  daqODev->daqOutScan(FT_DO, &chanDescriptors, 1, samplesPerPort, rate, options, (DaqOutScanFlag) flags, data);
	}

	return actualRate;
}

UlError DioUsb1808::getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus)
{
	if(direction == SD_INPUT)
		return mDaqDevice.daqIDevice()->getStatus(FT_DI, status, xferStatus);
	else
		return mDaqDevice.daqODevice()->getStatus(FT_DO, status, xferStatus);
}

UlError DioUsb1808::waitUntilDone(ScanDirection direction, double timeout)
{
	if(direction == SD_INPUT)
		return mDaqDevice.daqIDevice()->waitUntilDone(FT_DI, timeout);
	else
		return mDaqDevice.daqODevice()->waitUntilDone(FT_DO, timeout);
}


void DioUsb1808::stopBackground(ScanDirection direction)
{
	if(direction == SD_INPUT)
		mDaqDevice.daqIDevice()->stopBackground(FT_DI);
	else
		mDaqDevice.daqODevice()->stopBackground(FT_DO);
}

ScanStatus DioUsb1808::getScanState(ScanDirection direction) const
{
	if(direction == SD_INPUT)
		return mDaqDevice.daqIDevice()->getScanState();
	else
		return mDaqDevice.daqODevice()->getScanState();
}

void DioUsb1808::check_SetTrigger_Args(ScanDirection direction, TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const
{
	if(trigChan != AUXPORT0)
		throw UlException(ERR_BAD_PORT_TYPE);

	DioDevice::check_SetTrigger_Args(direction, trigType, trigChan,  level, variance, retriggerCount);
}

} /* namespace ul */
