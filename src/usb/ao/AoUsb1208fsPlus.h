/*
 * AoUsb1208fsPlus.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB1208FSPLUS_H_
#define USB_AO_AOUSB1208FSPLUS_H_

#include "AoUsbBase.h"

namespace ul
{

class UL_LOCAL AoUsb1208fs_Plus: public AoUsbBase
{
public:
	AoUsb1208fs_Plus(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb1208fs_Plus();

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]);

protected:
	virtual int getCalCoefIndex(int channel, Range range) const;
	unsigned char getChannelMask(int lowChan, int highChan) const;
	unsigned char getOptionsCode(int lowChan, int highChan, ScanOption options) const;

private:
	void setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options);

private:
	enum { FIFO_SIZE = 8 * 1024 };
	enum { CMD_AOUT = 0x18, CMD_AOUTSCAN_START = 0x1A, CMD_AOUTSTOP = 0x1B, CMD_AOUTSCAN_CLEAR_FIFO = 0x1C};

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

#endif /* USB_AO_AOUSB1208FSPLUS_H_ */
