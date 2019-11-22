/*
 * AoUsb1608hs.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB1608HS_H_
#define USB_AO_AOUSB1608HS_H_

#include "AoUsbBase.h"

namespace ul
{

class UL_LOCAL AoUsb1608hs: public AoUsbBase
{
public:
	AoUsb1608hs(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb1608hs();

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]);

	virtual UlError checkScanState(bool* scanDone) const;

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_SenseMode(int channel, AOutSenseMode mode);
	virtual AOutSenseMode getCfg_SenseMode(int channel) const;

protected:
	virtual int getCalCoefIndex(int channel, Range range) const;
	unsigned char getChannelMask(int lowChan, int highChan) const;
	unsigned char getOptionsCode(int lowChan, int highChan, ScanOption options) const;

	void readAOutVals();

private:
	void setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options);
	void calcPacerParams(double rate, unsigned char& prescale, unsigned short& preload);

private:
	enum { FIFO_SIZE = 1024 , CLK_FREQ = 10000000};
	enum { CMD_AOUT = 0x18, CMD_AOUTSCAN_CONFIG = 0x19, CMD_AOUTSCAN_START = 0x1A, CMD_AOUTSTOP = 0x1B, CMD_AOUT_CONFIG = 0x1C};

	unsigned short mAOutVals[2];

#pragma pack(1)
	struct TAINSCAN_CFG
	{
		unsigned int scan_count;
		unsigned char prescale;
		unsigned short preload;
		unsigned char options;
	} mScanConfig;
#pragma pack()

};


} /* namespace ul */

#endif /* USB_AO_AOUSB1608HS_H_ */
