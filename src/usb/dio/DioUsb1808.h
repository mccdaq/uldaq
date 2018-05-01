/*
 * DioUsb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSB1808_H_
#define USB_DIO_DIOUSB1808_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsb1808: public DioUsbBase
{
public:
	DioUsb1808(const UsbDaqDevice& daqDevice);
	virtual ~DioUsb1808();

	virtual void initialize();

	virtual void dConfigPort(DigitalPortType portType, DigitalDirection direction);
	virtual void dConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction);

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual double dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]);
	virtual double dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]);

	virtual UlError getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground(ScanDirection direction);

	virtual UlError waitUntilDone(ScanDirection direction, double timeout);

	virtual ScanStatus getScanState(ScanDirection direction) const;

protected:
	virtual unsigned long readPortDirMask(unsigned int portNum) const;
	virtual void check_SetTrigger_Args(ScanDirection direction, TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const;

private:
	enum { FIFO_SIZE_IN = 8 * 4 * 1024, FIFO_SIZE_OUT = 8 * 1024 };
	enum {CMD_DTRISTATE = 0x00, CMD_DPORT = 0x01, CMD_DLATCH = 0x02};
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSB1808_H_ */
