/*
 * DioETc.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_DIO_DIOETC_H_
#define NET_DIO_DIOETC_H_

#include "DioE1608.h"

namespace ul
{

class UL_LOCAL DioETc: public DioE1608
{
public:
	DioETc(const NetDaqDevice& daqDevice);
	virtual ~DioETc();

	virtual void dClearAlarm(DigitalPortType portType, unsigned long long mask);

protected:
	virtual void readAlarmMask();

private:
	enum { CMD_ALARM_CONFIG_R = 0x28, CMD_ALARM_STATUS_W = 0x2B};
};
} /* namespace ul */

#endif /* NET_DIO_DIOETC_H_ */
