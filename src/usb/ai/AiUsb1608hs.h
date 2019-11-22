/*
 * AiUsb1608hs.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB1608HS_H_
#define USB_AI_AIUSB1608HS_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb1608hs: public AiUsbBase
{
public:
	AiUsb1608hs(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb1608hs();

	virtual void initialize();
	virtual void disconnect();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

	virtual UlError checkScanState(bool* scanDone = NULL) const;

protected:
	virtual void readCalDate();

private:
	int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const;
	void loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const;
	void setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options);
	int mapRangeCode(Range range) const;
	int getModeCode(AiInputMode inputMode) const;
	unsigned char getOptionsCode(ScanOption options) const;
	unsigned int calcPacerPeriod(int chanCount, double rate, ScanOption options);

	void resetAInConfigs() const;
	void writeAInConfigs() const;

	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { FIFO_SIZE = 128 * 1024 };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_CONFIG = 0x11, CMD_AINSCAN_START = 0x12, CMD_AINSTOP = 0x13, CMD_AIN_CONFIG = 0x14, CMD_SETTRIG = 0x43};
	enum { DIFF_MODE = 0,  SE_MODE = 1, GROUND_MODE = 3};

#pragma pack(1)

	mutable struct
	{
		union
		{
			struct
			{
			  unsigned char rangeCode : 2;
			  unsigned char modeCode  : 2;
			  unsigned char reserved  : 4;
			};
			unsigned char code;
		};

	} mAInConfig[8];


	struct TAINSCAN_CFG
	{
		unsigned char low_chan;
		unsigned char chan_count;
		unsigned char scan_count[3];
		unsigned int pacer_period;
		unsigned char options;
	} mScanConfig;
#pragma pack()
};

} /* namespace ul */

#endif /* USB_AI_AIUSB1608HS_H_ */
