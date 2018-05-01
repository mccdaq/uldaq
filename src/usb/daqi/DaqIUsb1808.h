/*
 * DaqIUsb1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DAQI_DAQIUSB1808_H_
#define USB_DAQI_DAQIUSB1808_H_

#include "DaqIUsbBase.h"
#include "../tmr/TmrUsb1808.h"

namespace ul
{

class UL_LOCAL DaqIUsb1808: public DaqIUsbBase
{
	friend class TmrUsb1808;
public:
	DaqIUsb1808(const UsbDaqDevice& daqDevice);
	virtual ~DaqIUsb1808();

	virtual double daqInScan(FunctionType functionType, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double rate, ScanOption options, DaqInScanFlag flags, void* data);

protected:
	void loadScanConfigs(DaqInChanDescriptor chanDescriptors[], int numChans) const;
	std::vector<CalCoef> getScanCalCoefs(DaqInChanDescriptor chanDescriptors[], int numChans, DaqInScanFlag flags) const;
	std::vector<CustomScale> getCustomScales(DaqInChanDescriptor chanDescriptors[], int numChans) const;

private:
	void setScanConfig(FunctionType functionType, int chanCount, unsigned int scanCount, double rate, ScanOption options, DaqInScanFlag flags);
	unsigned char getOptionsCode(FunctionType functionType, ScanOption options, DaqInScanFlag flags) const;

private:
	enum { FIFO_SIZE = 8 * 4 * 1024 }; // samples size is 4
	enum { CMD_IN_SCAN_START = 0x12, CMD_IN_SCAN_STOP = 0x13, CMD_IN_SCAN_CONFIG = 0x14, CMD_SCAN_CLEARFIFO = 0x15};

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

#endif /* USB_DAQI_DAQIUSB1808_H_ */
