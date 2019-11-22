/*
 * CtrUsb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrUsb1808.h"
#include "../daqi/DaqIUsb1808.h"

namespace ul
{

CtrUsb1808::CtrUsb1808(const UsbDaqDevice& daqDevice, int numCtrs) : CtrUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;
	mCtrInfo.hasPacer(true);
	mCtrInfo.setResolution(32);

	mCtrInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mCtrInfo.setCInScanFlags(CINSCAN_FF_CTR32_BIT | CINSCAN_FF_NOCLEAR);
	mCtrInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_PATTERN_EQ | TRIG_PATTERN_NE | TRIG_PATTERN_ABOVE | TRIG_PATTERN_BELOW);

	mCtrInfo.setMinScanRate(minRate);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_1808X)
	{
		mCtrInfo.setMaxScanRate(200000);
		mCtrInfo.setMaxThroughput(4 * 200000);
	}
	else
	{
		mCtrInfo.setMaxScanRate(50000);
		mCtrInfo.setMaxThroughput(4 * 50000);
	}

	mCtrInfo.setFifoSize(FIFO_SIZE);


	mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH);  // ctr 0
	mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH);  // ctr 1
	mCtrInfo.addCtr(CMT_ENCODER);  // ctr 2
	mCtrInfo.addCtr(CMT_ENCODER);  // ctr 3

	mCtrInfo.setCtrMeasurementModes(CMT_COUNT , CMM_CLEAR_ON_READ | CMM_NO_RECYCLE | CMM_COUNT_DOWN | CMM_RANGE_LIMIT_ON);
	mCtrInfo.setCtrMeasurementModes(CMT_PERIOD , CMM_PERIOD_X1 | CMM_PERIOD_X10 | CMM_PERIOD_X100 | CMM_PERIOD_X1000);
	mCtrInfo.setCtrMeasurementModes(CMT_PULSE_WIDTH , CMM_PULSE_WIDTH_DEFAULT);
	mCtrInfo.setCtrMeasurementModes(CMT_ENCODER , CMM_ENCODER_X1 | CMM_ENCODER_X2 | CMM_ENCODER_X4 | CMM_ENCODER_LATCH_ON_Z | CMM_ENCODER_CLEAR_ON_Z | CMM_ENCODER_RANGE_LIMIT_ON | CMM_ENCODER_Z_ACTIVE_EDGE);

	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD | CRT_MIN_LIMIT | CRT_MAX_LIMIT);

	addSupportedTickSizes();
	addSupportedDebounceTimes();
}

CtrUsb1808::~CtrUsb1808()
{

}

void CtrUsb1808::initialize()
{
	initScanCountersState();
}

unsigned long long CtrUsb1808::cIn(int ctrNum)
{
	check_CIn_Args(ctrNum);

	return cRead(ctrNum, CRT_COUNT);
}

void CtrUsb1808::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	unsigned char cmd = CMD_CTR;
	unsigned short index = 0;
	unsigned int regVal = loadValue;

	if(regType == CRT_MIN_LIMIT || regType == CRT_MAX_LIMIT)
		cmd = CMD_LIMIT_VALS;

	if(regType == CRT_MAX_LIMIT)
		index = 1;

	daqDev().sendCmd(cmd, index, ctrNum, (unsigned char*) &regVal, sizeof(regVal));
}

void CtrUsb1808::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrUsb1808::cRead(int ctrNum, CounterRegisterType regType)
{
	unsigned long long count = 0;

	check_CRead_Args(ctrNum, regType);

	unsigned char cmd = CMD_CTR;
	unsigned short index = 0;
	unsigned int regVal = 0;

	if(regType == CRT_MIN_LIMIT || regType == CRT_MAX_LIMIT)
		cmd = CMD_LIMIT_VALS;

	if(regType == CRT_MAX_LIMIT)
		index = 1;

	daqDev().queryCmd(cmd, index, ctrNum, (unsigned char*) &regVal, sizeof(regVal));

	count = Endian::le_ui32_to_cpu(regVal);

	return count;
}

double CtrUsb1808::cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[])
{
	check_CInScan_Args(lowCtrNum, highCtrNum, samplesPerCounter, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsb1808* daqIDev = dynamic_cast<DaqIUsb1808*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		int numCtrs = highCtrNum - lowCtrNum + 1;

		DaqInChanDescriptor* chanDescriptors = new DaqInChanDescriptor[numCtrs];

		for(int i = 0; i < numCtrs; i++)
		{
			chanDescriptors[i].channel = lowCtrNum + i;
			chanDescriptors[i].type = DAQI_CTR32;
		}

		DaqInScanFlag daqInScanflags = (DaqInScanFlag) (flags & NOCLEAR); // only pass no clear flag to daqinscan if it is set

		actualRate =  daqIDev->daqInScan(FT_CTR, chanDescriptors, numCtrs, samplesPerCounter, rate, options, daqInScanflags, data);

		delete [] chanDescriptors;
	}

	return actualRate;
}

void CtrUsb1808::cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
							CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
							CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag)
{
	check_CConfigScan_Args(ctrNum, measureType,  measureMode, edgeDetection, tickSize, debounceMode, debounceTime, flag);

	unsigned char ctrParams[2];

	ctrParams[0] = getModeOptionCode(measureType, measureMode, tickSize);
	ctrParams[1] = getCtrOptionCode(measureType, measureMode, edgeDetection);

	daqDev().sendCmd(CMD_CTR_PARAMS, 0, ctrNum, ctrParams, sizeof(ctrParams));
}

unsigned char CtrUsb1808::getModeOptionCode(CounterMeasurementType measureType, CounterMeasurementMode measureMode, CounterTickSize tickSize) const
{

#pragma pack(1)
	union
	{
		struct
		{
			unsigned char mode		: 2;
			unsigned char res  		: 2;
			unsigned char tickSize	: 2;
			unsigned char reserved	: 2;
		};
		unsigned char optCode;
	}options;
#pragma pack()

	options.optCode = 0;

	if(measureType == CMT_PERIOD)
	{
		options.mode = 1;
		options.res = 0;

		if(measureMode & CMM_PERIOD_X10)
			options.res = 1;
		else if(measureMode & CMM_PERIOD_X100)
			options.res = 2;
		else if(measureMode & CMM_PERIOD_X1000)
			options.res = 3;

		options.tickSize = tickSize - CTS_TICK_20ns;
	}
	else if(measureType == CMT_PULSE_WIDTH)
	{
		options.mode = 2;
		options.tickSize = tickSize - CTS_TICK_20ns;
	}

	return options.optCode;
}

unsigned char CtrUsb1808::getCtrOptionCode(CounterMeasurementType measureType,  CounterMeasurementMode measureMode, CounterEdgeDetection edgeDetection) const
{
#pragma pack(1)
	union
	{
		struct
		{
			unsigned char clearOnRead			: 1;
			unsigned char noRecycleMode			: 1;
			unsigned char countDownOn			: 1;
			unsigned char rangeLimitOn			: 1;
			unsigned char countOnFallingEdge    : 1;
			unsigned char reserved				: 3;
		}counter;
		struct
		{
			unsigned char type					: 2;
			unsigned char clearOnZ				: 1;
			unsigned char latchOnZ				: 1;
			unsigned char noRecycleMode			: 1;
			unsigned char rangeLimitOn			: 1;
			unsigned char zActiveMode			: 1;
			unsigned char reserved				: 1;
		}encoder;

		unsigned char optCode;
	} options;

#pragma pack()
	options.optCode = 0;

	if(measureType != CMT_ENCODER)
	{
		if(measureMode & CMM_CLEAR_ON_READ)
			options.counter.clearOnRead = 1;

		if(measureMode & CMM_NO_RECYCLE)
			options.counter.noRecycleMode = 1;

		if(measureMode & CMM_COUNT_DOWN)
			options.counter.countDownOn = 1;

		if(measureMode & CMM_RANGE_LIMIT_ON)
			options.counter.rangeLimitOn = 1;

		if(edgeDetection == CED_FALLING_EDGE)
			options.counter.countOnFallingEdge = 1;
	}
	else
	{
		if(measureMode & CMM_ENCODER_X2)
			options.encoder.type = 1;
		else if(measureMode & CMM_ENCODER_X4)
			options.encoder.type = 2;

		if(measureMode & CMM_ENCODER_LATCH_ON_Z)
			options.encoder.latchOnZ = 1;

		if(measureMode & CMM_ENCODER_CLEAR_ON_Z)
			options.encoder.clearOnZ = 1;

		if(measureMode & CMM_ENCODER_RANGE_LIMIT_ON)
			options.encoder.rangeLimitOn = 1;

		if(measureMode & CMM_ENCODER_NO_RECYCLE)
			options.encoder.rangeLimitOn = 1;

		if(measureMode & CMM_ENCODER_Z_ACTIVE_EDGE)
		options.encoder.zActiveMode = 1;
	}

	return options.optCode;
}


UlError CtrUsb1808::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return mDaqDevice.daqIDevice()->getStatus(FT_CTR, status, xferStatus);
}

UlError CtrUsb1808::waitUntilDone(double timeout)
{
	return mDaqDevice.daqIDevice()->waitUntilDone(FT_CTR, timeout);
}


void CtrUsb1808::stopBackground()
{
	mDaqDevice.daqIDevice()->stopBackground(FT_CTR);
}

ScanStatus CtrUsb1808::getScanState() const
{
	return mDaqDevice.daqIDevice()->getScanState();
}

void CtrUsb1808::addSupportedTickSizes()
{
	mCtrInfo.addTickSize(CTS_TICK_20ns);
	mCtrInfo.addTickSize(CTS_TICK_200ns);
	mCtrInfo.addTickSize(CTS_TICK_2000ns);
	mCtrInfo.addTickSize(CTS_TICK_20000ns);
}

void CtrUsb1808::addSupportedDebounceTimes()
{
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_1500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_3500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_7500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_15500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_31500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_63500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_127500ns);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_100us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_300us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_700us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_1500us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_3100us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_6300us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_12700us);
	mCtrInfo.addDebounceTime(CDT_DEBOUNCE_25500us);
}

} /* namespace ul */
