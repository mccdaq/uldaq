/*
 * DaqDeviceInternal.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQDEVICE_H_
#define DAQDEVICE_H_

#include "DaqDeviceId.h"
#include "DaqDeviceInfo.h"
#include "DaqDeviceConfig.h"
#include "interfaces/UlDaqDevice.h"
#include "uldaq.h"
#include "ul_internal.h"
#include "utility/FnLog.h"

namespace ul
{
class IoDevice;
class AiDevice;
class AoDevice;
class DioDevice;
class CtrDevice;
class TmrDevice;
class DaqIDevice;
class DaqODevice;
class DaqEventHandler;

class UL_LOCAL DaqDevice: public UlDaqDevice
{
public:
	DaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~DaqDevice();

	virtual void connect()= 0;
	virtual void disconnect() = 0;
	virtual void connectionCode(long long code);

	unsigned int getDeviceType() const { return mDaqDeviceDescriptor.productId; }
	long long getDeviceNumber() const {return mDeviceNumber;}

	virtual const UlDaqDeviceInfo& getDevInfo() const { return mDaqDeviceInfo;}
	virtual UlDaqDeviceConfig& getDevConfig() { return *mDaqDeviceConfig;}

	AiDevice* aiDevice() const;
	void setAiDevice(AiDevice* aiDevice);

	AoDevice* aoDevice() const;
	void setAoDevice(AoDevice* aoDevice);

	DioDevice* dioDevice() const;
	void setDioDevice(DioDevice* dioDevice);

	CtrDevice* ctrDevice() const;
	void setCtrDevice(CtrDevice* ctrDevice);

	TmrDevice* tmrDevice() const;
	void setTmrDevice(TmrDevice* tmrDevice);

	DaqIDevice* daqIDevice() const;
	void setDaqIDevice(DaqIDevice* daqIDevice);

	DaqODevice* daqODevice() const;
	void setDaqODevice(DaqODevice* daqODevice);

	DaqEventHandler* eventHandler() const;

	bool isConnected() const { return mConnected;}

	void initializeIoDevices();
	void reconfigureIoDevices();
	void disconnectIoDevices();
	//void terminateScans();
	bool isScanRunning() const;
	bool isScanRunning(FunctionType functionType) const;
	void stopBackground(FunctionType functionType) const;

	void checkConnection() const;
	virtual void flashLed(int flashCount) const;

	virtual DaqDeviceDescriptor getDescriptor() const;

	void getEuScaling(Range range, double &scale, double &offset) const;

	virtual int memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	virtual int memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	virtual int memRead(MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	virtual int memWrite(MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	void setMemUnlockAddr(int addr) { mMemUnlockAddr = addr; }
	int getMemUnlockAddr() const { return mMemUnlockAddr; }

	void setMemUnlockCode(unsigned int code) { mMemUnlockCode = code; }
	unsigned int getMemUnlockCode() const { return mMemUnlockCode; }

	void addMemRegion(MemRegion memRegionType, unsigned long long address, unsigned long long size, long long accessTypes);

	virtual void setCalOutput (unsigned int index) const { };

	unsigned short getRawFwVer() const  { return mRawFwVersion;}

	// public interface functions
	UlAiDevice& getAiDevice() const;
	UlAoDevice& getAoDevice() const;
	UlDioDevice& getDioDevice() const;
	UlCtrDevice& getCtrDevice() const;
	UlTmrDevice& getTmrDevice() const;
	UlDaqIDevice& getDaqIDevice() const;
	UlDaqODevice& getDaqODevice() const;

	double getClockFreq() const;

	IoDevice* getIoDevice(FunctionType functionType) const;
	TriggerConfig getTriggerConfig(FunctionType functionType) const;

	pthread_mutex_t& getDeviceMutex() const { return mDeviceMutex;}

	bool hasExp() const { return mHasExp;}

	//////////////////////          Configuration functions          /////////////////////////////////
	void getCfg_FwVersionStr(DevVersionType verType, char* fpgaVerStr, unsigned int* maxStrLen) const;
	void getCfg_FpgaVersionStr(char* fpgaVerStr, unsigned int* maxStrLen) const;
	void getCfg_RadioVersionStr(char* fpgaVerStr, unsigned int* maxStrLen) const;
	virtual void getCfg_IpAddress(char* address, unsigned int* maxStrLen) const;
	virtual void getCfg_NetIfcName(char* ifcName, unsigned int* maxStrLen) const;

	virtual long long getCfg_ConnectionCode() const;
	virtual void setCfg_ConnectionCode(long long code);
	virtual long long getCfg_MemUnlockCode() const;
	virtual void setCfg_MemUnlockCode(long long code);
	virtual void setCfg_Reset();

protected:
	void setMinRawFwVersion(unsigned short ver) { mMinRawFwVersion = ver;}
	void check_MemRW_Args(MemRegion memRegionType, MemAccessType accessType, unsigned int address, unsigned char* buffer, unsigned int count, bool checkAccess = true) const;

protected:
	DaqDeviceDescriptor mDaqDeviceDescriptor;
	bool mConnected;
	DaqDeviceInfo mDaqDeviceInfo;
	DaqDeviceConfig* mDaqDeviceConfig;

	AiDevice* mAiDevice;
	AoDevice* mAoDevice;
	DioDevice* mDioDevice;
	CtrDevice* mCtrDevice;
	TmrDevice* mTmrDevice;
	DaqIDevice* mDaqIDevice;
	DaqODevice* mDaqODevice;
	DaqEventHandler* mEventHandler;

	unsigned short mMinRawFwVersion;

	unsigned short mRawFwVersion;
	unsigned short mRawFpgaVersion;
	unsigned short mRawRadioVersion;
	unsigned short mRawFwMeasurementVersion;
	unsigned short mRawFwExpMeasurementVersion;

	mutable unsigned long long mCurrentSuspendCount;

	mutable bool mHasExp;

private:
	static unsigned long long mNextAvailableDeviceNumber;
	static pthread_mutex_t mDeviceNumberMutex;
	mutable pthread_mutex_t mDeviceMutex;
	long long mDeviceNumber;
	int mMemUnlockAddr;
	unsigned int mMemUnlockCode;
};

} /* namespace ul */

#endif /* DAQDEVICE_H_ */
