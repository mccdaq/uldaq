/*
 * DaqIUsbCtrx.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DAQI_DAQIUSBCTRX_H_
#define USB_DAQI_DAQIUSBCTRX_H_

#include "DaqIUsbBase.h"

namespace ul
{

class UL_LOCAL DaqIUsbCtrx: public DaqIUsbBase
{
	friend class TmrUsb1808;
public:
	DaqIUsbCtrx(const UsbDaqDevice& daqDevice);
	virtual ~DaqIUsbCtrx();

	virtual double daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data);

protected:
	void loadScanConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const;

	void check_DaqInScan_Args(DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data) const;
private:
	void setScanConfig(FunctionType functionType, int chanCount, unsigned int scanCount, int sampleSize, double rate, ScanOption options, DaqInScanFlag flags);
	unsigned char getOptionsCode(FunctionType functionType, ScanOption options, DaqInScanFlag flags) const;
	int calcMaxSampleSize(DaqInChanDescriptor chanDescriptors[], int numChans) const;

private:
	enum { FIFO_SIZE = 8 * 2 * 1024 };
	enum { CMD_SCAN_CONFIG = 0x20, CMD_SCAN_START = 0x21, CMD_SCAN_STOP = 0x22, CMD_SCAN_CLEARFIFO = 0x23};

	unsigned int mInternalChanTypes;

#pragma pack(1)
	union TAINSCAN_CFG
	{
		struct
		{
			unsigned int scan_count;
			unsigned int retrig_count;
			unsigned int pacer_period;
			unsigned char packet_size;
			unsigned char options;
		};
		unsigned char raw[14];
	} mScanConfig;
#pragma pack()

};

} /* namespace ul */

#endif /* USB_DAQI_DAQIUSBCTRX_H_ */
