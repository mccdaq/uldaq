/*
 * AoUsb26xx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb26xx.h"

namespace ul
{
AoUsb26xx::AoUsb26xx(const UsbDaqDevice& daqDevice, int numChans) : AoUsb1208hs(daqDevice, numChans)
{
	mAoInfo.setResolution(16);
	mAoInfo.setMaxScanRate(1000000);
	mAoInfo.setMaxThroughput(1000000 * numChans);
	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefCount(numChans);
	mAoInfo.setCalCoefsStartAddr(0x7580);
	mAoInfo.setCalDateAddr(0x75A6);
}

AoUsb26xx::~AoUsb26xx()
{

}

} /* namespace ul */
