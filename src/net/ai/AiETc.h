/*
 * AiETc.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AI_AIETC_H_
#define NET_AI_AIETC_H_

#include "AiNetBase.h"

namespace ul
{

class UL_LOCAL AiETc: public AiNetBase
{
public:
	AiETc(const NetDaqDevice& daqDevice);
	virtual ~AiETc();

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

protected:
	virtual void loadAdcCoefficients() {};
	virtual int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const { return 0; }
	virtual void readCalDate();

private:

	enum { CMD_TIN = 0x10, CMD_CJC = 0x11, CMD_TIN_CONFIG_R = 0x12, CMD_TIN_CONFIG_W = 0x13,
		   CMD_MEASURE_CONFIG_R = 0x16, CMD_MEASURE_CONFIG_W = 0x17, CMD_MEASURE_MODE_R = 0x18, CMD_MEASURE_MODE_W = 0x19,
		   CMD_FACTORY_CAL_DATE_R = 0x1E, CMD_FIELD_CAL_DATE_R = 0x20};

	enum { NORMAL_MEASURE_MODE = 0, TEST_MEASURE_MODE = 1};

#pragma pack(1)
	typedef union
	{
		struct
		{
			unsigned char otd	: 1;
			unsigned char cal	: 1;
			unsigned char resv  : 6;
		};
		unsigned char code;
	} TMEASURE_CFG;
#pragma pack()
};
} /* namespace ul */

#endif /* NET_AI_AIETC_H_ */
