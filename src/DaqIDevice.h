/*
 * DaqIDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQIDEVICE_H_
#define DAQIDEVICE_H_

#include "IoDevice.h"
#include "DaqIInfo.h"
#include "interfaces/UlDaqIDevice.h"

namespace ul
{

class UL_LOCAL DaqIDevice: public IoDevice, public UlDaqIDevice
{
public:
	virtual ~DaqIDevice();
	DaqIDevice(const DaqDevice& daqDevice);

	virtual const UlDaqIInfo& getDaqIInfo() { return mDaqIInfo;}

	virtual double daqInScan(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, double data[]);
	virtual void setTrigger(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual UlError getStatus(FunctionType functionType, ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual void stopBackground(FunctionType functionType);
	UlError waitUntilDone(FunctionType functionType, double timeout);
	virtual UlError waitUntilDone(double timeout) { return IoDevice::waitUntilDone(timeout); }

	virtual double daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data);

protected:
	virtual void check_DaqInScan_Args(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data) const;
	virtual void check_DaqInSetTrigger_Args(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) const;

	void storeLastStatus();
	UlError getLastStatus(FunctionType functionType, TransferStatus* xferStatus);

protected:
	DaqIInfo mDaqIInfo;

private:
	struct
	{
		UlError error;
		unsigned long long scanCount;
		unsigned long long totalCount;
		long long index;
	}mLastStatus[4];
};

} /* namespace ul */

#endif /* DAQIDEVICE_H_ */
