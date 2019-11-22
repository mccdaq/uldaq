/*
 * CtrUsbCtrx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSBCTRX_H_
#define USB_CTR_CTRUSBCTRX_H_

#include "CtrUsbBase.h"

namespace ul
{

class UL_LOCAL CtrUsbCtrx: public CtrUsbBase
{
public:
	CtrUsbCtrx(const UsbDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsbCtrx();

	virtual void initialize();

	virtual unsigned long long cIn(int ctrNum);
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);
	virtual double cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]);

	virtual void cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
							CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
							CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	UlError waitUntilDone(double timeout);

	virtual ScanStatus getScanState() const;

private:
	unsigned char getModeOptionCode(CounterMeasurementType measureType, CounterMeasurementMode measureMode, CounterTickSize tickSize) const;
	unsigned char getCtrOptionCode(CounterMeasurementMode measureMode, CounterEdgeDetection edgeDetection) const;
	unsigned char getGateOptionCode(CounterMeasurementMode measureMode) const;
	unsigned char getOutputOptionCode(CounterMeasurementMode measureMode) const;
	unsigned char getDebounceOptionCode(CounterDebounceMode debounceMode, CounterDebounceTime debounceTime) const;

	void addSupportedTickSizes();
	void addSupportedDebounceTimes();

private:
	enum { FIFO_SIZE = 8 * 2 * 1024 }; // samples size is 2
	enum { CMD_CTR = 0x10, CMD_CTR_OUT_VALS = 0x16, CMD_LIMIT_VALS = 0x17, CMD_CTR_PARAMS = 0x18 };
};

} /* namespace ul */

#endif /* USB_CTR_CTRUSBCTRX_H_ */
