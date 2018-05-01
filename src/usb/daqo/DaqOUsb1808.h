/*
 * DaqOUsb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DAQO_DAQOUSB1808_H_
#define USB_DAQO_DAQOUSB1808_H_

#include "DaqOUsbBase.h"

namespace ul
{

class UL_LOCAL DaqOUsb1808: public DaqOUsbBase
{
public:
	DaqOUsb1808(const UsbDaqDevice& daqDevice);
	virtual ~DaqOUsb1808();

	virtual double daqOutScan(FunctionType functionType, DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqOutScanFlag flags, void* data);

private:
	void loadScanConfigs(DaqOutChanDescriptor chanDescriptors[], int numChans) const;
	void setScanConfig(FunctionType functionType, int chanCount, unsigned int scanCount, double rate, ScanOption options, DaqOutScanFlag flags);
	unsigned char getOptionsCode(FunctionType functionType, ScanOption options, DaqOutScanFlag flags) const;
	std::vector<CalCoef> getScanCalCoefs(DaqOutChanDescriptor chanDescriptors[], int numChans, DaqOutScanFlag flags) const;

private:
	enum { FIFO_SIZE = 8 * 1024 };
	enum {CMD_OUT_SCAN_CONFIG = 0x19, CMD_OUT_SCAN_START = 0x1A, CMD_OUT_SCAN_STOP = 0x1B, CMD_SCAN_CLEARFIFO = 0x1C};
	enum {EDGE = 1, LEVEL=0, LOW = 0, HIGH = 1, NEG_POL = 0, POS_POL = 1};

	bool mPatternTrig;
	unsigned int mRetrigCount;

#pragma pack(1)
	union TAINSCAN_CFG
	{
		struct
		{
			unsigned int scan_count;
			unsigned int retrig_count;
			unsigned int pacer_period;
			unsigned char options;
		};
		unsigned char raw[13];
	} mScanConfig;
#pragma pack()
};

} /* namespace ul */

#endif /* USB_DAQO_DAQOUSB1808_H_ */
