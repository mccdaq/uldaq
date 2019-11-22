/*
 * DaqOUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_DAQO_DAQOUSBBASE_H_
#define USB_DAQO_DAQOUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../DaqODevice.h"
#include "../UsbScanTransferOut.h"

namespace ul
{

class UL_LOCAL DaqOUsbBase: public DaqODevice
{
public:
	DaqOUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~DaqOUsbBase();

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

	void setChanDescriptors(DaqOutChanDescriptor* chanDescs, int chanCount);

	virtual unsigned int processScanData(void* transfer, unsigned int stageSize);

private:
	virtual unsigned int processScanData16_dbl(libusb_transfer* transfer, unsigned int stageSize);
	virtual unsigned int processScanData16_uint64(libusb_transfer* transfer, unsigned int stageSize);
	virtual unsigned int processScanData32_dbl(libusb_transfer* transfer, unsigned int stageSize);
	virtual unsigned int processScanData32_uint64(libusb_transfer* transfer, unsigned int stageSize);
	virtual unsigned int processScanData64_uint64(libusb_transfer* transfer, unsigned int stageSize);

private:
	const UsbDaqDevice&  mUsbDevice;
	int mScanEndpointAddr;
	int mTransferMode;
	unsigned char mScanStopCmd;

	DaqOutChanDescriptor mChanDescs[MAX_CHAN_COUNT];
};

} /* namespace ul */

#endif /* USB_DAQO_DAQOUSBBASE_H_ */
