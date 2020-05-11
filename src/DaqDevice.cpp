/*
 * DaqDeviceInternal.cpp
 *
 *     Author: Measurement Computing Corporation
 */
#include <sstream>

#include "DaqDevice.h"
#include "DaqDeviceManager.h"
#include "utility/EuScale.h"
#include "utility/UlLock.h"


#include "AiDevice.h"
#include "AoDevice.h"
#include "DioDevice.h"
#include "CtrDevice.h"
#include "TmrDevice.h"
#include "DaqIDevice.h"
#include "DaqODevice.h"
#include "DaqEventHandler.h"
#include "UlException.h"

namespace ul
{
pthread_mutex_t DaqDevice::mDeviceNumberMutex = PTHREAD_MUTEX_INITIALIZER;
unsigned long long DaqDevice::mNextAvailableDeviceNumber = 1;

DaqDevice::DaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor): mDaqDeviceDescriptor(daqDeviceDescriptor), mConnected(false),
		mAiDevice(NULL), mAoDevice(NULL), mDioDevice(NULL), mCtrDevice(NULL), mTmrDevice(NULL), mDaqIDevice(NULL), mDaqODevice(NULL)
{
	mEventHandler = new DaqEventHandler(*this);
	mDaqDeviceConfig = new DaqDeviceConfig(*this);
	mDaqDeviceInfo.setProductId(daqDeviceDescriptor.productId);

	mMinRawFwVersion = 0;

	mRawFwVersion = 0;
	mRawFpgaVersion = 0;
	mRawRadioVersion = 0;
	mRawFwMeasurementVersion = 0;
	mRawFwExpMeasurementVersion = 0;

	mMemUnlockAddr = -1;
	mMemUnlockCode = 0;

	mCurrentSuspendCount = 0;

	mHasExp = false;

	pthread_mutex_lock(&mDeviceNumberMutex);
	mDeviceNumber = mNextAvailableDeviceNumber;
	mNextAvailableDeviceNumber++;
	pthread_mutex_unlock(&mDeviceNumberMutex);

	UlLock::initMutex(mDeviceMutex, PTHREAD_MUTEX_RECURSIVE);
}

DaqDevice::~DaqDevice()
{
	if(mAiDevice != NULL)
	{
		delete mAiDevice;
		mAiDevice = NULL;
	}

	if(mAoDevice != NULL)
	{
		delete mAoDevice;
		mAoDevice = NULL;
	}

	if(mDioDevice != NULL)
	{
		delete mDioDevice;
		mDioDevice = NULL;
	}

	if(mCtrDevice != NULL)
	{
		delete mCtrDevice;
		mCtrDevice = NULL;
	}

	if(mTmrDevice != NULL)
	{
		delete mTmrDevice;
		mTmrDevice = NULL;
	}

	if(mDaqIDevice != NULL)
	{
		delete mDaqIDevice;
		mDaqIDevice = NULL;
	}

	if(mDaqODevice != NULL)
	{
		delete mDaqODevice;
		mDaqODevice = NULL;
	}

	if(mDaqDeviceConfig != NULL)
	{
		delete mDaqDeviceConfig;
		mDaqDeviceConfig = NULL;
	}

	if(mEventHandler !=NULL)
	{
		delete mEventHandler;
		mEventHandler = NULL;
	}

	DaqDeviceManager::removeFromCreatedList(mDeviceNumber);

	UlLock::destroyMutex(mDeviceMutex);
}

DaqDeviceDescriptor DaqDevice::getDescriptor() const
{
	return mDaqDeviceDescriptor;
}

void DaqDevice::disconnect()
{

	mEventHandler->stop();

	disconnectIoDevices();

	mConnected = false;
}

double DaqDevice::getClockFreq() const
{
	return mDaqDeviceInfo.getClockFreq();
}
void DaqDevice::addMemRegion(MemRegion memRegionType, unsigned long long address, unsigned long long size, long long accessTypes)
{
	mDaqDeviceInfo.memInfo()->addMemRegion(memRegionType, address, size, (MemAccessType) accessTypes);
}

void DaqDevice::setAiDevice(AiDevice* aiDevice)
{
	mAiDevice = aiDevice;

	if(aiDevice != NULL)
		mDaqDeviceInfo.hasAiDevice(true);

}

AiDevice* DaqDevice::aiDevice() const
{
	return mAiDevice;
}

UlAiDevice& DaqDevice::getAiDevice() const
{
	return *mAiDevice;
}

void DaqDevice::setAoDevice(AoDevice* aoDevice)
{
	mAoDevice = aoDevice;

	if(aoDevice != NULL)
		mDaqDeviceInfo.hasAoDevice(true);
}

AoDevice* DaqDevice::aoDevice() const
{
	return mAoDevice;
}

