/*
 * DioUsbDio32hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbDio32hs.h"

namespace ul
{

DioUsbDio32hs::DioUsbDio32hs(const UsbDaqDevice& daqDevice) : DioUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;
	mDioInfo.hasPacer(DD_INPUT, true);
	mDioInfo.hasPacer(DD_OUTPUT, true);
	mDioInfo.addPort(0, AUXPORT0, 16, DPIOT_BITIO);
	mDioInfo.addPort(1, AUXPORT1, 16, DPIOT_BITIO);

	mDioInfo.setScanFlags(DD_INPUT, 0);
	mDioInfo.setScanFlags(DD_OUTPUT, 0);

	mDioInfo.setScanOptions(DD_INPUT, SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mDioInfo.setScanOptions(DD_OUTPUT, SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mDioInfo.setTriggerTypes(DD_INPUT, TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);
	mDioInfo.setTriggerTypes(DD_OUTPUT, TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mDioInfo.setMinScanRate(DD_INPUT, minRate);
	mDioInfo.setMinScanRate(DD_OUTPUT, minRate);

	mDioInfo.setMaxScanRate(DD_INPUT, 8000000);
	mDioInfo.setMaxThroughput(DD_INPUT, 8000000);

	mDioInfo.setMaxScanRate(DD_OUTPUT, 8000000);
	mDioInfo.setMaxThroughput(DD_OUTPUT, 8000000);

	mDioInfo.setFifoSize(DD_INPUT, FIFO_SIZE);
	mDioInfo.setFifoSize(DD_OUTPUT, FIFO_SIZE);

	mDInScanDev = new UsbDInScan(daqDevice);
	mDOutScanDev = new UsbDOutScan(daqDevice);
}

DioUsbDio32hs::~DioUsbDio32hs()
{
	if(mDInScanDev)
		delete mDInScanDev;

	mDInScanDev = NULL;

	if(mDOutScanDev)
		delete mDOutScanDev;

	mDOutScanDev = NULL;
}

void DioUsbDio32hs::initialize()
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

void DioUsbDio32hs::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	check_DConfigPort_Args(portType, direction);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	unsigned short dir;

	if (direction == DD_OUTPUT)
		dir = 0;
	else
		dir = 0xffff;

	daqDev().sendCmd(CMD_DTRISTATE, dir, portNum, NULL, 0);

	setPortDirection(portType, direction);
}

void DioUsbDio32hs::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	check_DConfigBit_Args(portType, bitNum, direction);

	unsigned short portNum = mDioInfo.getPortNum(portType);

	std::bitset<32> portDir = getPortDirection(portType);

	if(direction == DD_OUTPUT)
		portDir.reset(bitNum);
	else
		portDir.set(bitNum);

	unsigned short dir = Endian::cpu_to_le_ui16(portDir.to_ulong());

	daqDev().sendCmd(CMD_DTRISTATE, dir, portNum, NULL, 0);

	setBitDirection(portType, bitNum, direction);
}

unsigned long long DioUsbDio32hs::dIn(DigitalPortType portType)
{
	check_DIn_Args(portType);

	unsigned short portValue[2] = {0, 0};

	unsigned short portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DPORT, 0, 0, (unsigned char*) &portValue, sizeof(portValue));

	unsigned short val = Endian::le_ui16_to_cpu(portValue[portNum]);

	return val;
}

void DioUsbDio32hs::dOut(DigitalPortType portType, unsigned long long data)
{
	check_DOut_Args(portType, data);

	unsigned short portValue[2] = {0, 0};

	unsigned short portNum = mDioInfo.getPortNum(portType);

	portValue[portNum] = Endian::cpu_to_le_ui16(data);

	daqDev().sendCmd(CMD_DLATCH, 0, portNum, (unsigned char*) &portValue, sizeof(portValue));
}

void DioUsbDio32hs::dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DInArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	unsigned short portValue[2] = {0, 0};

	daqDev().queryCmd(CMD_DPORT, 0, 0, (unsigned char*) &portValue, sizeof(portValue));

	int i = 0;

	for(unsigned int portNum = lowPortNum; portNum <=highPortNum; portNum++)
	{
		data[i] = Endian::le_ui16_to_cpu(portValue[portNum]);
		i++;
	}
}

void DioUsbDio32hs::dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DOutArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	unsigned short portValue[2] = {0, 0};

	int i = 0;
	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		portValue[portNum] = Endian::cpu_to_le_ui16(data[i]);
		i++;
	}

	int ports = lowPortNum;
	if (i > 1)
		ports = 2;

	daqDev().sendCmd(CMD_DLATCH, 0, ports, (unsigned char*) &portValue, sizeof(portValue));
}

unsigned long DioUsbDio32hs::readPortDirMask(unsigned int portNum) const
{
	unsigned short dirMask;

	daqDev().queryCmd(CMD_DTRISTATE, 0, portNum, (unsigned char*) &dirMask, sizeof(dirMask));

	std::bitset<16> mask(Endian::le_ui16_to_cpu(dirMask));

	return mask.to_ulong();
}


bool DioUsbDio32hs::dBitIn(DigitalPortType portType, int bitNum)
{
	check_DBitIn_Args(portType, bitNum);

	unsigned short portValue = dIn(portType);

	std::bitset<16> bitset(portValue);

	return bitset[bitNum];
}

void DioUsbDio32hs::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	check_DBitOut_Args(portType, bitNum);

	unsigned short portValue[2] = {0, 0};

	unsigned short portNum = mDioInfo.getPortNum(portType);

	daqDev().queryCmd(CMD_DPORT, 0, 0, (unsigned char*) &portValue, sizeof(portValue));

	std::bitset<16> bitset(Endian::le_ui16_to_cpu(portValue[portNum]));

	if(bitValue)
		bitset.set(bitNum);
	else
		bitset.reset(bitNum);

	portValue[portNum] = Endian::cpu_to_le_ui16(bitset.to_ulong());

	daqDev().sendCmd(CMD_DLATCH, 0, portNum, (unsigned char*) &portValue, sizeof(portValue));

}

double DioUsbDio32hs::dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[])
{
	check_DInScan_Args(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	TriggerConfig trigCfg = getTrigConfig(SD_INPUT);

	if((trigCfg.type & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW)) && (options & SO_RETRIGGER))
		throw UlException(ERR_BAD_RETRIG_TRIG_TYPE);

	double actualRate =  mDInScanDev->dInScan(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	return actualRate;
}
double DioUsbDio32hs::dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[])
{
	check_DOutScan_Args(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	TriggerConfig trigCfg = getTrigConfig(SD_OUTPUT);

	if((trigCfg.type & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW)) && (options & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_TRIG_TYPE);

	double actualRate =  mDOutScanDev->dOutScan(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	return actualRate;
}

UlError DioUsbDio32hs::getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus)
{
	if(direction == SD_INPUT)
		return mDInScanDev->getInputStatus(status, xferStatus);
	else
		return mDOutScanDev->getOutputStatus(status, xferStatus);
}

UlError DioUsbDio32hs::waitUntilDone(ScanDirection direction, double timeout)
{
	if(direction == SD_INPUT)
		return mDInScanDev->waitUntilDone(timeout);
	else
		return mDOutScanDev->waitUntilDone(timeout);
}


void DioUsbDio32hs::stopBackground(ScanDirection direction)
{
	if(direction == SD_INPUT)
		mDInScanDev->stopBackground();
	else
		mDOutScanDev->stopBackground();
}

void DioUsbDio32hs::check_SetTrigger_Args(ScanDirection direction, TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const
{
	if(trigType & (TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW))
	{
		if(trigChan != AUXPORT0 && trigChan != AUXPORT1)
			throw UlException(ERR_BAD_PORT_TYPE);

		if(retriggerCount != 0) // retrigger is not supported with pattern trigger
			throw UlException(ERR_BAD_RETRIG_COUNT);
	}

	DioDevice::check_SetTrigger_Args(direction, trigType, trigChan,  level, variance, retriggerCount);
}

} /* namespace ul */
