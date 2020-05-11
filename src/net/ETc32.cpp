/*
 * ETc32.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "ETc32.h"
#include "./ai/AiETc32.h"
#include "./dio/DioETc32.h"

namespace ul
{
ETc32::ETc32(const DaqDeviceDescriptor& daqDeviceDescriptor) : NetDaqDevice(daqDeviceDescriptor)
{
	FnLog log("ETc32::ETc32");

	setAiDevice(new AiETc32(*this));
	setDioDevice(new DioETc32(*this));

	addMemRegion(MR_USER, 0, 3839, MA_READ | MA_WRITE); // 0xEFF is not a valid address
	addMemRegion(MR_SETTINGS, 0, 32, MA_READ | MA_WRITE);
}

ETc32::~ETc32()
{
	FnLog log("ETc32::~ETc32");
}

void ETc32::initilizeHardware() const
{
	unsigned short status = readStatus();
	mHasExp = false;

	if(status & 0x01)
		mHasExp = true;

	readMeasurementFwVersions();
}

void ETc32::readMeasurementFwVersions() const
{
	unsigned short fwVers[6];
	memset(fwVers, 0, sizeof(fwVers));

	queryCmd(CMD_VERSION, NULL, 0, (unsigned char*) fwVers, sizeof(fwVers));

	const_cast<ETc32*>(this)->mRawFwMeasurementVersion = fwVers[2];
	const_cast<ETc32*>(this)->mRawFwExpMeasurementVersion = fwVers[4];
}

unsigned char ETc32::getMemCmd(MemRegion memRegionType, bool writeAccess) const
{
	unsigned char cmd = 0;

	if(writeAccess)
	{
		if(memRegionType == MR_USER)
			cmd = CMD_USER_MEM_W;
		else if(memRegionType == MR_SETTINGS)
			cmd = CMD_SETTINGS_MEM_W;
		else
			throw UlException(ERR_BAD_MEM_REGION);
	}
	else
	{
		if(memRegionType == MR_CAL)
			cmd = CMD_CAL_MEM_R;
		else if(memRegionType == MR_USER)
			cmd = CMD_USER_MEM_R;
		else if(memRegionType == MR_SETTINGS)
			cmd = CMD_SETTINGS_MEM_R;
		else
			throw UlException(ERR_BAD_MEM_REGION);
	}

	return cmd;
}
} /* namespace ul */
