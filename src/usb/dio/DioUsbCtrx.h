/*
 * DioUsbCtrx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSBCTRX_H_
#define USB_DIO_DIOUSBCTRX_H_

#include "DioUsb1608g.h"

namespace ul
{

class UL_LOCAL DioUsbCtrx: public DioUsb1608g
{
public:
	DioUsbCtrx(const UsbDaqDevice& daqDevice);
	virtual ~DioUsbCtrx();

	virtual double dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]);

	virtual UlError getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground(ScanDirection direction);

	virtual UlError waitUntilDone(ScanDirection direction, double timeout);

	virtual ScanStatus getScanState(ScanDirection direction) const;

private:
	enum { FIFO_SIZE = 8 * 2 * 1024 }; // samples size is 2
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSBCTRX_H_ */
