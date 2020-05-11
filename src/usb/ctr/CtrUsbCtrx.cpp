/*
 * CtrUsbCtrx.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrUsbCtrx.h"
#include "../daqi/DaqIUsbCtrx.h"

namespace ul
{

CtrUsbCtrx::CtrUsbCtrx(const UsbDaqDevice& daqDevice, int numCtrs) : CtrUsbBase(daqDevice)
{
	double minRate = daqDev().getClockFreq() / UINT_MAX;
	mCtrInfo.hasPacer(true);
	mCtrInfo.setResolution(32);

	mCtrInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_SINGLEIO|SO_BLOCKIO|SO_RETRIGGER);
	mCtrInfo.setCInScanFlags(CINSCAN_FF_CTR16_BIT | CINSCAN_FF_CTR32_BIT | CINSCAN_FF_CTR64_BIT | CINSCAN_FF_NOCLEAR);
	mCtrInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mCtrInfo.setMinScanRate(minRate);

	mCtrInfo.setMaxScanRate(4000000);
	mCtrInfo.setMaxThroughput(4000000);

	mCtrInfo.setFifoSize(FIFO_SIZE);

	mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
	mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
	mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
	mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);

	if(daqDev().getDeviceType() == DaqDeviceId::USB_CTR08)
	{
		mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
		mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
		mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
		mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_TIMING);
	}

	mCtrInfo.setCtrMeasurementModes(CMT_COUNT , CMM_CLEAR_ON_READ | CMM_NO_RECYCLE | CMM_COUNT_DOWN | CMM_RANGE_LIMIT_ON |
									CMM_GATING_ON | CMM_INVERT_GATE | CMM_GATE_CONTROLS_DIR | CMM_GATE_CLEARS_CTR | CMM_GATE_TRIG_SRC |
									CMM_OUTPUT_ON | CMM_OUTPUT_INITIAL_STATE_HIGH);
	mCtrInfo.setCtrMeasurementModes(CMT_PERIOD , CMM_PERIOD_X1| CMM_PERIOD_X10 | CMM_PERIOD_X100 | CMM_PERIOD_X1000 | CMM_PERIOD_GATING_ON | CMM_PERIOD_INVERT_GATE);
	mCtrInfo.setCtrMeasurementModes(CMT_PULSE_WIDTH , CMM_PULSE_WIDTH_GATING_ON | CMM_PULSE_WIDTH_INVERT_GATE);
	mCtrInfo.setCtrMeasurementModes(CMT_TIMING , CMM_TIMING_MODE_INVERT_GATE);


	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD | CRT_MIN_LIMIT | CRT_MAX_LIMIT | CRT_OUTPUT_VAL0 | CRT_OUTPUT_VAL1);

	addSupportedTickSizes();
	addSupportedDebounceTimes();
}

CtrUsbCtrx::~CtrUsbCtrx()
{

}

void CtrUsbCtrx::initialize()
{
	initScanCountersState();
}

unsigned long long CtrUsbCtrx::cIn(int ctrNum)
{
	check_CIn_Args(ctrNum);

	return cRead(ctrNum, CRT_COUNT);
}

void CtrUsbCtrx::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	unsigned char cmd = CMD_CTR;
	unsigned short index = 0;
	unsigned long long regVal = loadValue;

	if(regType == CRT_MIN_LIMIT)
	{
		cmd = CMD_LIMIT_VALS;
	}
	else if(regType == CRT_MAX_LIMIT)
	{
		cmd = CMD_LIMIT_VALS;
		index = 1;
	}
	else if(regType == CRT_OUTPUT_VAL0)
	{
		cmd = CMD_CTR_OUT_VALS;
	}
	else if(regType == CRT_OUTPUT_VAL1)
	{
		cmd = CMD_CTR_OUT_VALS;
		index = 1;
	}

	daqDev().sendCmd(cmd, index, ctrNum, (unsigned char*) &regVal, sizeof(regVal));
}

void CtrUsbCtrx::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrUsbCtrx::cRead(int ctrNum, CounterRegisterType regType)
{
	unsigned long long count = 0;

	check_CRead_Args(ctrNum, regType);

	unsigned char cmd = CMD_CTR;
	unsigned short index = 0;
	unsigned long long regVal = 0;

	if(regType == CRT_MIN_LIMIT)
	{
		cmd = CMD_LIMIT_VALS;
	}
	else if(regType == CRT_MAX_LIMIT)
	{
		cmd = CMD_LIMIT_VALS;
		index = 1;
	}
	else if(regType == CRT_OUTPUT_VAL0)
	{
		cmd = CMD_CTR_OUT_VALS;
	}
	else if(regType == CRT_OUTPUT_VAL1)
	{
		cmd = CMD_CTR_OUT_VALS;
		index = 1;
	}

	daqDev().queryCmd(cmd, index, ctrNum, (unsigned char*) &regVal, sizeof(regVal));

	count = Endian::le_ui64_to_cpu(regVal);

	return count;
}

double CtrUsbCtrx::cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[])
{
	check_CInScan_Args(lowCtrNum, highCtrNum, samplesPerCounter, rate, options, flags, data);

	double actualRate = 0;

	DaqIUsbCtrx* daqIDev = dynamic_cast<DaqIUsbCtrx*>(mDaqDevice.daqIDevice());

	if(daqIDev)
	{
		int numCtrs = highCtrNum - lowCtrNum + 1;

		DaqInChanDescriptor* chanDescriptors = new DaqInChanDescriptor[numCtrs];

		DaqInChanType daqIChanType = DAQI_CTR16;

		if(flags == CINSCAN_FF_CTR32_BIT)
			daqIChanType = DAQI_CTR32;
		else if(flags == CINSCAN_FF_CTR64_BIT)
			daqIChanType = (DaqInChanType) DAQI_CTR64_INTERNAL;

		for(int i = 0; i < numCtrs; i++)
		{
			chanDescriptors[i].channel = lowCtrNum + i;
			chanDescriptors[i].type = daqIChanType;
		}

		DaqInScanFlag daqInScanflags = (DaqInScanFlag) (flags & NOCLEAR); // only pass "no clear" flag to daqinscan if it is set

		actualRate =  daqIDev->daqInScan(FT_CTR, chanDescriptors, numCtrs, samplesPerCounter, rate, options, daqInScanflags, data);

		delete [] chanDescriptors;
	}

	return actualRate;
}

void CtrUsbCtrx::cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
							CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
							CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag)
{
	check_CConfigScan_Args(ctrNum, measureType,  measureMode, edgeDetection, tickSize, debounceMode, debounceTime, flag);

	unsigned char ctrParams[5];

	ctrParams[0] = getModeOptionCode(measureType, measureMode, tickSize);
	ctrParams[1] = getCtrOptionCode(measureMode, edgeDetection);
	ctrParams[2] = getGateOptionCode(measureMode);
	ctrParams[3] = getOutputOptionCode(measureMode);
	ctrParams[4] = getDebounceOptionCode(debounceMode, debounceTime);

	daqDev().sendCmd(CMD_CTR_PARAMS, 0, ctrNum, ctrParams, sizeof(ctrParams));
}

unsigned char CtrUsbCtrx::getModeOptionCode(CounterMeasurementType measureType, CounterMeasurementMode measureMode, CounterTickSize tickSize) const
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

		options.tickSize = tickSize - CTS_TICK_20PT83ns;
	}
	else if(measureType == CMT_PULSE_WIDTH)
	{
		options.mode = 2;
		options.tickSize = tickSize - CTS_TICK_20PT83ns;
	}
	else if(measureType == CMT_TIMING)
	{
		options.mode = 3;
		options.tickSize = tickSize - CTS_TICK_20PT83ns;
	}

	return options.optCode;
}

unsigned char CtrUsbCtrx::getCtrOptionCode(CounterMeasurementMode measureMode, CounterEdgeDetection edgeDetection) const
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
		};

		unsigned char optCode;
	} options;

#pragma pack()
	options.optCode = 0;


	if(measureMode & CMM_CLEAR_ON_READ)
		options.clearOnRead = 1;

	if(measureMode & CMM_NO_RECYCLE)
		options.noRecycleMode = 1;

	if(measureMode & CMM_COUNT_DOWN)
		options.countDownOn = 1;

	if(measureMode & CMM_RANGE_LIMIT_ON)
		options.rangeLimitOn = 1;

	if(edgeDetection == CED_FALLING_EDGE)
		options.countOnFallingEdge = 1;

	return options.optCode;
}

unsigned char CtrUsbCtrx::getGateOptionCode(CounterMeasurementMode measureMode) const
{
#pragma pack(1)
	union
	{
		struct
		{
			unsigned char enableGatePin		: 1;
			unsigned char activeLow			: 1;
			unsigned char gateMode			: 2;
			unsigned char reserved			: 4;
		};
		unsigned char optCode;
	} options;
#pragma pack()

	options.optCode = 0;

	if((measureMode & CMM_GATING_ON) || (measureMode & CMM_PERIOD_GATING_ON) || (measureMode & CMM_PULSE_WIDTH_GATING_ON) ||
	   (measureMode & CMM_GATE_CONTROLS_DIR) || (measureMode & CMM_GATE_CLEARS_CTR) || (measureMode & CMM_GATE_TRIG_SRC))
		options.enableGatePin = 1;

	if((measureMode & CMM_INVERT_GATE) || (measureMode & CMM_PERIOD_INVERT_GATE) || (measureMode & CMM_PULSE_WIDTH_INVERT_GATE) || (measureMode & CMM_TIMING_MODE_INVERT_GATE))
		options.activeLow = 1;

	if(measureMode & CMM_GATE_CONTROLS_DIR)
		options.gateMode = 1;

	if(measureMode & CMM_GATE_CLEARS_CTR)
		options.gateMode = 2;

	if(measureMode & CMM_GATE_TRIG_SRC)
		options.gateMode = 3;

	return options.optCode;
}

unsigned char CtrUsbCtrx::getOutputOptionCode(CounterMeasurementMode measureMode) const
{
#pragma pack(1)
	union
	{
		struct
		{
			unsigned char outputOn	: 1;
			unsigned char initHigh	: 1;
			unsigned char reserved	: 5;
		};
		unsigned char optCode;
	} options;
#pragma pack()

	options.optCode = 0;

	if(measureMode & CMM_OUTPUT_ON)
		options.outputOn = 1;

	if(measureMode & CMM_OUTPUT_INITIAL_STATE_HIGH)
		options.initHigh = 1;

	return options.optCode;
}

unsigned char CtrUsbCtrx::getDebounceOptionCode(CounterDebounceMode debounceMode, CounterDebounceTime debounceTime) const
{
#pragma pack(1)
	union
	{
		struct
		{
			unsigned char time		: 5 ;
			unsigned char mode		: 1;
			unsigned char Reserved	: 2;
		};
		unsigned char optCode;
	} options;
#pragma pack()

	options.optCode = 0;

	if(debounceMode != CDM_NONE)
	{
		options.time = debounceTime - CDT_DEBOUNCE_500ns;
		options.mode = debounceMode - CDM_TRIGGER_AFTER_STABLE ;
	}
	else
		options.optCode = 16; // disable

	return options.optCode;
}



UlError CtrUsbCtrx::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	return mDaqDevice.daqIDevice()->getStatus(FT_CTR, status, xferStatus);
}

UlError CtrUsbCtrx::waitUntilDone(double timeout)
{
	return mDaqDevice.daqIDevice()->waitUntilDone(FT_CTR, timeout);
}


void CtrUsbCtrx::stopBackground()
{
	mDaqDevice.daqIDevice()->stopBackground(FT_CTR);
}

ScanStatus CtrUsbCtrx::getScanState() const
{
	return mDaqDevice.daqIDevice()->getScanState();
}

void CtrUsbCtrx::addSupportedTickSizes()
{
	mCtrInfo.addTickSize(CTS_TICK_20PT83ns);
	mCtrInfo.addTickSize(CTS_TICK_208PT3ns);
	mCtrInfo.addTickSize(CTS_TICK_2083PT3ns);
	mCtrInfo.addTickSize(CTS_TICK_20833PT3ns);
}

void CtrUsbCtrx::addSupportedDebounceTimes()
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
