/*
 * DioDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioDevice.h"

#include "UlException.h"

namespace ul
{

DioDevice::DioDevice(const DaqDevice& daqDevice) : IoDevice(daqDevice), UlDioDevice()
{
	mDioConfig = new DioConfig(*this);

	mScanInState = SS_IDLE;
	mScanOutState = SS_IDLE;

	mDisableCheckDirection = false;

	memset(&mDiTrigCfg, 0, sizeof(mDiTrigCfg));
	mDiTrigCfg.type = TRIG_POS_EDGE;

	memset(&mDoTrigCfg, 0, sizeof(mDoTrigCfg));
	mDoTrigCfg.type = TRIG_POS_EDGE;
}

DioDevice::~DioDevice()
{
	if(mDioConfig != NULL)
	{
		delete mDioConfig;
		mDioConfig = NULL;
	}
}


void DioDevice::dConfigPort(DigitalPortType portType, DigitalDirection direction)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DioDevice::dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

unsigned long long DioDevice::dIn(DigitalPortType portType)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DioDevice::dOut(DigitalPortType portType, unsigned long long data)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DioDevice::dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DInArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int i = 0;

	for(unsigned int portNum = lowPortNum; portNum <=highPortNum; portNum++)
	{
		data[i] = dIn(mDioInfo.getPortType(portNum));
		i++;
	}
}

void DioDevice::dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	check_DOutArray_Args(lowPort, highPort, data);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int i = 0;
	for(unsigned int portNum = lowPortNum; portNum <= highPortNum; portNum++)
	{
		dOut(mDioInfo.getPortType(portNum), data[i]);
		i++;
	}
}

bool DioDevice::dBitIn(DigitalPortType portType, int bitNum)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DioDevice::dBitOut(DigitalPortType portType, int bitNum, bool bitValue)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

double DioDevice::dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[])
{
	throw UlException(ERR_BAD_DEV_TYPE);
}
double DioDevice::dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[])
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

UlError DioDevice::getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void  DioDevice::stopBackground(ScanDirection direction)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DioDevice::setTrigger(ScanDirection direction, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	check_SetTrigger_Args(direction, type, trigChan, level, variance, retriggerCount);

	if(direction == SD_INPUT)
	{
		mDiTrigCfg.type = type;
		mDiTrigCfg.trigChan = trigChan;
		mDiTrigCfg.level = level;
		mDiTrigCfg.variance = variance;
		mDiTrigCfg.retrigCount = retriggerCount;
	}
	else
	{
		mDoTrigCfg.type = type;
		mDoTrigCfg.trigChan = trigChan;
		mDoTrigCfg.level = level;
		mDoTrigCfg.variance = variance;
		mDoTrigCfg.retrigCount = retriggerCount;
	}
}

void DioDevice::dInSetTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	setTrigger(SD_INPUT, type, trigChan, level, variance, retriggerCount);
}

void DioDevice::dOutSetTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	setTrigger(SD_OUTPUT, type, trigChan, level, variance, retriggerCount);
}

UlError DioDevice::dInGetStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return getStatus(SD_INPUT, status, xferStatus);
}

UlError DioDevice::dOutGetStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return getStatus(SD_OUTPUT, status, xferStatus);
}

void DioDevice::dInStopBackground()
{
	stopBackground(SD_INPUT);
}

void DioDevice::dOutStopBackground()
{
	stopBackground(SD_OUTPUT);
}

void DioDevice::dClearAlarm(DigitalPortType portType, unsigned long long mask)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}


void DioDevice::initPortsDirectionMask()
{
	std::bitset<32> portDirectionMask;
	portDirectionMask.reset();

	unsigned int portDirMask = 0;
	mPortDirectionMask.clear();

	for(unsigned int portNum = 0; portNum < mDioInfo.getNumPorts(); portNum++)
	{
		portDirMask = readPortDirMask(portNum);

		std::bitset<32> portDirectionMask(portDirMask);

		mPortDirectionMask.push_back(portDirectionMask);
	}
}

TriggerConfig DioDevice::getTrigConfig(ScanDirection direction) const
{
	if(direction == SD_INPUT)
		return mDiTrigCfg;
	else
		return mDoTrigCfg;
}

void DioDevice::check_DConfigPort_Args(DigitalPortType portType, DigitalDirection direction)
{
	if(mDioInfo.isPortSupported(portType) == false)
		throw UlException(ERR_BAD_PORT_TYPE);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}


void DioDevice::check_DConfigBit_Args(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	if(mDioInfo.isPortSupported(portType) == false)
		throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int portNum = mDioInfo.getPortNum(portType);
	int bitCount = mDioInfo.getNumBits(portNum);

	if(bitCount <= bitNum)
		throw UlException(ERR_BAD_BIT_NUM);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DIn_Args(DigitalPortType portType)
{
	if(mDioInfo.isPortSupported(portType) == false)
		throw UlException(ERR_BAD_PORT_TYPE);

	/*unsigned int portNum = mDioInfo.getPortNum(portType);
	DigitalPortIoType  ioType = mDioInfo.getPortIoType(portNum);

	if(ioType == DPIOT_OUT)
		throw UlException(ERR_WRONG_DIG_CONFIG);*/

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DOut_Args(DigitalPortType portType, unsigned long long data)
{
	if(mDioInfo.isPortSupported(portType) == false)
		throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int portNum = mDioInfo.getPortNum(portType);
	unsigned int bitCount = mDioInfo.getNumBits(portNum);

	DigitalPortIoType ioType = mDioInfo.getPortIoType(portNum);

	if(ioType == DPIOT_IN)
		throw UlException(ERR_BAD_DIG_OPERATION);
	else if((ioType == DPIOT_IO || ioType == DPIOT_BITIO) && !mDisableCheckDirection)
	{
		if(mPortDirectionMask[portNum].any())
			throw UlException(ERR_WRONG_DIG_CONFIG);
	}

	unsigned long long maxVal = (1ULL << bitCount) - 1;

	if(data > maxVal)
		throw UlException(ERR_BAD_PORT_VAL);

	if(!mDaqDevice.isConnected())
			throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DInArray_Args(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	if(!mDioInfo.isPortSupported(lowPort) || !mDioInfo.isPortSupported(highPort))
			throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	if(lowPortNum > highPortNum )
			throw UlException(ERR_BAD_PORT_TYPE);

	/*for(unsigned int portNum = lowPortNum; portNum <=highPortNum; portNum++)
	{
		if(mDioInfo.getPortIoType(portNum) == DPIOT_OUT)
				throw UlException(ERR_WRONG_DIG_CONFIG);
	}*/

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DOutArray_Args(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	if(!mDioInfo.isPortSupported(lowPort) || !mDioInfo.isPortSupported(highPort))
			throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	if(lowPortNum > highPortNum )
		throw UlException(ERR_BAD_PORT_TYPE);

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	DigitalPortIoType ioType;
	int bitCount;
	unsigned long maxVal;
	int i = 0;

	for(unsigned int portNum = lowPortNum; portNum <=highPortNum; portNum++)
	{
		ioType = mDioInfo.getPortIoType(portNum);
		bitCount = mDioInfo.getNumBits(portNum);

		if(ioType == DPIOT_IN)
			throw UlException(ERR_BAD_DIG_OPERATION);
		else if((ioType == DPIOT_IO || ioType == DPIOT_BITIO) && !mDisableCheckDirection)
		{
			if(mPortDirectionMask[portNum].any())
				throw UlException(ERR_WRONG_DIG_CONFIG);
		}

		maxVal = (1 << bitCount) - 1;

		if(data[i] > maxVal)
			throw UlException(ERR_BAD_PORT_VAL);

		i++;
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}


void DioDevice::check_DBitIn_Args(DigitalPortType portType, int bitNum)
{
	if(mDioInfo.isPortSupported(portType) == false)
		throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int portNum = mDioInfo.getPortNum(portType);
	int bitCount = mDioInfo.getNumBits(portNum);

	if(bitNum >= bitCount)
		throw UlException(ERR_BAD_BIT_NUM);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DBitOut_Args(DigitalPortType portType, int bitNum)
{
	if(mDioInfo.isPortSupported(portType) == false)
		throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int portNum = mDioInfo.getPortNum(portType);
	int bitCount = mDioInfo.getNumBits(portNum);

	if(bitNum >= bitCount)
		throw UlException(ERR_BAD_BIT_NUM);

	DigitalPortIoType ioType = mDioInfo.getPortIoType(portNum);

	if(ioType == DPIOT_IN)
		throw UlException(ERR_BAD_DIG_OPERATION);
	else if((ioType == DPIOT_IO || ioType == DPIOT_BITIO) && !mDisableCheckDirection)
	{
		if(mPortDirectionMask[portNum][bitNum])
			throw UlException(ERR_WRONG_DIG_CONFIG);
	}

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DInScan_Args(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]) const
{
	if(!mDioInfo.isPortSupported(lowPort) || !mDioInfo.isPortSupported(highPort))
		throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int numOfScanPorts = highPortNum - lowPortNum + 1;

	if(!mDioInfo.hasPacer(DD_INPUT))
		throw UlException(ERR_BAD_DEV_TYPE);

	if(getScanState(SD_INPUT) == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(((options & SO_SINGLEIO) && (options & SO_BLOCKIO)) || ((options & SO_SINGLEIO) && (options & SO_BURSTIO)) || ((options & SO_BLOCKIO) && (options & SO_BURSTIO)))
		throw UlException(ERR_BAD_OPTION);

	if(lowPortNum > highPortNum )
		throw UlException(ERR_BAD_PORT_TYPE);

	/*for(unsigned int portNum = lowPortNum; portNum <=highPortNum; portNum++)
	{
		if(mDioInfo.getPortIoType(portNum) == DPIOT_OUT)
				throw UlException(ERR_WRONG_DIG_CONFIG);
	}*/

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(~mDioInfo.getScanOptions(DD_INPUT) & options)
		throw UlException(ERR_BAD_OPTION);

	if(~mDioInfo.getScanFlags(DD_INPUT) & flags)
		throw UlException(ERR_BAD_FLAG);

	double throughput = rate * numOfScanPorts;

	if(!(options & SO_EXTCLOCK))
	{
		if(((options & SO_BURSTIO) && (rate > mDioInfo.getDiMaxBurstRate() || throughput > mDioInfo.getDiMaxBurstThroughput())) || (!(options & SO_BURSTIO) && (rate > mDioInfo.getMaxScanRate(DD_INPUT) || throughput > mDioInfo.getMaxThroughput(DD_INPUT))) )
			throw UlException(ERR_BAD_RATE);
	}

	if(rate <= 0.0)
		throw UlException(ERR_BAD_RATE);

	if(samplesPerPort < mMinScanSampleCount)
		throw UlException(ERR_BAD_SAMPLE_COUNT);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_DOutScan_Args(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]) const
{
	if(!mDioInfo.isPortSupported(lowPort) || !mDioInfo.isPortSupported(highPort))
		throw UlException(ERR_BAD_PORT_TYPE);

	unsigned int lowPortNum = mDioInfo.getPortNum(lowPort);
	unsigned int highPortNum = mDioInfo.getPortNum(highPort);

	int numOfScanPorts = highPortNum - lowPortNum + 1;

	if(!mDioInfo.hasPacer(DD_OUTPUT))
		throw UlException(ERR_BAD_DEV_TYPE);

	if(getScanState(SD_OUTPUT) == SS_RUNNING)
		throw UlException(ERR_ALREADY_ACTIVE);

	if(((options & SO_SINGLEIO) && (options & SO_BLOCKIO)) || ((options & SO_SINGLEIO) && (options & SO_BURSTIO)) || ((options & SO_BLOCKIO) && (options & SO_BURSTIO)))
		throw UlException(ERR_BAD_OPTION);

	if(lowPortNum > highPortNum )
		throw UlException(ERR_BAD_PORT_TYPE);

	DigitalPortIoType ioType;
	for(unsigned int portNum = lowPortNum; portNum <=highPortNum; portNum++)
	{
		ioType = mDioInfo.getPortIoType(portNum);

		if(ioType == DPIOT_IN)
			throw UlException(ERR_BAD_DIG_OPERATION);
		else if((ioType == DPIOT_IO || ioType == DPIOT_BITIO) && !mDisableCheckDirection)
		{
			if(mPortDirectionMask[portNum].any())
				throw UlException(ERR_WRONG_DIG_CONFIG);
		}
	}

	if(data == NULL)
		throw UlException(ERR_BAD_BUFFER);

	if(~mDioInfo.getScanOptions(DD_OUTPUT) & options)
		throw UlException(ERR_BAD_OPTION);

	if(~mDioInfo.getScanFlags(DD_OUTPUT) & flags)
		throw UlException(ERR_BAD_FLAG);

	double throughput = rate * numOfScanPorts;

	if(!(options & SO_EXTCLOCK))
	{
		if(rate > mDioInfo.getMaxScanRate(DD_OUTPUT) || throughput > mDioInfo.getMaxThroughput(DD_OUTPUT))
			throw UlException(ERR_BAD_RATE);
	}

	if(rate <= 0.0)
		throw UlException(ERR_BAD_RATE);

	if(samplesPerPort < mMinScanSampleCount)
		throw UlException(ERR_BAD_SAMPLE_COUNT);

	if(!mDaqDevice.isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}

void DioDevice::check_SetTrigger_Args(ScanDirection direction, TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const
{
	DigitalDirection digitalDirection = DD_INPUT;
	if(direction == SD_OUTPUT)
		digitalDirection = DD_OUTPUT;

	if(mDioInfo.supportsTrigger(digitalDirection))
	{
		if(!(mDioInfo.getTriggerTypes(digitalDirection) & trigType))
			throw UlException(ERR_BAD_TRIG_TYPE);

		std::bitset<32> typeBitSet(trigType);

		if(typeBitSet.count() > 1)
			throw UlException(ERR_BAD_TRIG_TYPE);

		if(retriggerCount > 0 && !(mDioInfo.getScanOptions(digitalDirection) & SO_RETRIGGER))
			throw UlException(ERR_BAD_RETRIG_COUNT);
	}
	else
		throw UlException(ERR_BAD_DEV_TYPE);
}

std::bitset<32> DioDevice::getPortDirection(DigitalPortType portType) const
{
	unsigned int portNum = mDioInfo.getPortNum(portType);

	return mPortDirectionMask[portNum];
}

void DioDevice::setPortDirection(DigitalPortType portType, DigitalDirection direction)
{
	unsigned int portNum = mDioInfo.getPortNum(portType);
	unsigned int bitCount = mDioInfo.getNumBits(portNum);

	if(direction == DD_OUTPUT)
		mPortDirectionMask[portNum].reset();
	else
	{
		for(unsigned int bit = 0; bit < bitCount; bit++)
		mPortDirectionMask[portNum].set(bit, true);
	}
}

void DioDevice::setBitDirection(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	unsigned int portNum = mDioInfo.getPortNum(portType);

	if(direction == DD_OUTPUT)
		mPortDirectionMask[portNum].reset(bitNum);
	else
		mPortDirectionMask[portNum].set(bitNum);
}

void DioDevice::setScanState(ScanDirection direction, ScanStatus state)
{
	if(direction == SD_INPUT)
		mScanInState = state;
	else
		mScanOutState = state;
}

ScanStatus DioDevice::getScanState(ScanDirection direction) const
{
	if(direction == SD_INPUT)
		return mScanInState;
	else
		return mScanOutState;
}


void DioDevice::stopBackground()
{
	if(getScanState(SD_INPUT) == SS_RUNNING)
		stopBackground(SD_INPUT);

	if(getScanState(SD_OUTPUT) == SS_RUNNING)
		stopBackground(SD_OUTPUT);
}

ScanStatus DioDevice::getScanState() const
{
	ScanStatus scanState = SS_IDLE;

	if(getScanState(SD_INPUT) == SS_RUNNING || getScanState(SD_OUTPUT) == SS_RUNNING)
		scanState = SS_RUNNING;

	return scanState;
}

UlError DioDevice::wait(ScanDirection direction, WaitType waitType, long long waitParam, double timeout)
{
	UlError err = ERR_NO_ERROR;

	if(waitType == WAIT_UNTIL_DONE)
	{
		err = waitUntilDone(direction, timeout);
	}

	return err;
}

UlError DioDevice::waitUntilDone(ScanDirection direction, double timeout)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

//////////////////////          Configuration functions          /////////////////////////////////

unsigned long long DioDevice::getCfg_PortDirectionMask(unsigned int portNum) const
{
	mDaqDevice.checkConnection();

	unsigned long long dirMask = 0;

	if(portNum < mDioInfo.getNumPorts())
	{
		DigitalPortType portType = mDioInfo.getPortType(portNum);
		std::bitset<32> bitsetMask = getPortDirection(portType);

		unsigned int portNum = mDioInfo.getPortNum(portType);
		unsigned int bitCount = mDioInfo.getNumBits(portNum);

		unsigned long long bits = (1ULL << bitCount) - 1;
		dirMask = bitsetMask.flip().to_ulong() & bits;
	}
	else
		throw UlException(ERR_BAD_PORT_INDEX);

	return dirMask;
}

void DioDevice::setCfg_PortInitialOutputVal(unsigned int portNum, unsigned long long val)
{
	DigitalPortType portType = mDioInfo.getPortType(portNum);

	if(portType)
	{
		DigitalPortIoType portIoType =  mDioInfo.getPortIoType(portNum);

		if(portIoType == DPIOT_IO || portIoType == DPIOT_BITIO)
		{
			bool currenState = mDisableCheckDirection;
			mDisableCheckDirection = true;

			try
			{
				dOut(portType, val);
			}
			catch(UlException& e)
			{
				throw UlException(e.getError());
			}

			mDisableCheckDirection = currenState;
		}
		else
			throw UlException(ERR_CONFIG_NOT_SUPPORTED);
	}
	else
		throw UlException(ERR_BAD_PORT_INDEX);
}

void DioDevice::setCfg_PortIsoMask(unsigned int portNum, unsigned long long mask)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

unsigned long long DioDevice::getCfg_PortIsoMask(unsigned int portNum)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

unsigned long long DioDevice::getCfg_PortLogic(unsigned int portNum)
{
	throw UlException(ERR_CONFIG_NOT_SUPPORTED);
}

} /* namespace ul */
