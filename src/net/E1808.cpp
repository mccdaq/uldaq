/*
 * E1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "E1808.h"
#include "./ai/AiE1808.h"

namespace ul
{

E1808::E1808(const DaqDeviceDescriptor& daqDeviceDescriptor) : VirNetDaqDevice(daqDeviceDescriptor)
{
	mDaqDeviceInfo.setClockFreq(100000000);

	/*setDaqIDevice(new DaqIUsb1808(*this));
	setDaqODevice(new DaqOUsb1808(*this));*/

	setAiDevice(new AiE1808(*this));
	/*setAoDevice(new AoUsb1808(*this, 2));
	setDioDevice(new DioUsb1808(*this));
	setCtrDevice(new CtrUsb1808(*this, 4));
	setTmrDevice(new TmrUsb1808(*this, 2));*/

	/*setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0010);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0x0008);
	setScanDoneBitMask(0x40);*/

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);

	//setMultiCmdMem(false);
	setMemUnlockAddr(0x8000);
	setMemUnlockCode(0xAA55);

	addMemRegion(MR_CAL, 0x7000, 278, MA_READ);
	addMemRegion(MR_USER, 0x7200, 3584, MA_READ | MA_WRITE);

}

E1808::~E1808()
{
	// TODO Auto-generated destructor stub
}

} /* namespace ul */
