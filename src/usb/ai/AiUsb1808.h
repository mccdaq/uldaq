/*
 * AiUsb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB1808_H_
#define USB_AI_AIUSB1808_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb1808: public AiUsbBase
{
	friend class DaqIUsb1808;
public:
	AiUsb1808(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb1808();

	virtual void initialize();
	virtual void disconnect();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

	CalCoef getChanCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const;
	CustomScale getChanCustomScale(int channel) const;

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	virtual ScanStatus getScanState() const;
	virtual UlError waitUntilDone(double timeout);

protected:
	void loadAInConfig(int chan, AiInputMode mode, Range range) const;
	void loadAInConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const;
	void resetAInConfigs() const;
	void writeAInConfigs() const;

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const;
	int mapRangeCode(Range range) const;

	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { FIFO_SIZE = 8 * 4 * 1024 }; // samples size is 4
	enum { CMD_AIN = 0x10, CMD_ADC_SETUP = 0x11 };
	enum { DIFF_MODE = 0,  SE_MODE = 1, GROUND_MODE = 3};

#pragma pack(1)
	typedef union
	{
	  struct
	  {
		  unsigned char range : 2;
		  unsigned char mode : 2;
		  unsigned char resv : 4;
	  };

	  unsigned char mask;
	} TADCCONFIG;
#pragma pack()

	mutable TADCCONFIG mAdcConfig[8];
};

} /* namespace ul */

#endif /* USB_AI_AIUSB1808_H_ */
