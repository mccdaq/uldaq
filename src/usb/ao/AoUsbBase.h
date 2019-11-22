/*
 * AoUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSBBASE_H_
#define USB_AO_AOUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../AoDevice.h"
#include "../UsbScanTransferOut.h"

namespace ul
{

class UL_LOCAL AoUsbBase: public AoDevice
{
public:
	AoUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~AoUsbBase();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone) const;

	int getScanEndpointAddr() const;

protected:
	virtual void loadDacCoefficients();

	int calcStageSize(int epAddr, double rate, int chanCount, int sampleCount) const;
	void setTransferMode(long long scanOptions, double rate);
	int getTransferMode() const;
	void setScanEndpointAddr(int addr);
	void setScanStopCmd(unsigned char cmd) { mScanStopCmd = cmd;}
	unsigned char getScanStopCmd() { return mScanStopCmd;}

	virtual unsigned int processScanData(void* transfer, unsigned int stageSize);

	virtual void readCalDate();

	virtual void sendStopCmd();

private:
	virtual unsigned int processScanData16(libusb_transfer* transfer, unsigned int stageSize);
	virtual unsigned int processScanData32(libusb_transfer* transfer, unsigned int stageSize);

protected:
	int mTransferMode;

private:
	const UsbDaqDevice&  mUsbDevice;
	int mScanEndpointAddr;
	unsigned char mScanStopCmd;
};

} /* namespace ul */

#endif /* USB_AO_AOUSBBASE_H_ */