UlAoDevice& DaqDevice::getAoDevice() const
{
	return *mAoDevice;
}

void DaqDevice::setDioDevice(DioDevice* dioDevice)
{
	mDioDevice = dioDevice;

	if(dioDevice != NULL)
		mDaqDeviceInfo.hasDioDevice(true);
}

DioDevice* DaqDevice::dioDevice() const
{
	return mDioDevice;
}

UlDioDevice& DaqDevice::getDioDevice() const
{
	return *mDioDevice;
}

UlCtrDevice& DaqDevice::getCtrDevice() const
{
	return *mCtrDevice;
}
void DaqDevice::setCtrDevice(CtrDevice* ctrDevice)
{
	mCtrDevice = ctrDevice;

	if(ctrDevice != NULL)
		mDaqDeviceInfo.hasCtrDevice(true);
}

CtrDevice* DaqDevice::ctrDevice() const
{
	return mCtrDevice;
}

UlTmrDevice& DaqDevice::getTmrDevice() const
{
	return *mTmrDevice;
}
void DaqDevice::setTmrDevice(TmrDevice* tmrDevice)
{
	mTmrDevice = tmrDevice;

	if(tmrDevice != NULL)
		mDaqDeviceInfo.hasTmrDevice(true);
}

TmrDevice* DaqDevice::tmrDevice() const
{
	return mTmrDevice;
}


UlDaqIDevice& DaqDevice::getDaqIDevice() const
{
	return *mDaqIDevice;
}
void DaqDevice::setDaqIDevice(DaqIDevice* daqIDevice)
{
	mDaqIDevice = daqIDevice;

	if(daqIDevice != NULL)
		mDaqDeviceInfo.hasDaqIDevice(true);
}

DaqIDevice* DaqDevice::daqIDevice() const
{
	return mDaqIDevice;
}

UlDaqODevice& DaqDevice::getDaqODevice() const
{
	return *mDaqODevice;
}
void DaqDevice::setDaqODevice(DaqODevice* daqODevice)
{
	mDaqODevice = daqODevice;

	if(daqODevice != NULL)
		mDaqDeviceInfo.hasDaqODevice(true);
}

DaqODevice* DaqDevice::daqODevice() const
{
	return mDaqODevice;
}

DaqEventHandler* DaqDevice::eventHandler() const
{
	return mEventHandler;
}

void DaqDevice::initializeIoDevices()
{

	if(mAiDevice != NULL)
		mAiDevice->initialize();

	if(mAoDevice != NULL)
		mAoDevice->initialize();

	if(mDioDevice != NULL)
		mDioDevice->initialize();

	if(mCtrDevice != NULL)
		mCtrDevice->initialize();

	if(mTmrDevice != NULL)
		mTmrDevice->initialize();

	if(mDaqIDevice != NULL)
		mDaqIDevice->initialize();

	if(mDaqODevice != NULL)
		mDaqODevice->initialize();
}



void DaqDevice::reconfigureIoDevices()
{
	if(mAiDevice != NULL)
		mAiDevice->reconfigure();

	if(mAoDevice != NULL)
		mAoDevice->reconfigure();

	if(mDioDevice != NULL)
		mDioDevice->reconfigure();

	if(mCtrDevice != NULL)
		mCtrDevice->reconfigure();

	if(mTmrDevice != NULL)
		mTmrDevice->reconfigure();

	if(mDaqIDevice != NULL)
		mDaqIDevice->reconfigure();

	if(mDaqODevice != NULL)
		mDaqODevice->reconfigure();
}

