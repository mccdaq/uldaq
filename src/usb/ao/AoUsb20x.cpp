/*
 * AoUsb20x.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb20x.h"

namespace ul
{
AoUsb20x::AoUsb20x(const UsbDaqDevice& daqDevice, int numChans) : AoUsbBase(daqDevice)
{
	mAoInfo.hasPacer(false);
	mAoInfo.setNumChans(numChans);
	mAoInfo.setResolution(12);

	mAoInfo.setCalCoefCount(0);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(UNI5VOLTS);
}

AoUsb20x::~AoUsb20x()
{
}

void AoUsb20x::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	check_AOut_Args(channel, range, flags, dataValue);

	unsigned short calData = calibrateData(channel, range, flags, dataValue);

	daqDev().sendCmd(CMD_AOUT, calData, channel, NULL, 0);
}

} /* namespace ul */
