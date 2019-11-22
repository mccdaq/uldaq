/*
 * CtrUsb9837x.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "CtrUsb9837x.h"
#include "../daqi/DaqIUsb9837x.h"

namespace ul
{
CtrUsb9837x::CtrUsb9837x(const UsbDaqDevice& daqDevice, int numCtrs) : CtrUsbBase(daqDevice)
{
	double minRate = 195.313;

	mCtrInfo.hasPacer(true);
	mCtrInfo.setResolution(32);

	mCtrInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_SINGLEIO|SO_BLOCKIO|SO_EXTTIMEBASE|SO_TIMEBASEOUT);
	mCtrInfo.setCInScanFlags(CINSCAN_FF_CTR32_BIT /*| CINSCAN_FF_NOCLEAR*/);
	mCtrInfo.setTriggerTypes(TRIG_POS_EDGE | TRIG_RISING);

	mCtrInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
	{
		mCtrInfo.setMaxScanRate(105469.0);
		mCtrInfo.setMaxThroughput(105469.0 * numCtrs); // for daqinscan add tachs to the throughput calc

		mCtrInfo.setFifoSize(FIFO_SIZE * 2);
	}
	else
	{
		mCtrInfo.setMaxScanRate(52734.0);
		mCtrInfo.setMaxThroughput(52734.0 * numCtrs);

		mCtrInfo.setFifoSize(FIFO_SIZE);
	}

	mCtrInfo.addCtr(CMT_PERIOD);  // ctr 0
	mCtrInfo.addCtr(CMT_PERIOD | CMT_PULSE_WIDTH);  // ctr 1
	mCtrInfo.addCtr(CMT_PERIOD | CMT_PULSE_WIDTH);  // ctr 2

	mCtrInfo.setCtrMeasurementModes(CMT_COUNT , CMM_DEFAULT);
	mCtrInfo.setCtrMeasurementModes(CMT_PERIOD ,CMM_PERIOD_X1);
	mCtrInfo.setCtrMeasurementModes(CMT_PULSE_WIDTH , CMM_PULSE_WIDTH_DEFAULT);


	//mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD | CRT_MIN_LIMIT | CRT_MAX_LIMIT);

	//addSupportedTickSizes();
	//addSupportedDebounceTimes();
}

CtrUsb9837x::~CtrUsb9837x()
{

}

void CtrUsb9837x::initialize()
{
	/*try
	{
		// Note: The initial value of General Control Register 4 is stored in EEPROM at addresses 0x0C and 0x0D (OpenLayer control panel
		// can be used to change the value), the firmware applies the values stored at those addresses to GCR4 at power up.
		// see General_Control_Reg_Init() in UsbProcessorConfig.c

		// read General Control Register 4
		dtDev().Cmd_ReadSingleWordFromLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG4, &mReg4Val);
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}*/
}

double CtrUsb9837x::cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[])
{
	check_CInScan_Args(lowCtrNum, highCtrNum, samplesPerCounter, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsb9837x* daqIDev = dynamic_cast<DaqIUsb9837x*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		int numCtrs = highCtrNum - lowCtrNum + 1;

		DaqInChanDescriptor* chanDescriptors = new DaqInChanDescriptor[numCtrs];

		for(int i = 0; i < numCtrs; i++)
		{
			chanDescriptors[i].channel = lowCtrNum + i;
			chanDescriptors[i].type = DAQI_CTR32;
		}

		DaqInScanFlag daqInScanflags = (DaqInScanFlag) (flags & NOCLEAR); // only pass "no clear" flag to daqinscan if it is set

		actualRate =  daqIDev->daqInScan(FT_CTR, chanDescriptors, numCtrs, samplesPerCounter, rate, options, daqInScanflags, data);

		delete [] chanDescriptors;
	}

	return actualRate;
}

void CtrUsb9837x::check_CtrSetTrigger_Args(TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const
{
	CtrDevice::check_CtrSetTrigger_Args(trigType, trigChan, level, variance, retriggerCount);

	if(trigType & TRIG_RISING)
	{
		if(daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_A ||
		   daqDev().getDeviceType() == DaqDeviceId::UL_DT9837_B)
		{
			if(trigChan != 0)
				throw UlException(ERR_BAD_TRIG_CHANNEL);

			double VoltageRangeHigh = 9.8;
			double VoltageRangeLow = 0.2;

			if ((level >= VoltageRangeHigh) || (level <= VoltageRangeLow))
				throw UlException(ERR_BAD_TRIG_LEVEL);
		}
	}
}

UlError CtrUsb9837x::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return mDaqDevice.daqIDevice()->getStatus(FT_CTR, status, xferStatus);
}

UlError CtrUsb9837x::waitUntilDone(double timeout)
{
	return mDaqDevice.daqIDevice()->waitUntilDone(FT_CTR, timeout);
}


void CtrUsb9837x::stopBackground()
{
	mDaqDevice.daqIDevice()->stopBackground(FT_CTR);
}

ScanStatus CtrUsb9837x::getScanState() const
{
	return mDaqDevice.daqIDevice()->getScanState();
}

void CtrUsb9837x::setCfg_CtrReg(int ctrNum, long long regVal)
{
	if(regVal > 0xFFFF)
		throw UlException(ERR_BAD_CONFIG_VAL);

	unsigned short readOnlyMask = (1 << 2) | (1 << 5) | (1 << 9); // bits 2, 5 and 9 are read-only
	readOnlyMask = ~readOnlyMask;

	unsigned short reg4Val = regVal & readOnlyMask;

	dtDev().Cmd_WriteSingleWordToLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG4, reg4Val);
}

long long CtrUsb9837x::getCfg_CtrReg(int ctrNum) const
{
	unsigned short reg4Val = 0;

	dtDev().Cmd_ReadSingleWordFromLocalBus(Usb9837xDefs::GENERAL_CNTRL_REG4, &reg4Val);

	return reg4Val;
}

} /* namespace ul */
