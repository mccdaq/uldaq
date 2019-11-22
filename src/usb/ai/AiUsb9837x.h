/*
 * AiUsb9837x.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB9837X_H_
#define USB_AI_AIUSB9837X_H_

#include "AiUsbBase.h"
#include "../Usb9837x.h"

namespace ul
{

class UL_LOCAL AiUsb9837x: public AiUsbBase
{
	friend class DaqIUsb9837x;
public:
	AiUsb9837x(const Usb9837x& daqDevice);
	virtual ~AiUsb9837x();

	const Usb9837x& dtDev() const {return (const Usb9837x&) daqDev();}

	virtual void initialize();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

	CalCoef getChanCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const;
	CustomScale getChanCustomScale(int channel) const;

	static void VoltsToRawValue (double volts, double gain, unsigned int* rawValue, int Resolution);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	virtual ScanStatus getScanState() const;
	virtual UlError waitUntilDone(double timeout);

	void setCurrentChanRange(int channel, Range range) const;
	Range getCurrentChanRange(int channel) const;

	virtual void setCfg_ChanIepeMode(int channel, IepeMode mdoe);
	virtual IepeMode getCfg_ChanIepeMode(int channel);

	virtual void setCfg_ChanCouplingMode(int channel, CouplingMode mode);
	virtual CouplingMode getCfg_ChanCouplingMode(int channel);

	virtual void setCfg_ChanSensorSensitivity(int channel, double sensitivity);
	virtual double getCfg_ChanSensorSensitivity(int channel);

protected:
	virtual void loadAdcCoefficients();

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const { return channel;}

	virtual void check_AInSetTrigger_Args(TriggerType trigtype, int trigChan,  double level, double variance, unsigned int retriggerCount) const;

	void applyEepromIepeSettings();
	void configureIepe();

	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { FIFO_SIZE = 2 * 1024 * 4 };  // 2K sample FIFO according to DT9837A HW specification (page 4
	enum { NUMBER_OF_ADC = 4};
	enum { DC = 0, AC = 1};
	enum { INTERNAL = 0, EXTERNAL = 1, DISABLED = 2 };

	int mCouplingType[NUMBER_OF_ADC];
	int mCurrentSource[NUMBER_OF_ADC];
	double mSensorSensitivity[NUMBER_OF_ADC];

	mutable Range mCurrentChanRange[NUMBER_OF_ADC];

};

} /* namespace ul */

#endif /* USB_AI_AIUSB9837X_H_ */
