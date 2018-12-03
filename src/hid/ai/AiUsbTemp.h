/*
 * AiUsbTemp.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_AI_AIUSBTEMP_H_
#define HID_AI_AIUSBTEMP_H_

#include "AiHidBase.h"

namespace ul
{

class UL_LOCAL AiUsbTemp: public AiHidBase
{
public:
	AiUsbTemp(const HidDaqDevice& daqDevice);
	virtual ~AiUsbTemp();

	virtual void initialize();

	virtual void tIn(int channel, TempScale scale, TInFlag flags, double* data);
	virtual void tInArray(int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual AiChanType getCfg_ChanType(int channel) const;
	virtual SensorConnectionType getCfg_SensorConnectionType(int channel) const;

	virtual void getCfg_ChanCoefsStr(int channel, char* coefsStr, unsigned int* len) const;

	virtual void setCfg_ChanTcType(int channel, TcType tcType);
	virtual TcType getCfg_ChanTcType(int channel) const;

protected:
	virtual void loadAdcCoefficients() {};
	virtual int getCalCoefIndex(int channel, AiInputMode inputMode, Range range) const { return 0; }

	unsigned char tcCode(TcType tcType) const;
	TcType tcType(unsigned char tcCode) const;

private:
	enum {CMD_TIN = 0x18, CMD_TINSCAN = 0x19, CMD_SETITEM = 0x49, CMD_GETITEM = 0x4A};
	enum {SUBITEM_SENSOR_TYPE = 0, SUBITEM_CONNECTION_TYPE = 1, SUBITEM_TC_TYPE = 0x10, SUBITEM_COEF0 = 0x14};

	enum {ST_RTD = 0, ST_THERMISTOR = 1, ST_THERMOCOUPLE = 2, ST_SEMICONDUCTOR = 3, ST_DISABLED = 4};
	enum {CT_2W_1S = 0, CT_2W_2S = 1, CT_3W = 2, CT_4W = 3 };
};

} /* namespace ul */

#endif /* HID_AI_AIUSBTEMP_H_ */
