/*
 * CtrUsb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSB1808_H_
#define USB_CTR_CTRUSB1808_H_

#include "CtrUsbBase.h"

namespace ul
{

class UL_LOCAL CtrUsb1808: public CtrUsbBase
{
public:
	CtrUsb1808(const UsbDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsb1808();

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
	unsigned char getCtrOptionCode(CounterMeasurementType measureType,  CounterMeasurementMode measureMode, CounterEdgeDetection edgeDetection) const;

	void addSupportedTickSizes();
	void addSupportedDebounceTimes();

private:
	enum { FIFO_SIZE = 8 * 4 * 1024 }; // samples size is 4
	enum { CMD_CTR = 0x20, CMD_LIMIT_VALS = 0x22, CMD_CTR_PARAMS = 0x24};
};

} /* namespace ul */

#endif /* USB_CTR_CTRUSB1808_H_ */
