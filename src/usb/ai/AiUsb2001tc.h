/*
 * AiUsb2001tc.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB2001TC_H_
#define USB_AI_AIUSB2001TC_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb2001tc: public AiUsbBase
{

public:
	AiUsb2001tc(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb2001tc();
	virtual void initialize();

	virtual void tIn(int channel, TempScale scale, TInFlag flags, double* data);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus) { return ERR_BAD_DEV_TYPE; }
	virtual void stopBackground() { throw UlException(ERR_BAD_DEV_TYPE); }

	virtual void setCfg_ChanTcType(int channel, TcType tcType);
	virtual TcType getCfg_ChanTcType(int channel) const;


protected:
	virtual void loadAdcCoefficients();
	virtual int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const { return 0; }

	virtual void readCalDate();

private:
	void getTcRange(TcType tcType, double* min, double* max) const;
	void setAdcRange(int rangeIndex) const;
	void waitUntilAdcReady() const;
	double calibrateInputData(unsigned int rawData) const;
	double scaleData(double rawData) const;

private:
	enum { CMD_AIN=0x10, CMD_CJCIN = 0x17, CMD_ADCRANGE = 0x18, CMD_AINSTATUS = 0x44};
	enum { RANGE_146PT25_IDX = 3,RANGE_73PT125_IDX = 4};
	enum { ADC_READY = 0, ADC_BUSY = 1};
	enum {CJC0 = 0x80};

	TcType mTcType;
};

} /* namespace ul */

#endif /* USB_AI_AIUSB2001TC_H_ */
