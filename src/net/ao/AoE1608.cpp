/*
 * AoE1608.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoE1608.h"

namespace ul
{
AoE1608::AoE1608(const NetDaqDevice& daqDevice) : AoNetBase(daqDevice)
{
	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA);

	mAoInfo.hasPacer(false);
	mAoInfo.setNumChans(2);
	mAoInfo.setResolution(16);

	mAoInfo.setCalCoefsStartAddr(0x40);
	mAoInfo.setCalDateAddr(0x50);
	mAoInfo.setCalCoefCount(2);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(BIP10VOLTS);
}

AoE1608::~AoE1608()
{
}

void AoE1608::initialize()
{
	try
	{
		loadDacCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AoE1608::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	check_AOut_Args(channel, range, flags, dataValue);

#pragma pack(1)
	struct
	{
	 unsigned char	chan;
	 unsigned short value;
	}params;
#pragma pack()

	params.chan = channel;
	params.value = calibrateData(channel, range, flags, dataValue);

	daqDev().queryCmd(CMD_AOUT, (unsigned char*) &params, sizeof(params));
}

} /* namespace ul */