void DaqDevice::disconnectIoDevices()
{
	try
	{
		if(mAiDevice != NULL)
			mAiDevice->disconnect();

		if(mAoDevice != NULL)
			mAoDevice->disconnect();

		if(mDioDevice != NULL)
			mDioDevice->disconnect();

		if(mCtrDevice != NULL)
			mCtrDevice->disconnect();

		if(mDaqIDevice != NULL)
			mDaqIDevice->disconnect();

		if(mDaqODevice != NULL)
			mDaqODevice->disconnect();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

bool DaqDevice::isScanRunning() const
{
	bool running = false;

	if(mAiDevice != NULL && mAiDevice->getScanState() == SS_RUNNING)
		running = true;
	else  if(mAoDevice != NULL && mAoDevice->getScanState() == SS_RUNNING)
		running = true;
	else if(mDioDevice != NULL && mDioDevice->getScanState(SD_INPUT) == SS_RUNNING)
		running = true;
	else if(mDioDevice != NULL && mDioDevice->getScanState(SD_OUTPUT) == SS_RUNNING)
		running = true;
	else if(mCtrDevice != NULL && mCtrDevice->getScanState() == SS_RUNNING)
		running = true;
	else if(mDaqIDevice != NULL && mDaqIDevice->getScanState() == SS_RUNNING)
		running = true;
	else if(mDaqODevice != NULL && mDaqODevice->getScanState() == SS_RUNNING)
			running = true;

	return running;
}

bool DaqDevice::isScanRunning(FunctionType functionType) const
{
	bool running = false;

	IoDevice* ioDevice = NULL;

	switch(functionType)
	{
	case FT_AI:
		ioDevice = mAiDevice;
		break;
	case FT_AO:
		ioDevice = mAoDevice;
		break;
	case FT_DI:
	case FT_DO:
		ioDevice = mDioDevice;
		break;
	case FT_CTR:
		ioDevice = mCtrDevice;
		break;
	case FT_DAQI:
		ioDevice = mDaqIDevice;
		break;
	case FT_DAQO:
		ioDevice = mDaqODevice;
		break;

	default:
		break;
	}

	if(ioDevice != NULL && ioDevice->getScanState() == SS_RUNNING)
		running = true;

	return running;
}

void DaqDevice::stopBackground(FunctionType functionType) const
{
	IoDevice* ioDevice = NULL;

	switch(functionType)
	{
	case FT_AI:
		ioDevice = mAiDevice;
		break;
	case FT_AO:
		ioDevice = mAoDevice;
		break;
	case FT_DI:
	case FT_DO:
		ioDevice = mDioDevice;
		break;
	case FT_CTR:
		ioDevice = mCtrDevice;
		break;
	case FT_DAQI:
		ioDevice = mDaqIDevice;
		break;
	case FT_DAQO:
		ioDevice = mDaqODevice;
		break;
	default:
		break;
	}

	if(ioDevice != NULL)
		ioDevice->stopBackground();
	else
		std::cout << "########## stopBackground not implemented" << std::endl;

}

IoDevice* DaqDevice::getIoDevice(FunctionType functionType) const
{
	IoDevice* ioDevice = NULL;

	switch(functionType)
	{
	case FT_AI:
		ioDevice = mAiDevice;
		break;
	case FT_AO:
		ioDevice = mAoDevice;
		break;
	case FT_DI:
	case FT_DO:
		ioDevice = mDioDevice;
		break;
	case FT_CTR:
		ioDevice = mCtrDevice;
		break;
	case FT_TMR:
		ioDevice = mTmrDevice;
		break;
	case FT_DAQI:
		ioDevice = mDaqIDevice;
		break;
	case FT_DAQO:
		ioDevice = mDaqODevice;
		break;

	default:
		break;
	}

	if(ioDevice == NULL)
		std::cout << "########## getIoDevice not implemented" << std::endl;

	return ioDevice;

}

TriggerConfig DaqDevice::getTriggerConfig(FunctionType functionType) const
{
	TriggerConfig trigCfg = {(TriggerType) 0, 0, 0, 0, 0};

	if(functionType == FT_AI && mAiDevice)
		trigCfg = mAiDevice->getTrigConfig();
	else if(functionType == FT_AO && mAiDevice)
		trigCfg = mAoDevice->getTrigConfig();
	else if(functionType == FT_DI && mDioDevice)
		trigCfg = mDioDevice->getTrigConfig(SD_INPUT);
	else if(functionType == FT_DO && mDioDevice)
		trigCfg = mDioDevice->getTrigConfig(SD_OUTPUT);
	else if(functionType == FT_CTR && mCtrDevice)
		trigCfg = mCtrDevice->getTrigConfig();
	else if(functionType == FT_TMR && mTmrDevice)
		trigCfg = mTmrDevice->getTrigConfig();
	else if(functionType == FT_DAQI && mDaqIDevice)
		trigCfg = mDaqIDevice->getTrigConfig();
	else if(functionType == FT_DAQO && mDaqODevice)
		trigCfg = mDaqODevice->getTrigConfig();

	return trigCfg;
}

void DaqDevice::checkConnection() const
{
	if(!mConnected)
	{
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
	}
}

void DaqDevice::getEuScaling(Range range, double &scale, double &offset) const
{
	EuScale::getEuScaling(range, scale, offset);
}

void DaqDevice::flashLed(int flashCount) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqDevice::connectionCode(long long code)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

//////////////////////          Configuration functions          /////////////////////////////////

void DaqDevice::getCfg_FwVersionStr(DevVersionType verType, char* fwVerStr, unsigned int* maxStrLen) const
{
	unsigned short rawFwVer = mRawFwVersion;

	if(verType ==DEV_VER_FW_MEASUREMENT)
		rawFwVer = mRawFwMeasurementVersion;
	else if(verType ==DEV_VER_FW_MEASUREMENT_EXP)
		rawFwVer = mRawFwExpMeasurementVersion;

	if(fwVerStr)
		fwVerStr[0] = '\0';

	std::stringstream stream;
	stream << std::hex << rawFwVer;
	std::string verStr( stream.str());

	while(verStr.length() < 3)
		verStr.insert(verStr.begin(), '0');

	// fw version is stored in two bytes (Each nibble represents one digit)
	verStr.insert(verStr.end() - 2, '.');

	if(rawFwVer == 0)
		verStr = "";

	unsigned int len = verStr.size() + 1;

	if (len <= *maxStrLen)
	{
		memcpy(fwVerStr, verStr.c_str(), len);
		*maxStrLen = len;
	}
	else
	{
		*maxStrLen = len;
		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}

void DaqDevice::getCfg_FpgaVersionStr(char* fpgaVerStr, unsigned int* maxStrLen) const
{
	if(fpgaVerStr)
		fpgaVerStr[0] = '\0';

	std::stringstream stream;
	stream << std::hex << mRawFpgaVersion;
	std::string verStr( stream.str());

	while(verStr.length() < 3)
		verStr.insert(verStr.begin(), '0');

	// fpga version is stored in two bytes (Each nibble represents one digit)
	verStr.insert(verStr.end() - 2, '.');

	if(mRawFpgaVersion == 0)
		verStr = "";

	unsigned int len = verStr.size() + 1;

	if (len <= *maxStrLen)
	{
		memcpy(fpgaVerStr, verStr.c_str(), len);
		*maxStrLen = len;
	}
	else
	{
		*maxStrLen = len;
		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}

void DaqDevice::getCfg_RadioVersionStr(char* fpgaVerStr, unsigned int* maxStrLen) const
{
	if(fpgaVerStr)
		fpgaVerStr[0] = '\0';

	std::stringstream stream;
	stream << std::hex << mRawRadioVersion;
	std::string verStr( stream.str());

	while(verStr.length() < 3)
		verStr.insert(verStr.begin(), '0');

	//  radio version is stored in two bytes (Each nibble represents one digit)
	verStr.insert(verStr.end() - 2, '.');

	if(mRawRadioVersion == 0)
		verStr = "";

	unsigned int len = verStr.size() + 1;

	if (len <= *maxStrLen)
	{
		memcpy(fpgaVerStr, verStr.c_str(), len);
		*maxStrLen = len;
	}
	else
	{
		*maxStrLen = len;
		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}

long long DaqDevice::getCfg_ConnectionCode() const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqDevice::setCfg_ConnectionCode(long long code)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

long long DaqDevice::getCfg_MemUnlockCode() const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqDevice::setCfg_MemUnlockCode(long long code)
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqDevice::setCfg_Reset()
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

void DaqDevice::getCfg_IpAddress(char* address, unsigned int* maxStrLen) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}
void DaqDevice::getCfg_NetIfcName(char* ifcName, unsigned int* maxStrLen) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}


int DaqDevice::memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

int DaqDevice::memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

int DaqDevice::memRead(MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count);

	return memRead(MT_EEPROM, memRegionType, address, buffer, count);
}

int DaqDevice::memWrite(MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_WRITE, address, buffer, count);

	return memWrite(MT_EEPROM, memRegionType, address, buffer, count);
}

void DaqDevice::check_MemRW_Args(MemRegion memRegionType, MemAccessType accessType, unsigned int address, unsigned char* buffer, unsigned int count, bool checkAccess) const
{
	if(address == (unsigned long long) getMemUnlockAddr())
		return;

	std::bitset<32> memRegionBitset(mDaqDeviceInfo.memInfo()->getMemRegionTypes());

	if((memRegionType & mDaqDeviceInfo.memInfo()->getMemRegionTypes()) || memRegionBitset.count() == 1)
	{
		UlMemRegionInfo& memRegionInfo = mDaqDeviceInfo.memInfo()->getMemRegionInfo(memRegionType);

		if(checkAccess && !(memRegionInfo.getAccessTypes() & accessType))
			throw UlException(ERR_MEM_ACCESS_DENIED);

		unsigned long long memStartAddr = memRegionInfo.getAddress();
		unsigned long long endAddr = memRegionInfo.getAddress() + memRegionInfo.getSize() - 1;

		if(address < memStartAddr || address > endAddr)
			throw UlException(ERR_BAD_MEM_ADDRESS);

		if((address + count - 1) > endAddr)
			throw UlException(ERR_BAD_MEM_ADDRESS);

		if(buffer == NULL)
			throw UlException(ERR_BAD_BUFFER);
	}
	else
		throw UlException(ERR_BAD_MEM_REGION);

	if(!isConnected())
		throw UlException(ERR_NO_CONNECTION_ESTABLISHED);
}



} /* namespace ul */
