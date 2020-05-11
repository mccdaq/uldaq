/*
 * DioDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DIODEVICE_H_
#define DIODEVICE_H_

#include <vector>
#include <bitset>

#include "IoDevice.h"
#include "DioInfo.h"
#include "DioConfig.h"
#include "interfaces/UlDioDevice.h"

namespace ul
{

class UL_LOCAL DioDevice: public IoDevice, public UlDioDevice
{
public:
	DioDevice(const DaqDevice& daqDevice);
	virtual ~DioDevice();

	virtual const UlDioInfo& getDioInfo() { return mDioInfo;}
	virtual UlDioConfig& getDioConfig() { return *mDioConfig;}

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);
	virtual void dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction);
	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);
	virtual void dInArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	virtual void dOutArray(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual double dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]);
	virtual double dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]);

	void setScanState(ScanDirection direction, ScanStatus state);
	virtual ScanStatus getScanState(ScanDirection direction) const;
	ScanStatus getScanState() const;

	virtual UlError getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground(ScanDirection direction);
	virtual void stopBackground();

	virtual UlError wait(ScanDirection direction, WaitType waitType, long long waitParam, double timeout);
	virtual UlError wait(WaitType waitType, long long waitParam, double timeout) { return IoDevice::wait(waitType, waitParam, timeout); }
	virtual UlError waitUntilDone(ScanDirection direction, double timeout);
	virtual UlError waitUntilDone(double timeout) { return IoDevice::waitUntilDone(timeout); }

	void setTrigger(ScanDirection direction, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);

	TriggerConfig getTrigConfig(ScanDirection direction) const;

	virtual void dInSetTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);
	virtual void dOutSetTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);
	virtual UlError dInGetStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual UlError dOutGetStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void dInStopBackground();
	virtual void dOutStopBackground();
	virtual void dClearAlarm(DigitalPortType portType, unsigned long long mask);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual unsigned long long getCfg_PortDirectionMask(unsigned int portNum) const;
	virtual void setCfg_PortInitialOutputVal(unsigned int portNum, unsigned long long val);
	virtual void setCfg_PortIsoMask(unsigned int portNum, unsigned long long mask);
	virtual unsigned long long getCfg_PortIsoMask(unsigned int portNum);
	virtual unsigned long long getCfg_PortLogic(unsigned int portNum);


protected:
	void initPortsDirectionMask();
	virtual unsigned long readPortDirMask(unsigned int portNum) const { return 0; }

	void check_DConfigPort_Args(DigitalPortType portType, DigitalDirection direction);
	void check_DConfigBit_Args(DigitalPortType portType, int bitNum, DigitalDirection direction);
	void check_DIn_Args(DigitalPortType portType);
	void check_DOut_Args(DigitalPortType portType, unsigned long long data);
	void check_DInArray_Args(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	void check_DOutArray_Args(DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);
	void check_DBitIn_Args(DigitalPortType portType, int bitNum);
	void check_DBitOut_Args(DigitalPortType portType, int bitNum);
	void check_DInScan_Args(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]) const;
	void check_DOutScan_Args(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]) const;
	virtual void check_SetTrigger_Args(ScanDirection direction, TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const;

	std::bitset<32> getPortDirection(DigitalPortType portType) const;
	void setPortDirection(DigitalPortType portType, DigitalDirection direction);
	void setBitDirection(DigitalPortType portType, int bitNum, DigitalDirection direction);

protected:
	DioInfo mDioInfo;
	DioConfig* mDioConfig;

private:
	std::vector<std::bitset<32> > mPortDirectionMask;
	ScanStatus mScanInState;
	ScanStatus mScanOutState;

	TriggerConfig mDiTrigCfg;
	TriggerConfig mDoTrigCfg;

	bool mDisableCheckDirection;
};

} /* namespace ul */

#endif /* DIODEVICE_H_ */
