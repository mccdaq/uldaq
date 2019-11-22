/*
 * UsbDOutScan.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_USBDOUTSCAN_H_
#define USB_DIO_USBDOUTSCAN_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL UsbDOutScan: public DioUsbBase
{
public:
	UsbDOutScan(const UsbDaqDevice& daqDevice);
	virtual ~UsbDOutScan();

	virtual double dOutScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]);

	virtual UlError getOutputStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone) const;
	virtual void setScanState(ScanStatus state);

	int getScanEndpointAddr() const;

protected:
	int calcStageSize(int epAddr, double rate, int chanCount, int sampleCount, int sampleSize) const;
	void setTransferMode(long long scanOptions, double rate);
	int getTransferMode() const;
	void setScanEndpointAddr(int addr);
	void setScanStopCmd(unsigned char cmd) { mScanStopCmd = cmd;}
	unsigned char getScanStopCmd() { return mScanStopCmd;}

	virtual unsigned int processScanData(void* transfer, unsigned int stageSize);

	virtual void sendStopCmd();

private:
	void setScanConfig(int lowPortNum, int highPortNum, unsigned int scanCount, double rate, ScanOption options);
	unsigned char getOptionsCode(ScanOption options) const;

	virtual unsigned int processScanData16(libusb_transfer* transfer, unsigned int stageSize);

private:
	enum { DOUT_SCAN_EP = 0x02 };
	enum { CMD_DOUT_SCAN_START = 0x24, CMD_DOUT_SCAN_STOP = 0x25, CMD_DOUT_SCAN_CLEARFIFO = 0x26};

private:
	int mScanEndpointAddr;
	int mTransferMode;
	unsigned char mScanStopCmd;

#pragma pack(1)
	struct TDINSCAN_CFG
	{
		unsigned char chan_map;
		unsigned int scan_count;
		unsigned int retrig_count;
		unsigned int pacer_period;
		unsigned char options;
	} mScanConfig;
#pragma pack()
};

} /* namespace ul */

#endif /* USB_DIO_USBDOUTSCAN_H_ */
