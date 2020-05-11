/*
 * AiETc32.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AI_AIETC32_H_
#define NET_AI_AIETC32_H_

#include "AiNetBase.h"

namespace ul
{

class UL_LOCAL AiETc32: public AiNetBase
{
public:
	AiETc32(const NetDaqDevice& daqDevice);
	virtual ~AiETc32();

	virtual void initialize();
	virtual void tIn(int channel, TempScale scale, TInFlag flags, double* data);
	virtual void tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]);

	void enableAllChannels();
	void setMeasureMode(int mode);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_ChanType(int channel, AiChanType chanType);
	virtual AiChanType getCfg_ChanType(int channel) const;
	virtual void setCfg_ChanTcType(int channel, TcType tcType);
	virtual TcType getCfg_ChanTcType(int channel) const;
	virtual void setCfg_OpenTcDetectionMode(int dev, OtdMode mode);
	virtual OtdMode getCfg_OpenTcDetectionMode(int dev) const;
	virtual void setCfg_CalTableType(int dev, AiCalTableType calTableType);
	virtual AiCalTableType getCfg_CalTableType(int dev) const;
	virtual void setCfg_RejectFreqType(int dev, AiRejectFreqType calTableType);
	virtual AiRejectFreqType getCfg_RejectFreqType(int dev) const;
	virtual unsigned long long getCfg_ExpCalDate(int calTableIndex);
	virtual void getCfg_ExpCalDateStr(int calTableIndex, char* calDate, unsigned int* maxStrLen);

protected:
	virtual void loadAdcCoefficients() {};
	virtual int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const { return 0; }
	virtual void readCalDate();

	virtual void check_TInArray_Args(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]) const;

private:
	int mActualChanCount;
	int mActualCjcCount;
	unsigned long long mExpFactoryCalDate; // cal date in sec
	unsigned long long mTc32FieldCalDates[4]; // cal date in sec

	enum { CMD_TIN = 0x10, CMD_CJC = 0x11, CMD_TIN_MULTI = 0x12, CMD_CJC_MULTI = 0x13, CMD_TIN_CONFIG_R = 0x14, CMD_TIN_CONFIG_W = 0x15,
		   CMD_MEASURE_CONFIG_R = 0x18, CMD_MEASURE_CONFIG_W = 0x19, CMD_MEASURE_MODE_R = 0x1A, CMD_MEASURE_MODE_W = 0x1B,
		   CMD_FACTORY_CAL_DATE_R = 0x3A, CMD_FIELD_CAL_DATE_R = 0x3C};

	enum { NORMAL_MEASURE_MODE = 0, TEST_MEASURE_MODE = 1};
	enum { MAX_UNIT_CHAN_COUNT = 32 };

#pragma pack(1)
	typedef union
	{
		struct
		{
			unsigned char otd	 : 1;
			unsigned char filter : 1;
			unsigned char cal	 : 1;
			unsigned char resv   : 5;
		};
		unsigned char code;
	} TMEASURE_CFG;
#pragma pack()
};
} /* namespace ul */

#endif /* NET_AI_AIETC32_H_ */
