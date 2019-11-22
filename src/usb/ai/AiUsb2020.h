/*
 * AiUsb2020.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB2020_H_
#define USB_AI_AIUSB2020_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb2020: public AiUsbBase
{
public:
	AiUsb2020(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb2020();

	virtual void initialize();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

protected:
	virtual void readCalDate();

	virtual void check_AInScan_Args(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) const;

	void setupTrigger(int lowChan, int highChan, Range range, ScanOption options) const;

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const;
	void loadAInConfigs(Range range, int lowChan, int highChan, bool queueEnabled) const;
	void setScanConfig(int chanCount, unsigned int scanCount, double rate, ScanOption options);
	int mapRangeCode(Range range) const;
	//int getModeCode(int chan, AiInputMode inputMode) const;
	unsigned char getOptionsCode(ScanOption options) const;

	unsigned short getRawThreshold(int trigChan, Range range, double threshold) const;

	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { MIN_DRAM_SAMPLE_COUNT = 40, MAX_DRAM_SAMPLE_COUNT = 128 * 1024 * 1024 / 2, BURST_SIZE_DRAM = 4 };
	enum { FIFO_SIZE = 2 * MAX_DRAM_SAMPLE_COUNT };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_START = 0x12, CMD_AINSTOP = 0x13, CMD_AIN_CONFIG = 0x14, CMD_AINSCAN_CLEAR_FIFO = 0x15, CMD_SETTRIG = 0x43};
	enum {TRIGGER = 0, GATE = 1,
		  DIGITAL_TRIG = 0, ANALOG_TRIG = 1,
		  LEVEL=0, EDGE = 1, HYSTERESIS = 2, WINDOW = 3,
		  LOW = 0, HIGH = 1 , FALLING_ = 0, RISING_ = 1, BELOW = 0, ABOVE = 1, NEG_EDGE = 0, POS_EDGE = 1,  NEG_HYS = 0, POS_HYS = 1, WINDOW_OUT = 0, WINDOW_IN = 1};

#pragma pack(1)
	struct TAINSCAN_CFG
	{
		unsigned int scan_count;
		unsigned int retrig_count;
		unsigned int pacer_period;
		unsigned char packet_size;
		unsigned char options;
		unsigned char reserved;
	} mScanConfig;
#pragma pack()
};

} /* namespace ul */

#endif /* USB_AI_AIUSB2020_H_ */
