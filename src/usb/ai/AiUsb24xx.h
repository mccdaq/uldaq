/*
 * AiUsb24xx.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AI_AIUSB24XX_H_
#define USB_AI_AIUSB24XX_H_

#include "AiUsbBase.h"

namespace ul
{

class UL_LOCAL AiUsb24xx: public AiUsbBase
{
public:
	AiUsb24xx(const UsbDaqDevice& daqDevice);
	virtual ~AiUsb24xx();
	virtual void initialize();

	virtual void tIn(int channel, TempScale scale, TInFlag flags, double* data);
	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	virtual double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);

	virtual UlError checkScanState(bool* scanDone = NULL) const;

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_ChanType(int channel, AiChanType chanType);
	virtual AiChanType getCfg_ChanType(int channel) const;

	virtual void setCfg_ChanTcType(int channel, TcType tcType);
	virtual TcType getCfg_ChanTcType(int channel) const;

	virtual void setCfg_ChanDataRate(int channel, double rate);
	virtual double getCfg_ChanDataRate(int channel) const;

	virtual void setCfg_ChanOpenTcDetectionMode(int channel, OtdMode mode);
	virtual OtdMode getCfg_ChanOpenTcDetectionMode(int channel) const;

	virtual void updateScanParam(int param);

protected:
	virtual void setTransferMode(ScanOption scanOptions, double rate);
	virtual void loadAdcCoefficients();
	virtual int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const;
	virtual void initCustomScales();

	virtual CalCoef getCalCoef(int channel, AiInputMode inputMode, Range range, long long flags) const;

	virtual void readCalDate();

	virtual void check_AIn_Args(int channel, AiInputMode inputMode, Range range, AInFlag flags) const;
	virtual void check_AInScan_Args(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]) const;
	virtual void check_AInLoadQueue_Args(const AiQueueElement queue[], unsigned int numElements) const;

	virtual void check_TIn_Args(int channel, TempScale scale, TInFlag flags) const;

private:
	void initChanConfig();
	void initChanNums();
	double getChanDataRate(int chanRateIndex);
	int mapRangeCode(Range range) const;
	int mapModeCode(int channel, AiInputMode mode) const;

	void setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options);
	void loadAInScanQueue(AiInputMode inputMode, Range range, int lowChan, int highChan);
	unsigned int calcPacerPeriod(int lowChan, int highChan, double rate);
	double calcMaxRate(int lowChan, int highChan);

	int convertI24ToI32(int i24);
	unsigned int convertToU32(int i32);

	void updateCjcValues();
	void copyCjcValues(double cjcValues[32]);

	void addSupportedRanges();
	void addQueueInfo();

	void virtual processScanData32(libusb_transfer* transfer);

private:
	bool mHadExp;
	int mActualChanCount;
	int mActualCjcCount;
	bool mScanHasTcChan;
	timeval mLastCjcUpdateTime;
	int mActualChanNum[64];
	double mCjcTemps[8];
	double mChanCjcVal[32];
	double mCjcCorrectionValues[32];
	int mFieldCalDateAddr;

	mutable pthread_mutex_t mCjcsMutex;

	enum { FIFO_SIZE = 4 * 512 };
	enum { CMD_AIN = 0x10, CMD_AINSCAN_START = 0x11, CMD_AINSTOP = 0x12, CMD_AINSCAN_STATUS = 0x13, CMD_AINSCAN_QUEUE = 0x14, CMD_CJC = 0x42/*, CMD_AIN_CONFIG = 0x14, CMD_AINSCAN_CLEAR_FIFO = 0x15, CMD_SETTRIG = 0x43*/};
	enum { CHR_30000 = 0, CHR_15000 = 1, CHR_7500 = 2, CHR_3750 = 3, CHR_2000 = 4, CHR_1000 = 5, CHR_500 = 6,
		   CHR_100 = 7, CHR_60 = 8, CHR_50 = 9, CHR_30 = 10, CHR_25 = 11, CHR_15 = 12, CHR_10 = 13, CHR_5 = 14, CHR_2PT5 = 15 };

	enum {MODE_DIFF = 0, MODE_SE_HI = 1, MODE_SE_LO = 2, MODE_DAC_READ = 3, MODE_TC = 4, MODE_TC_NO_OTD = 10 };

	struct
	{
		AiChanType chanType;
		double chanDataRate;
		unsigned int chanDataRateIdx;
		TcType tcType;
		bool detectOpenTc;
	} mChanCfg[64];



#pragma pack(1)

	typedef struct
	{
		unsigned char  channel;
		unsigned char mode;
		unsigned char  range;
		unsigned char rate;
	} TCHAN_CFG;

	typedef union
	{
		TCHAN_CFG cfg;
		struct
		{
			unsigned short wValue;
			unsigned short wIndex;
		} raw;
	} TAIN_CFG;

	struct TAINSCAN_CFG
	{
		unsigned int pacer_period;
		unsigned short scan_count;
		unsigned char packet_size;
	} mScanConfig;

#pragma pack()

	struct
	{
		int channel;
		AiChanType chanType;
		Range range;
		TcType tcType;
		bool detectOpenTc;
	} mScanChanInfo[64];
};
} /* namespace ul */

#endif /* USB_AI_AIUSB24XX_H_ */
