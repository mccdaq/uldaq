/*
 * DaqIUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DAQI_DAQIUSBBASE_H_
#define USB_DAQI_DAQIUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../DaqIDevice.h"
#include "../UsbScanTransferIn.h"

namespace ul
{

class UL_LOCAL DaqIUsbBase: public DaqIDevice
{
public:
	DaqIUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~DaqIUsbBase();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone = NULL) const;

	int getScanEndpointAddr() const;

protected:
	int calcStageSize(int epAddr, double rate, int chanCount, int sampleCount, int sampleSize) const;
	void setTransferMode(long long scanOptions, double rate);
	int getTransferMode() const;

	void setScanEndpointAddr(int addr);

	void setScanStopCmd(unsigned char cmd) { mScanStopCmd = cmd;}
	unsigned char getScanStopCmd() { return mScanStopCmd;}

	virtual void sendStopCmd();

	virtual void processScanData(void* transfer);

	private:
		virtual void processScanData16_dbl(libusb_transfer* transfer);
		virtual void processScanData16_uint64(libusb_transfer* transfer);
		virtual void processScanData32_dbl(libusb_transfer* transfer);
		virtual void processScanData32_uint64(libusb_transfer* transfer);
		virtual void processScanData64_uint64(libusb_transfer* transfer);

private:
	const UsbDaqDevice&  mUsbDevice;
	int mScanEndpointAddr;
	int mTransferMode;
	unsigned char mScanStopCmd;
};

} /* namespace ul */

#endif /* USB_DAQI_DAQIUSBBASE_H_ */
