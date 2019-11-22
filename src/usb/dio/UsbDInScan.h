/*
 * UsbDInScan.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_USBDINSCAN_H_
#define USB_DIO_USBDINSCAN_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL UsbDInScan: public DioUsbBase
{
public:
	UsbDInScan(const UsbDaqDevice& daqDevice);
	virtual ~UsbDInScan();

	virtual double dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[]);

	virtual UlError getInputStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone = NULL) const;
	virtual void setScanState(ScanStatus state);

	int getScanEndpointAddr() const;

protected:
	int calcStageSize(int epAddr, double rate, int portCount, int sampleCount, int sampleSize) const;
	void setTransferMode(ScanOption scanOptions, double rate);
	int getTransferMode() const;

	void setScanEndpointAddr(int addr);

	void setScanStopCmd(unsigned char cmd) { mScanStopCmd = cmd;}
	unsigned char getScanStopCmd() { return mScanStopCmd;}

	virtual void sendStopCmd();

	virtual void processScanData(void* transfer);

private:
	void setScanConfig(int lowPortNum, int highPortNum, unsigned int scanCount, double rate, ScanOption options);
	unsigned char getOptionsCode(ScanOption options) const;

	void virtual processScanData16(libusb_transfer* transfer);

private:
	enum { DIN_SCAN_EP = 0x86 };
	enum { CMD_DIN_SCAN_START = 0x20, CMD_DIN_SCAN_STOP = 0x21, CMD_DIN_SCAN_CLEARFIFO = 0x22, CMD_DIN_SCAN_BULK_FLUSH = 0x23};

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
		unsigned char packet_size;
		unsigned char options;
	} mScanConfig;
#pragma pack()
};

} /* namespace ul */

#endif /* USB_DIO_USBDINSCAN_H_ */
