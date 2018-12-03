/*
 * AiUsb1608fsPlus.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AIUSB1608FSPLUS_H_
#define AIUSB1608FSPLUS_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb1608fs_Plus: public AiUsbBase
{
private:
#pragma pack(1)

	struct TAINSCAN_CFG
	{
		unsigned int  scan_count;
		unsigned int  pacer_period;
		unsigned char chan_mask;
		unsigned char options;
	};

#pragma pack()

public:
	AiUsb1608fs_Plus(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb1608fs_Plus();

	virtual void initialize();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const;
	void aInConfig(int lowChan, int highChan, Range range);
	TAINSCAN_CFG scanConfig(int lowChan, int highChan, int scanCount, double rate, ScanOption options);
	int mapRangeCode(Range range) const;
	unsigned char getChannelMask(int lowChan, int highChan) const;
	unsigned char getOptionsCode(ScanOption options) const;
	unsigned char getTrigModeCode() const;

	void addSupportedRanges();
	void addSupportedTriggerTypes();
	void addQueueInfo();

private:
	enum { FIFO_SIZE = 64 * 1024 };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_START = 0x11, CMD_AINSTOP = 0x12, CMD_AINSCAN_CONFIG = 0x14, CMD_AINSCAN_CLEAR_FIFO = 0x15};
	enum { TRIG_EDGE_RISING = 1, TRIG_EDGE_FALLING = 2, TRIG_LEVEL_HIGH = 3, TRIG_LEVEL_LOW = 4};
};

} /* namespace ul */

#endif /* AIUSB1608FSPLUS_H_ */
