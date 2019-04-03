/*
 * AiUsbBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AIUSBBASE_H_
#define AIUSBBASE_H_

#include "../UsbDaqDevice.h"
#include "../../AiDevice.h"
#include "../UsbScanTransferIn.h"

namespace ul
{

class UL_LOCAL AiUsbBase: public AiDevice
{
public:
	AiUsbBase(const UsbDaqDevice& daqDevice);
	virtual ~AiUsbBase();

	const UsbDaqDevice& daqDev() const {return mUsbDevice;}

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone = NULL) const;

	int getScanEndpointAddr() const;

protected:
	virtual void loadAdcCoefficients();

	int calcStageSize(int epAddr, double rate, int chanCount, int sampleCount) const;
	virtual void setTransferMode(ScanOption scanOptions, double rate);
	int getTransferMode() const;

	void setScanEndpointAddr(int addr);

	void setScanStopCmd(unsigned char cmd) { mScanStopCmd = cmd;}
	unsigned char getScanStopCmd() { return mScanStopCmd;}

	virtual void sendStopCmd();

	virtual void processScanData(void* transfer);

	virtual void readCalDate();

private:
	void virtual processScanData16(libusb_transfer* transfer);
	void virtual processScanData32(libusb_transfer* transfer);

protected:
	int mTransferMode;

private:
	const UsbDaqDevice&  mUsbDevice;
	int mScanEndpointAddr;
	unsigned char mScanStopCmd;
};

} /* namespace ul */

#endif /* AIUSBBASE_H_ */
