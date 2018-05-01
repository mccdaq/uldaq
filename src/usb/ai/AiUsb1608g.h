/*
 * AiUsb1608g.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB1608G_H_
#define USB_AI_AIUSB1608G_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb1608g: public AiUsbBase
{
public:
	AiUsb1608g(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb1608g();

	virtual void initialize();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

protected:
	virtual void readCalDate();

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const;
	void loadAInConfigs(AiInputMode inputMode, Range range, int lowChan, int highChan, bool queueEnabled) const;
	void setScanConfig(int chanCount, unsigned int scanCount, double rate, ScanOption options);
	int mapRangeCode(Range range) const;
	int getModeCode(int chan, AiInputMode inputMode) const;
	unsigned char getOptionsCode(ScanOption options) const;

	void addSupportedRanges();
	void addQueueInfo();

// calibration functions
/*	void calibrate() const;
	void calAdc(Range range) const;
	void getCalOutputIndexs(Range range, unsigned int* Idx) const;
	void getVRefs(Range range, double* refs) const;
	unsigned int getVRefOffset(Range range) const;
	void storeCalCoefs(Range range, float slope, float offset) const;
	void storeCalDate() const;
	void reloadCalCoefs() const;*/

private:
	enum { FIFO_SIZE = 8 * 1024 };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_START = 0x12, CMD_AINSTOP = 0x13, CMD_AIN_CONFIG = 0x14, CMD_AINSCAN_CLEAR_FIFO = 0x15, CMD_SETTRIG = 0x43};

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

#endif /* USB_AI_AIUSB1608G_H_ */
