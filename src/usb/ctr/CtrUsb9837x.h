/*
 * CtrUsb9837x.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSB9837X_H_
#define USB_CTR_CTRUSB9837X_H_

#include "CtrUsbBase.h"
#include "../Usb9837x.h"

namespace ul
{

class UL_LOCAL CtrUsb9837x: public CtrUsbBase
{
public:
	CtrUsb9837x(const UsbDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsb9837x();

	const Usb9837x& dtDev() const {return (const Usb9837x&) daqDev();}

	virtual void initialize();

	virtual double cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	UlError waitUntilDone(double timeout);

	virtual ScanStatus getScanState() const;

	virtual void setCfg_CtrReg(int ctrNum, long long regVal);
	virtual long long getCfg_CtrReg(int ctrNum) const;

protected:
	virtual void check_CtrSetTrigger_Args(TriggerType trigtype, int trigChan,  double level, double variance, unsigned int retriggerCount) const;

private:
	enum { FIFO_SIZE = 2 * 1024 * 4 };  // 2K sample FIFO according to DT9837A HW specification (page 4)

	//unsigned short mReg4Val;

};

} /* namespace ul */

#endif /* USB_CTR_CTRUSB9837X_H_ */
