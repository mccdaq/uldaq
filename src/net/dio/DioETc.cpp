/*
 * DioETc.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioETc.h"

namespace ul
{

DioETc::DioETc(const NetDaqDevice& daqDevice) : DioE1608(daqDevice)
{

}

DioETc::~DioETc()
{

}

void DioETc::dClearAlarm(DigitalPortType portType, unsigned long long mask)
{
	check_DOut_Args(portType, mask);

	unsigned char clearMask = mask;

	daqDev().queryCmd(CMD_ALARM_STATUS_W, &clearMask, sizeof(clearMask));
}


void DioETc::readAlarmMask()
{
#pragma pack(1)
	struct
	{
		unsigned char settings[8];
		float threshold1[8];
		float threshold2[8];
	}alarm;
#pragma pack()

	memset(&alarm, 0, sizeof(alarm));
	unsigned char mask = 0;

	daqDev().queryCmd(CMD_ALARM_CONFIG_R, 0, 0, (unsigned char*) &alarm, sizeof(alarm));

	for(int i = 0; i < 8; i++)
	{
		mask |=  (alarm.settings[i] & 0x01) << i;
	}

	mAlarmMask = mask;
}
} /* namespace ul */
