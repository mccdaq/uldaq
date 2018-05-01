/*
 * AoUsb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB1808_H_
#define USB_AO_AOUSB1808_H_

#include "AoUsbBase.h"

namespace ul
{

class UL_LOCAL AoUsb1808: public AoUsbBase
{
public:
	AoUsb1808(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb1808();

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	virtual ScanStatus getScanState() const;
	virtual UlError waitUntilDone(double timeout);

	CalCoef getChanCalCoef(int channel, long long flags) const;

protected:
	virtual int getCalCoefIndex(int channel, Range range) const;

private:
	enum { FIFO_SIZE = 8 * 1024 };
	enum { CMD_AOUT = 0x18};
};

} /* namespace ul */

#endif /* USB_AO_AOUSB1808_H_ */
