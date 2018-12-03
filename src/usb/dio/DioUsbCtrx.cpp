/*
 * DioUsbCtrx.cpp
 *
 *     Author: Measurement Computing Corporation
 */


#include "DioUsbCtrx.h"
#include "../daqi/DaqIUsbCtrx.h"

namespace ul
{

DioUsbCtrx::DioUsbCtrx(const UsbDaqDevice& daqDevice) : DioUsb1608g(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;
	mDioInfo.hasPacer(DD_INPUT, true);
	mDioInfo.hasPacer(DD_OUTPUT, false);

	mDioInfo.setScanFlags(DD_INPUT, 0);

	mDioInfo.setScanOptions(DD_INPUT, SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mDioInfo.setTriggerTypes(DD_INPUT, TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mDioInfo.setMinScanRate(DD_INPUT, minRate);

	mDioInfo.setMaxScanRate(DD_INPUT, 4000000);
	mDioInfo.setMaxThroughput(DD_INPUT, 4000000);

	mDioInfo.setFifoSize(DD_INPUT, FIFO_SIZE);
}

DioUsbCtrx::~DioUsbCtrx()
{

}

double DioUsbCtrx::dInScan(DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double rate, ScanOption options, DInScanFlag flags, unsigned long long data[])
{
	check_DInScan_Args(lowPort, highPort, samplesPerPort, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsbCtrx* daqIDev = dynamic_cast<DaqIUsbCtrx*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		DaqInChanDescriptor chanDescriptors;

		chanDescriptors.channel = AUXPORT;
		chanDescriptors.type = DAQI_DIGITAL;

		actualRate =  daqIDev->daqInScan(FT_DI, &chanDescriptors, 1, samplesPerPort, rate, options, (DaqInScanFlag) flags, data);
	}

	return actualRate;
}

UlError DioUsbCtrx::getStatus(ScanDirection direction, ScanStatus* status, TransferStatus* xferStatus)
{
	if(direction == SD_INPUT)
		return mDaqDevice.daqIDevice()->getStatus(FT_DI, status, xferStatus);
	else
		return ERR_BAD_DEV_TYPE;
}

UlError DioUsbCtrx::waitUntilDone(ScanDirection direction, double timeout)
{
	if(direction == SD_INPUT)
		return mDaqDevice.daqIDevice()->waitUntilDone(FT_DI, timeout);
	else
		return ERR_BAD_DEV_TYPE;
}


void DioUsbCtrx::stopBackground(ScanDirection direction)
{
	if(direction == SD_INPUT)
		mDaqDevice.daqIDevice()->stopBackground(FT_DI);
}

ScanStatus DioUsbCtrx::getScanState(ScanDirection direction) const
{
	if(direction == SD_INPUT)
		return mDaqDevice.daqIDevice()->getScanState();
	else
		return SS_IDLE;
}

} /* namespace ul */
