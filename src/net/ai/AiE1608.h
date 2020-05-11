/*
 * AiE1608.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AI_AIE1608_H_
#define NET_AI_AIE1608_H_

#include "AiNetBase.h"

namespace ul
{

class UL_LOCAL AiE1608: public AiNetBase
{
public:
	AiE1608(const NetDaqDevice& daqDevice);
	virtual ~AiE1608();

	virtual void initialize();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();

private:
	virtual void sendStopCmd();
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const;
	void loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const;
	void setScanConfig(int chanCount, unsigned int scanCount, double rate, ScanOption options);
	int mapRangeCode(Range range) const;
	int getChanCode(int chan, AiInputMode inputMode) const;
	unsigned char getOptionsCode(ScanOption options) const;
	unsigned int calcPacerPeriod(int chanCount, double rate, ScanOption options);

	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { CMD_AINSTOP = 0x13 };
	enum { FIFO_SIZE = 48 * 1024 };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_START = 0x11, CMD_AIQUEUE_R = 0x14, CMD_AIQUEUE_W = 0x15};
	enum { TRIG_EDGE_RISING = 1, TRIG_EDGE_FALLING = 2, TRIG_LEVEL_HIGH = 3, TRIG_LEVEL_LOW = 4};

#pragma pack(1)
	struct TAINSCAN_CFG
	{
		unsigned int scan_count;
		unsigned int pacer_period;
		unsigned char options;
	} mScanConfig;
#pragma pack()


};

} /* namespace ul */

#endif /* NET_AI_AIE1608_H_ */
