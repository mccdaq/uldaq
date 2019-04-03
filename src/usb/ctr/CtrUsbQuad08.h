/*
 * CtrUsbQuad08.h
 *
 * Author: Measurement Computing Corporation
 */

#ifndef USB_CTR_CTRUSBQUAD08_H_
#define USB_CTR_CTRUSBQUAD08_H_

#include "CtrUsbBase.h"
#include "../UsbScanTransferIn.h"
#include "../../utility/UlLock.h"

namespace ul
{

class UL_LOCAL CtrUsbQuad08: public CtrUsbBase
{
public:
	CtrUsbQuad08(const UsbDaqDevice& daqDevice, int numCtrs);
	virtual ~CtrUsbQuad08();

	virtual void initialize();

	virtual unsigned long long cIn(int ctrNum);
	virtual void cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue);
	virtual void cClear(int ctrNum);
	virtual unsigned long long cRead(int ctrNum, CounterRegisterType regType);
	virtual double cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[]);

	virtual void cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
								CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
								CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();
	virtual UlError terminateScan();
	virtual UlError checkScanState(bool* scanDone = NULL) const;

	int getScanEndpointAddr() const;
	virtual void updateScanParam(int param);

protected:
	int calcStageSize(int epAddr, double rate, int ctrCount, int sampleCount,int sampleSize) const;
	void setScanEndpointAddr(int addr);
	virtual void sendStopCmds();
	virtual void processScanData(void* transfer);


private:
	void virtual processScanData16(libusb_transfer* transfer);
	void virtual processScanData32(libusb_transfer* transfer);
	void virtual processScanData64(libusb_transfer* transfer);



	void setScanListFifoCfg(int ctrNum, bool firstCtr, bool lastCtr, CInScanFlag flags);
	unsigned short getScanListWord1(int ctrNum, bool ssh, bool lastCtr);
	unsigned long long calcPacerDevisor(double rate, ScanOption options);
	void setupPacerClock(double rate, ScanOption options);
	unsigned short getTrigModeCode();

private:
	unsigned char getDebounceOptionCode(CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CounterEdgeDetection edgeDetection) const;

	void setCounterSetupReg(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode, CounterTickSize tickSize);
	void setDebounceSetupReg(int ctrNum, CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CounterEdgeDetection edgeDetection);
	void setModuloReg(int ctrNum, unsigned long long value);

	unsigned char getModeCode(CounterMeasurementType measureType) const;
	unsigned char getOptionCode(CounterMeasurementType measureType, CounterMeasurementMode measureMode) const;
	unsigned char getTickSizeCode(CounterMeasurementType measureType, CounterTickSize tickSize) const;

	void addSupportedTickSizes();
	void addSupportedDebounceTimes();

private:
	enum { FIFO_SIZE = 8 * 2 * 1024 }; // not an actual fifo size
	enum { MODULO_REG_1 = 0x10, MODULO_REG_2 = 0x20, MODULO_REG_3 = 0x30 };

	int mScanEndpointAddr;

	pthread_mutex_t mCtrSelectMutex;

	struct
	{
		bool asyncMode;
		CounterMeasurementType measureType;
		CounterMeasurementMode measureMode;
		CounterEdgeDetection edgeDetection;
		CounterTickSize tickSize;
		CounterDebounceMode debounceMode;
		CounterDebounceTime debounceTime;
		bool rangeLimitEnabled;
		unsigned long long maxLimitVal;
	}mCounterConfig[8];

	bool mFirstDataPacketReceived;
	bool mDisableHwTrigger;
};

} /* namespace ul */

#endif /* USB_CTR_CTRUSBQUAD08_H_ */
