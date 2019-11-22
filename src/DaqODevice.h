/*
 * DaqODevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQODEVICE_H_
#define DAQODEVICE_H_

#include "IoDevice.h"
#include "DaqOInfo.h"
#include "interfaces/UlDaqODevice.h"

namespace ul
{

class UL_LOCAL DaqODevice: public IoDevice, public UlDaqODevice
{
public:
	virtual ~DaqODevice();
	DaqODevice(const DaqDevice& daqDevice);

	virtual const UlDaqOInfo& getDaqOInfo() { return mDaqOInfo;}

	virtual double daqOutScan(DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, double data[]);
	virtual void setTrigger(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual UlError getStatus(FunctionType functionType, ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual void stopBackground(FunctionType functionType);
	UlError waitUntilDone(FunctionType functionType, double timeout);
	virtual UlError waitUntilDone(double timeout) { return IoDevice::waitUntilDone(timeout); }

	virtual double daqOutScan(FunctionType functionType, DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, void* data);

protected:
	void check_DaqOutScan_Args(DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, void* data) const;
	void check_DaqOutSetTrigger_Args(TriggerType type, DaqInChanDescriptor trigChanDesc, double level, double variance, unsigned int retriggerCount) const;

	void storeLastStatus();
	UlError getLastStatus(FunctionType functionType, TransferStatus* xferStatus);

protected:
	DaqOInfo mDaqOInfo;

private:
	struct
	{
		UlError error;
		unsigned long long scanCount;
		unsigned long long totalCount;
		long long index;
	}mLastStatus[3];
};

} /* namespace ul */

#endif /* DAQODEVICE_H_ */
