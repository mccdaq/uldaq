/*
 * AoUsb9837x.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB9837X_H_
#define USB_AO_AOUSB9837X_H_

#include "AoUsbBase.h"
#include "../Usb9837x.h"

namespace ul
{

class UL_LOCAL AoUsb9837x: public AoUsbBase
{
public:
	AoUsb9837x(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb9837x();

	const Usb9837x& dtDev() const {return (const Usb9837x&) daqDev();}

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]);

	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone) const;

	void underrunOccured() { mUnderrunOccurred = true;}

	CalCoef getInputChanCalCoef(int channel, long long flags) const;


	void CmdSetSingleValueDAC(unsigned int wAnalogOutValue, unsigned char Channel);

protected:
	void check_AOutSetTrigger_Args(TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const;
	virtual void loadDacCoefficients();
	virtual int getCalCoefIndex(int channel, Range range) const { return 0;}
	/*unsigned char getChannelMask(int lowChan, int highChan) const;
	unsigned char getOptionsCode(int lowChan, int highChan, ScanOption options) const;*/

	virtual void sendStopCmd();

private:
	void setDAOutputSampleClock(double rate);
	void configureScan(int samplesPerChan, ScanOption options);

	void StopUsbOutputStreaming();
	void CmdEnableDAEvents();
	void CmdDisableDAEvents();
	void CmdSetArmDACtrls(ScanOption options);

	virtual unsigned int processScanData32(libusb_transfer* transfer, unsigned int stageSize);


private:
	enum { FIFO_SIZE = 8 * 1024 * 4};

	bool mUnderrunOccurred;

	double mPreviousClockFreq;
	//double mPreviousActualClockFreq;
};

} /* namespace ul */

#endif /* USB_AO_AOUSB9837X_H_ */
