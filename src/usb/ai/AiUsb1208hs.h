/*
 * AiUsb1208hs.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB1208HS_H_
#define USB_AI_AIUSB1208HS_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb1208hs: public AiUsbBase
{
public:
	AiUsb1208hs(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb1208hs();

	virtual void initialize();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const;
	void loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const;
	void setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options);
	int getAdcChanNum( int chan, AiInputMode inputMode) const;
	int mapRangeCode(AiInputMode inputMode, Range range) const;
	int getModeCode(AiInputMode inputMode) const;
	unsigned char getOptionsCode(ScanOption options) const;
	unsigned int calcPacerPeriod(int chanCount, double rate, ScanOption options);

	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { FIFO_SIZE = 8 * 1024 };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_START = 0x12, CMD_AINSTOP = 0x13, CMD_AIN_CONFIG = 0x14, CMD_AINSCAN_CLEAR_FIFO = 0x15, CMD_SETTRIG = 0x43};

#pragma pack(1)

	mutable struct
	{
		  unsigned char modeCode;
		  unsigned char rangeCode[8];
	} mAInConfig;

	struct TAINSCAN_CFG
	{
		unsigned int scan_count;
		unsigned int retrig_count;
		unsigned int pacer_period;
		unsigned char channels;
		unsigned char packet_size;
		unsigned char options;
	} mScanConfig;
#pragma pack()
};

} /* namespace ul */

#endif /* USB_AI_AIUSB1208HS_H_ */
