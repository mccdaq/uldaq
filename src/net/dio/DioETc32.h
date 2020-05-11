/*
 * DioETc32.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_DIO_DIOETC32_H_
#define NET_DIO_DIOETC32_H_

#include "DioNetBase.h"

namespace ul
{

class UL_LOCAL DioETc32: public DioNetBase
{
public:
	DioETc32(const NetDaqDevice& daqDevice);
	virtual ~DioETc32();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual void dClearAlarm(DigitalPortType portType, unsigned long long mask);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual unsigned long long getCfg_PortDirectionMask(unsigned int portNum) const;

	protected:
		void readAlarmMask();

private:
	std::bitset<32> mAlarmMask[2];
	enum {CMD_DIN = 0x00, CMD_DOUT_R = 0x02, CMD_DOUT_W = 0x03, CMD_ALARM_CONFIG_R = 0x20, CMD_ALARM_STATUS_W = 0x23};
};

} /* namespace ul */

#endif /* NET_DIO_DIOETC32_H_ */
