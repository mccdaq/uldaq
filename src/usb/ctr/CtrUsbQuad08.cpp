/*
 * CtrUsbQuad08.cpp
 *
 * Author: Measurement Computing Corporation
 */

#include "CtrUsbQuad08.h"
#include "../UsbIotech.h"

#define PHASE_B_EDGE_MASK		3 << 2
#define INDEX_EDGE_MASK			3 << 4


namespace ul
{
CtrUsbQuad08::CtrUsbQuad08(const UsbDaqDevice& daqDevice, int numCtrs) : CtrUsbBase(daqDevice)
{
	double minRate = 1.0 / ( 24 * 3600); // 24 hours
	mCtrInfo.hasPacer(true);
	mCtrInfo.setResolution(48);

	mCtrInfo.setScanOptions(SO_DEFAULTIO|SO_CONTINUOUS|SO_EXTTRIGGER|SO_EXTCLOCK|SO_BLOCKIO);
	mCtrInfo.setCInScanFlags(CINSCAN_FF_CTR16_BIT | CINSCAN_FF_CTR32_BIT | CINSCAN_FF_CTR48_BIT);
	mCtrInfo.setTriggerTypes(TRIG_HIGH | TRIG_LOW | TRIG_POS_EDGE | TRIG_NEG_EDGE);

	mCtrInfo.setMinScanRate(minRate);

	mCtrInfo.setMaxScanRate(8000000);
	mCtrInfo.setMaxThroughput(8000000);

	mCtrInfo.setFifoSize(FIFO_SIZE);

	for(int i = 0; i < numCtrs; i++)
		mCtrInfo.addCtr(CMT_COUNT | CMT_PERIOD | CMT_PULSE_WIDTH | CMT_ENCODER);


	mCtrInfo.setCtrMeasurementModes(CMT_COUNT , CMM_CLEAR_ON_READ | CMM_NO_RECYCLE | CMM_RANGE_LIMIT_ON |
									CMM_GATING_ON | CMM_PHB_CONTROLS_DIR | CMM_DECREMENT_ON | CMM_LATCH_ON_INDEX);
	mCtrInfo.setCtrMeasurementModes(CMT_PERIOD , CMM_PERIOD_X1| CMM_PERIOD_X10 | CMM_PERIOD_X100 | CMM_PERIOD_X1000 | CMM_PERIOD_GATING_ON);
	mCtrInfo.setCtrMeasurementModes(CMT_PULSE_WIDTH , CMM_PULSE_WIDTH_GATING_ON);
	mCtrInfo.setCtrMeasurementModes(CMT_ENCODER , CMM_ENCODER_X1 | CMM_ENCODER_X2 | CMM_ENCODER_X4 | CMM_ENCODER_LATCH_ON_Z | CMM_ENCODER_CLEAR_ON_Z | CMM_ENCODER_NO_RECYCLE | CMM_ENCODER_RANGE_LIMIT_ON);


	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD | CRT_MAX_LIMIT);

	addSupportedTickSizes();
	addSupportedDebounceTimes();

	setScanEndpointAddr(0x82);

	memset(mCounterConfig, 0, sizeof(mCounterConfig));

	for(int i = 0; i < numCtrs; i++)
	{
		mCounterConfig[i].rangeLimitEnabled = false;
		mCounterConfig[i].maxLimitVal = 0xFFFFFFFFFFFF;
	}

	mFirstDataPacketReceived = false;
	mDisableHwTrigger = false;

	UlLock::initMutex(mCtrSelectMutex, PTHREAD_MUTEX_RECURSIVE);

}

CtrUsbQuad08::~CtrUsbQuad08()
{
	UlLock::destroyMutex(mCtrSelectMutex);
}

void CtrUsbQuad08::initialize()
{
	initScanCountersState();

	for(int ctr = 0; ctr < mCtrInfo.getNumCtrs(); ctr++)
	{
		mCounterConfig[ctr].asyncMode = true; // must be set before calling cConfigScan
		cConfigScan(ctr, CMT_COUNT, CMM_DEFAULT, CED_RISING_EDGE, CTS_TICK_20PT83ns, CDM_NONE, CDT_DEBOUNCE_0ns, CF_DEFAULT);
	}

	// workaround
	/*{
		unsigned long long data[2];
		cInScan(0, 0, 2, 1000, SO_EXTTRIGGER, CINSCAN_FF_CTR16_BIT, data);
		stopBackground();

		// calll cIn to put ctr 0 back in async mode
		cIn(0);
	}*/
}

unsigned long long CtrUsbQuad08::cIn(int ctrNum)
{
	check_CIn_Args(ctrNum);

	if(getScanState() == SS_RUNNING && isScanCounterActive(ctrNum))
	{
		throw UlException(ERR_ALREADY_ACTIVE);
	}

	UlLock lock(mCtrSelectMutex);

	union
	{
		struct
		{
			unsigned long long bank0 : 16;
			unsigned long long bank1 : 16;
			unsigned long long bank2 : 16;
			unsigned long long rsv   : 16;
		};

		unsigned long long value;
	} counter;

	counter.value = 0;

	if(!mCounterConfig[ctrNum].asyncMode)
	{
		mCounterConfig[ctrNum].asyncMode = true; // must be set before calling cConfigScan

		cConfigScan(ctrNum, mCounterConfig[ctrNum].measureType,  mCounterConfig[ctrNum].measureMode,
					mCounterConfig[ctrNum].edgeDetection, mCounterConfig[ctrNum].tickSize,
					mCounterConfig[ctrNum].debounceMode, mCounterConfig[ctrNum].debounceTime, CF_DEFAULT);
	}

	// select counter
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, ctrNum, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	unsigned short bankVal = 0;
	daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegCtrEnhRead, (unsigned char*) &bankVal, sizeof(bankVal));

	counter.bank0 = bankVal;

	bankVal = 0;
	daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegCtrEnhRead + 1, (unsigned char*) &bankVal, sizeof(bankVal));

	counter.bank1 = bankVal;

	bankVal = 0;
	daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegCtrEnhRead + 2, (unsigned char*) &bankVal, sizeof(bankVal));

	counter.bank2 = bankVal;

	return counter.value;

}

void CtrUsbQuad08::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	if(regType == CRT_MAX_LIMIT)
	{
		mCounterConfig[ctrNum].maxLimitVal = loadValue;

		if(mCounterConfig[ctrNum].rangeLimitEnabled)
		{
			setModuloReg(ctrNum, mCounterConfig[ctrNum].maxLimitVal);
		}
	}
	else if (regType == CRT_LOAD)
	{
		if(loadValue == 0)
		{
			cClear(ctrNum);
		}
		else
			throw UlException(ERR_BAD_CTR_VAL);
	}
}

unsigned long long CtrUsbQuad08::cRead(int ctrNum, CounterRegisterType regType)
{
	unsigned long long count = 0;

	check_CRead_Args(ctrNum, regType);

	if(regType == CRT_MAX_LIMIT)
	{
		count = mCounterConfig[ctrNum].maxLimitVal;
	}
	else if(regType == CRT_COUNT)
	{
		count = cIn(ctrNum);
	}

	return count;
}

void CtrUsbQuad08::cClear(int ctrNum)
{
	if(ctrNum < 0 || ctrNum >= mCtrInfo.getNumCtrs())
				throw UlException(ERR_BAD_CTR);

	// configuring the counter resets it
	cConfigScan(ctrNum, mCounterConfig[ctrNum].measureType,  mCounterConfig[ctrNum].measureMode,
			mCounterConfig[ctrNum].edgeDetection, mCounterConfig[ctrNum].tickSize,
			mCounterConfig[ctrNum].debounceMode, mCounterConfig[ctrNum].debounceTime, CF_DEFAULT);
}

double CtrUsbQuad08::cInScan(int lowCtrNum, int highCtrNum, int samplesPerCounter, double rate, ScanOption options, CInScanFlag flags, unsigned long long data[])
{
	check_CInScan_Args(lowCtrNum, highCtrNum, samplesPerCounter, rate, options, flags, data);

	UlLock lock(mCtrSelectMutex);

	//double actualRate = 0;

	mFirstDataPacketReceived = false;
	mDisableHwTrigger = false;

	int epAddr = getScanEndpointAddr();

	int numCtrs = highCtrNum - lowCtrNum + 1;


	int sampleSize = 2;

	if(flags & CINSCAN_FF_CTR32_BIT)
		sampleSize = 4;
	else if(flags & CINSCAN_FF_CTR48_BIT)
		sampleSize = 8;

	int stageSize = calcStageSize(epAddr, rate, numCtrs,  samplesPerCounter, sampleSize);

	for(int ctrNum = lowCtrNum; ctrNum <= highCtrNum; ctrNum++)
	{
		setScanCounterActive(ctrNum);

		if(mCounterConfig[ctrNum].asyncMode)
		{
			mCounterConfig[ctrNum].asyncMode = false; // must be set before calling cConfigScan

			cConfigScan(ctrNum, mCounterConfig[ctrNum].measureType,  mCounterConfig[ctrNum].measureMode,
					mCounterConfig[ctrNum].edgeDetection, mCounterConfig[ctrNum].tickSize,
					mCounterConfig[ctrNum].debounceMode, mCounterConfig[ctrNum].debounceTime, CF_DEFAULT);
		}
	}


	// daqx sends this section twice.
	for(int i = 0; i < 2; i++)
	{
		// reset the scan list fifo
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, AcqResetScanListFifo, UsbIotech::HWRegAcqControl, NULL, 0);

		bool firstCtr = true;
		bool lastCtr = false;

		for(int ctrNum = lowCtrNum; ctrNum <= highCtrNum; ctrNum++)
		{
			if(ctrNum == highCtrNum)
				lastCtr = true;

			setScanListFifoCfg(ctrNum, firstCtr, lastCtr, flags);

			firstCtr = false;
		}

		// reset the setpoint list
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, SetpointFIFOReset, UsbIotech::HWRegAcqControl, NULL, 0);

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, DigEnhDisable, UsbIotech::HWRegAcqControl, NULL, 0);

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegVariableConvRate, NULL, 0);
	}

	// from adcArm()

	// stop the H/W to be on the safe side
	sendStopCmds();

	// setup pacer clock information
	setupPacerClock(rate, options);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, AcqResetResultsFifo | AcqResetConfigPipe, UsbIotech::HWRegAcqControl, NULL, 0);

	unsigned short endpoint = 2;
	daqDev().sendCmd(UsbIotech::VR_EP_CONTROL, EPC_RESET, endpoint, NULL, 0);

	daqDev().clearHalt(epAddr);

	// set scan info and xfers here

	std::vector<CalCoef> calCoefs;
	std::vector<CustomScale> customScales;

	setScanInfo(FT_CTR, numCtrs, samplesPerCounter, sampleSize, 0, options, flags, calCoefs, customScales, data);

	daqDev().scanTranserIn()->initilizeTransfers(this, epAddr, stageSize);

	// enable DMA Channel 1 transfers (via fpga)
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, DmaCh1Enable, UsbIotech::HWRegDmaControl, NULL, 0);

	// start the setpoint list FIFO
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, SetpointFIFOStart, UsbIotech::HWRegAcqControl, NULL, 0);

	// Start the scan list FIFO to load configuration data
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, SeqStartScanList, UsbIotech::HWRegAcqControl, NULL, 0);

	// Wait for the config pipe to be full
	int retry = 0;
	unsigned short status = 0;
	do
	{
		daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegAcqStatus, (unsigned char*) &status, sizeof(status));

		// Check if any hardware errors have occurred
		if(status & AcqHardwareError)
		{
			// Hardware error, stop the H/W to be on the safe side
			sendStopCmds();

			throw UlException(ERR_INTERNAL);
		}

		// Check if the results fifo is in some kind of bogus state
		if(status & (AcqResultsFIFOMore1Sample | AcqResultsFIFOHasValidData | AcqResultsFIFOOverrun))
		{
			// Results fifo in a bogus state, stop the H/W to be on the safe side
			if(status & AcqHardwareError)
			{
				// Hardware error, stop the H/W to be on the safe side
				sendStopCmds();

				throw UlException(ERR_INTERNAL);
			}
		}

		// No errors, ready to continue if config pipe full and acq logic between scans
		if( (status & (AcqConfigPipeFull | AcqLogicScanning)) == AcqConfigPipeFull )
			break;

		retry++;
	}
	while(retry < 100);

	unsigned short adcPacerCfg = AdcPacerInternal;

	if(options & SO_EXTCLOCK)
		adcPacerCfg = AdcPacerExternal;

	if(options & SO_EXTTRIGGER)
	{
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, adcPacerCfg, UsbIotech::HWRegAcqControl, NULL, 0);

		unsigned short trigModeCode = getTrigModeCode();

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, trigModeCode, UsbIotech::HWRegTrigControl, NULL, 0);
	}
	else
	{
		adcPacerCfg |= AdcPacerEnable;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, adcPacerCfg, UsbIotech::HWRegAcqControl, NULL, 0);
	}

	setScanState(SS_RUNNING);

	status = 0;
	daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegAcqStatus, (unsigned char*) &status, sizeof(status));

	return actualScanRate();
}

unsigned short CtrUsbQuad08::getTrigModeCode()
{
	unsigned char code;
	switch (mTrigCfg.type)
	{
		case TRIG_POS_EDGE:
			code = TrigTransLoHi | TrigEdgeSense | TrigEnable;
			break;
		case TRIG_NEG_EDGE:
			code = TrigTransHiLo | TrigEdgeSense | TrigEnable;
			break;
		case TRIG_HIGH:
			code = TrigAbove | TrigLevelSense | TrigEnable;
			break;
		case TRIG_LOW:
			code = TrigBelow | TrigLevelSense | TrigEnable;
			break;
		default:
			throw UlException(ERR_BAD_TRIG_TYPE);
	}

	code|= TrigTTL;

	return code;
}

void CtrUsbQuad08::setupPacerClock(double rate, ScanOption options)
{

	if(!(options & SO_EXTCLOCK))
	{
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, AdcPacerNormalMode, UsbIotech::HWRegAcqControl, NULL, 0);


		unsigned long long devisor = calcPacerDevisor(rate, options);

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)(devisor & 0xffff), UsbIotech::HWRegAcqPacerClockDivLow, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)((devisor >> 16) & 0xffff), UsbIotech::HWRegAcqPacerClockDivMed, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, (unsigned short)((devisor >> 32) & 0xffff), UsbIotech::HWRegAcqPacerClockDivHigh, NULL, 0);
	}
	else
		setActualScanRate(rate);
}

unsigned long long CtrUsbQuad08::calcPacerDevisor(double rate, ScanOption options)
{
	unsigned long long devisor = 0;

	if(rate < mCtrInfo.getMinScanRate())
		rate = mCtrInfo.getMinScanRate();

	double clockFreq = mDaqDevice.getClockFreq();
	double devisorDbl =  clockFreq / rate;

	if (devisorDbl > 0)
		--devisorDbl;

	devisor = devisorDbl;

	double actualrate = clockFreq / (1ULL + devisor);

	setActualScanRate(actualrate);

	return devisor;
}

void CtrUsbQuad08::setScanListFifoCfg(int ctrNum, bool firstCtr, bool lastCtr, CInScanFlag flags)
{
	struct
	{
		unsigned short word1;
		unsigned short word2;
	}scanListCfg;

	if((flags & CINSCAN_FF_CTR16_BIT) || (!flags))
	{
		scanListCfg.word1 = getScanListWord1(ctrNum, !firstCtr, lastCtr);

		scanListCfg.word2 = UsbIotech::HWRegCtrEnhRead * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
	}
	else if(flags & CINSCAN_FF_CTR32_BIT)
	{
		scanListCfg.word1 = getScanListWord1(ctrNum, !firstCtr, false);
		scanListCfg.word2 = UsbIotech::HWRegCtrEnhRead * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);

		scanListCfg.word1 =  getScanListWord1(ctrNum, true, lastCtr);
		scanListCfg.word2 = (UsbIotech::UsbIotech::HWRegCtrEnhRead + 1) * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
	}
	else if(flags & CINSCAN_FF_CTR48_BIT)
	{
		scanListCfg.word1 = getScanListWord1(ctrNum, !firstCtr, false);
		scanListCfg.word2 = UsbIotech::HWRegCtrEnhRead * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);


		scanListCfg.word1 =  getScanListWord1(ctrNum, true, false);
		scanListCfg.word2 = (UsbIotech::UsbIotech::HWRegCtrEnhRead + 1) * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);


		scanListCfg.word1 =  getScanListWord1(ctrNum, true, false);
		scanListCfg.word2 = (UsbIotech::UsbIotech::HWRegCtrEnhRead + 2) * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);



		scanListCfg.word1 =  getScanListWord1(ctrNum, true, lastCtr);
		scanListCfg.word2 = (UsbIotech::UsbIotech::HWRegCtrEnhRead + 3) * 2;

		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word1, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, scanListCfg.word2, UsbIotech::HWRegAcqScanListFIFO, NULL, 0);
	}
}

unsigned short CtrUsbQuad08::getScanListWord1(int ctrNum, bool ssh, bool lastCtr)
{
#pragma pack(1)
	union
	{
		struct
		{
			unsigned short digital		: 1;
			unsigned short internal  	: 1;
			unsigned short lastCtr		: 1;
			unsigned short ssh			: 1; // sample and hold
			unsigned short chsel		: 7;
			unsigned short resv			: 5;
		};
		unsigned short code;
	}word1;
#pragma pack()

	word1.code = 0;

	word1.digital = 1;
	word1.internal = 1;

	if(lastCtr)
		word1.lastCtr = 1;

	if(ssh)
		word1.ssh = 1;

	word1.chsel = ctrNum;

	return word1.code;

}


void CtrUsbQuad08::cConfigScan(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode,
							CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
							CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag)
{
	check_CConfigScan_Args(ctrNum, measureType,  measureMode, edgeDetection, tickSize, debounceMode, debounceTime, flag);

	UlLock lock(mCtrSelectMutex);

	setDebounceSetupReg(ctrNum, debounceMode, debounceTime, edgeDetection);

	setCounterSetupReg(ctrNum, measureType, measureMode, tickSize);

	if((measureType == CMT_COUNT && ((measureMode & CMM_RANGE_LIMIT_ON) /*|| measureMode == CMM_NO_RECYCLE*/)) ||
	   (measureType == CMT_ENCODER && ((measureMode & CMM_ENCODER_RANGE_LIMIT_ON) /*|| measureMode == CMM_ENCODER_NO_RECYCLE*/)))
	{
		mCounterConfig[ctrNum].rangeLimitEnabled = true;
		setModuloReg(ctrNum, mCounterConfig[ctrNum].maxLimitVal);
	}
	else
	{
		mCounterConfig[ctrNum].rangeLimitEnabled = false;
		setModuloReg(ctrNum, 0xFFFFFFFFFFFF);
	}


	mCounterConfig[ctrNum].measureType = measureType;
	mCounterConfig[ctrNum].measureMode = measureMode;
	mCounterConfig[ctrNum].edgeDetection = edgeDetection;
	mCounterConfig[ctrNum].tickSize = tickSize;
	mCounterConfig[ctrNum].debounceMode = debounceMode;
	mCounterConfig[ctrNum].debounceTime = debounceTime;
}

void CtrUsbQuad08::setCounterSetupReg(int ctrNum, CounterMeasurementType measureType,  CounterMeasurementMode measureMode, CounterTickSize tickSize)
{
	UlLock lock(mCtrSelectMutex);

#pragma pack(1)
	union
	{
	  struct
	  {
		 unsigned char mode 		: 3;
		 unsigned char opt			: 7;
		 unsigned char noRecycle	: 1;
		 unsigned char chA			: 1;
		 unsigned char rangeLimit	: 1;
		 unsigned char asyncMode	: 1;
		 unsigned char tick			: 2;
	  };
	  unsigned short code;
	} reg;
#pragma pack()

	reg.code = 0;
	reg.chA = 1; //why?

	if(mCounterConfig[ctrNum].asyncMode)
		reg.asyncMode = 1;

	if((measureType == CMT_COUNT && (measureMode & CMM_NO_RECYCLE)) ||
	   (measureType == CMT_ENCODER && (measureMode & CMM_ENCODER_NO_RECYCLE)))
	{
		if((measureType == CMT_COUNT && (measureMode & CMM_RANGE_LIMIT_ON)) ||
		   (measureType == CMT_ENCODER && (measureMode & CMM_ENCODER_RANGE_LIMIT_ON)))
		{
			reg.rangeLimit = 1;
		}
		else
			reg.noRecycle = 1;
	}


	reg.mode = getModeCode(measureType);
	reg.opt = getOptionCode(measureType, measureMode);
	reg.tick = getTickSizeCode(measureType, tickSize);

	// select counter
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, ctrNum, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	// setup counter
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, reg.code, UsbIotech::HWRegCtrEnhSetup, NULL, 0);
}

void CtrUsbQuad08::setDebounceSetupReg(int ctrNum, CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CounterEdgeDetection edgeDetection)
{
	UlLock lock(mCtrSelectMutex);

#pragma pack(1)
	union
	{
	  struct
	  {
		 unsigned char phase_a;
		 unsigned char phase_b;
	  };
	  struct
	  {
		 unsigned char index;
		 unsigned char not_used;
	  };
	  unsigned short code;
	} reg;
#pragma pack()

	reg.code = 0;

	// note: edgeDetection argument is applied to phase A, B and Index unless user specifies sets phase B bits (2 and 3) or index bits (4 and 5)

	unsigned char phase_b_edgeDetection = (edgeDetection & PHASE_B_EDGE_MASK) >> 2;
	unsigned char index_edgeDetection = (edgeDetection & INDEX_EDGE_MASK) >> 4;

	// phase A and B are set with one command and Index is set separately

	unsigned char debounceOptionCode = getDebounceOptionCode(debounceMode, debounceTime, edgeDetection);

	// phase A and B
	// select counter
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, ctrNum, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	reg.phase_a = debounceOptionCode;

	if(phase_b_edgeDetection) // overwrite the edge detection if user specified edge for phase B otherwise use the same value for phase A and B
		reg.phase_b = getDebounceOptionCode(debounceMode, debounceTime, (CounterEdgeDetection) phase_b_edgeDetection);
	else
		reg.phase_b =  debounceOptionCode;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, reg.code, UsbIotech::HWRegCtrEnhDebounce, NULL, 0);

	//// setup for index

	reg.code = 0;

	// index
	// select counter
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, ctrNum, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	reg.not_used = 0x80;

	if(index_edgeDetection) // overwrite the edge detection is user specified edge for index otherwise use the same value for phace A and B
		reg.index = getDebounceOptionCode(debounceMode, debounceTime, (CounterEdgeDetection) index_edgeDetection);
	else
		reg.index =  debounceOptionCode;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, reg.code, UsbIotech::HWRegCtrEnhDebounce + 1, NULL, 0);
}



void CtrUsbQuad08::setModuloReg(int ctrNum, unsigned long long value)
{
	UlLock lock(mCtrSelectMutex);

	unsigned short moduloReg = 0;
	unsigned short moduloRegVal = 0;

	// select module reg 3
	moduloReg = MODULO_REG_3 + ctrNum;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, moduloReg, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	// set value of reg 3
	moduloRegVal = (value & 0xFFFF00000000) >> 32;
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, moduloRegVal, UsbIotech::HWRegCtrEnhSetup, NULL, 0);

	// select module reg 2
	moduloReg = MODULO_REG_2 + ctrNum;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, moduloReg, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	// set value of reg 2
	moduloRegVal = (value & 0x0000FFFF0000) >> 16;
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, moduloRegVal, UsbIotech::HWRegCtrEnhSetup, NULL, 0);

	// select module reg 1
	moduloReg = MODULO_REG_1 + ctrNum;

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, moduloReg, UsbIotech::HWRegCtrEnhBankSelect, NULL, 0);

	// set value of reg 1
	moduloRegVal = (value & 0x00000000FFFF);
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, moduloRegVal, UsbIotech::HWRegCtrEnhSetup, NULL, 0);
}

unsigned char CtrUsbQuad08::getDebounceOptionCode(CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CounterEdgeDetection edgeDetection) const
{
#pragma pack(1)
	union
	{
	  struct
	  {
		 unsigned char time		    : 4;
		 unsigned char enable	    : 1;
		 unsigned char edge			: 1;
		 unsigned char mode 	    : 1;
		 unsigned char chan_on      : 1;
	  };
	  unsigned char code;
	} options;
#pragma pack()

	options.code = 0;
	options.chan_on = 1;
	options.edge = edgeDetection - CED_RISING_EDGE;


	if(debounceMode != CDM_NONE)
	{
		options.enable = 1;
		options.time = debounceTime - CDT_DEBOUNCE_500ns;
		options.mode = debounceMode - CDM_TRIGGER_AFTER_STABLE;
	}

	return options.code;

}

unsigned char CtrUsbQuad08::getModeCode(CounterMeasurementType measureType) const
{
	unsigned char code = 0;

	switch(measureType)
	{
	case CMT_COUNT:
		code = 1;
		break;
	case CMT_PERIOD:
		code = 2;
		break;
	case CMT_PULSE_WIDTH:
		code = 3;
		break;
	case CMT_TIMING:
		code = 4;
		break;
	case CMT_ENCODER:
		code = 5;
		break;
	default:
		code = 0;
		break;
	}

	return code;
}

unsigned char CtrUsbQuad08::getOptionCode(CounterMeasurementType measureType, CounterMeasurementMode measureMode) const
{
	union
	{
		struct
		{
			unsigned char opt0 : 1;
			unsigned char opt1 : 1;
			unsigned char opt2 : 1;
			unsigned char opt3 : 1;
			unsigned char opt4 : 1;
			unsigned char opt5 : 1;
			unsigned char opt6 : 1;
			unsigned char reserved : 1;
		};

		unsigned char code;
	}option;

	option.code = 0;

	if(measureType == CMT_COUNT)
	{
		if(measureMode & CMM_CLEAR_ON_READ)
			option.opt0 = 1;

		if(measureMode & CMM_NO_RECYCLE) // it seems daqx does not set this bit. remove if not necessary
			option.opt1 = 1;

		if(measureMode & CMM_PHB_CONTROLS_DIR)
			option.opt2 = 1;

		if(measureMode & CMM_LATCH_ON_INDEX)
			option.opt3 = 1;

		if(measureMode & CMM_GATING_ON)
			option.opt4 = 1;

		if(measureMode & CMM_DECREMENT_ON)
			option.opt5 = 1;
	}
	else if(measureType == CMT_PERIOD)
	{
		if(measureMode & CMM_PERIOD_X10)
			option.opt0 = 1;
		else if(measureMode & CMM_PERIOD_X100)
			option.opt1 = 1;
		else if(measureMode & CMM_PERIOD_X1000)
		{
			option.opt0 = 1;
			option.opt1 = 1;
		}

		if(measureMode & CMM_PERIOD_GATING_ON)
			option.opt4 = 1;
	}
	else if(measureType == CMT_PULSE_WIDTH)
	{
		if(measureMode & CMM_PULSE_WIDTH_GATING_ON)
			option.opt4 = 1;
	}
	else if(measureType == CMT_ENCODER)
	{
		if(measureMode & CMM_ENCODER_X2)
			option.opt0 = 1;
		else if(measureMode & CMM_ENCODER_X4)
			option.opt1 = 1;

		if(measureMode & CMM_ENCODER_LATCH_ON_Z)
			option.opt3 = 1;

		/*if(measureMode & CMM_GATING_ON) //gating for encoder mode is not supported in the UL
			option.opt4 = 1;*/

		if(measureMode & CMM_ENCODER_CLEAR_ON_Z)
			option.opt5 = 1;
	}

	return option.code;
}

unsigned char CtrUsbQuad08::getTickSizeCode(CounterMeasurementType measureType, CounterTickSize tickSize) const
{
	unsigned char code = 0;

	if((measureType == CMT_PERIOD) || (measureType == CMT_PULSE_WIDTH) || (measureType == CMT_TIMING))
	{
		switch(tickSize)
		{
		case CTS_TICK_208PT3ns:
			code = 1;
			break;
		case CTS_TICK_2083PT3ns:
			code = 2;
			break;
		case CTS_TICK_20833PT3ns:
			code = 3;
			break;
		default:
			code = 0;
			break;
		}
	}

	return code;
}

void CtrUsbQuad08::setScanEndpointAddr(int addr)
{
	mScanEndpointAddr = addr;
}

int CtrUsbQuad08::getScanEndpointAddr() const
{
	return mScanEndpointAddr;
}

int CtrUsbQuad08::calcStageSize(int epAddr, double rate, int ctrCount, int sampleCount, int sampleSize) const
{
	int stageSize = 0;
	int minStageSize = daqDev().getBulkEndpointMaxPacketSize(epAddr);

	double aggRate =  ctrCount * rate * sampleSize; // bytes per second
	long long bufferBytesCount = (long long) sampleCount * sampleSize;
	double stageRate = daqDev().scanTranserIn()->getStageRate();
	stageSize = (int)(aggRate * stageRate);

	if(stageSize % minStageSize != 0)
		stageSize = stageSize + minStageSize - (stageSize % minStageSize);

	if(stageSize > bufferBytesCount)
		stageSize = (int)(bufferBytesCount - (bufferBytesCount % minStageSize));

	if (stageSize < minStageSize)
		stageSize = minStageSize;

	if(stageSize > UsbScanTransferIn::MAX_STAGE_SIZE)
		stageSize = UsbScanTransferIn::MAX_STAGE_SIZE;


	return stageSize;
}

UlError CtrUsbQuad08::getStatus(ScanStatus* status, TransferStatus* xferStatus)
{
	UlError err = ERR_NO_ERROR;

	if(status && xferStatus)
	{
		ScanStatus scanStatus = getScanState();

		getXferStatus(xferStatus);

		if(scanStatus != SS_RUNNING)
			err = daqDev().scanTranserIn()->getXferError();

		*status = scanStatus;
	}
	else
		err = ERR_BAD_ARG;

	return err;
}

void CtrUsbQuad08::sendStopCmds()
{
	// from adcDisarmAcquisition()

	// disable hardware triggers
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, TrigAnalog | TrigDisable, UsbIotech::HWRegTrigControl, NULL, 0);
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, TrigTTL | TrigDisable, UsbIotech::HWRegTrigControl, NULL, 0);

	// stop the pacer clock
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, AdcPacerDisable, UsbIotech::HWRegAcqControl, NULL, 0);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, SeqStopScanList, UsbIotech::HWRegAcqControl, NULL, 0);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, AcqResetConfigPipe | AcqResetResultsFifo, UsbIotech::HWRegAcqControl, NULL, 0);

	// Disable channel 1 transfer (via fpga)
	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, DmaCh1Disable, UsbIotech::HWRegDmaControl, NULL, 0);

	daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegAcqControl, NULL, 0);
}

UlError CtrUsbQuad08::terminateScan()
{
	UlError err = ERR_NO_ERROR;

	try
	{
		sendStopCmds();
	}
	catch(UlException& e)
	{
		err = e.getError();
	}
	catch(...)
	{
		err = ERR_UNHANDLED_EXCEPTION;
	}

	daqDev().scanTranserIn()->stopTransfers();

	setScanCountersInactive();

	return err;
}

void CtrUsbQuad08::stopBackground()
{
	UlError err = terminateScan();

	daqDev().scanTranserIn()->waitForXferStateThread();

	setScanState(SS_IDLE);

	if(err)
		throw UlException(err);
}

UlError CtrUsbQuad08::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	unsigned short status =0;

	try
	{
		daqDev().queryCmd(UsbIotech::VR_FPGA_REGIO, 0, UsbIotech::HWRegAcqStatus, (unsigned char*)&status, sizeof(status));

		if(status & AcqResultsFIFOOverrun)
		{
			err = ERR_OVERRUN;
		}
		else if(status & AcqPacerOverrun)
		{
			err = ERR_PACER_OVERRUN;
		}
	}
	catch(UlException& e)
	{
		err = e.getError();
	}
	catch(...)
	{
		err = ERR_UNHANDLED_EXCEPTION;
	}

	return err;
}

void CtrUsbQuad08::addSupportedTickSizes()
{
	mCtrInfo.addTickSize(CTS_TICK_20PT83ns);
	mCtrInfo.addTickSize(CTS_TICK_208PT3ns);
	mCtrInfo.addTickSize(CTS_TICK_2083PT3ns);
	mCtrInfo.addTickSize(CTS_TICK_20833PT3ns);
}

void CtrUsbQuad08::addSupportedDebounceTimes()
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

void CtrUsbQuad08::processScanData(void* transfer)
{
	libusb_transfer* usbTransfer = (libusb_transfer*)transfer;

	switch(mScanInfo.sampleSize)
	{
	case 2:  // 2 bytes
			processScanData16(usbTransfer);
		break;
	case 4:  // 4 bytes
			processScanData32(usbTransfer);
		break;
	case 8:  // 8 bytes
			processScanData64(usbTransfer);
		break;
	default:
		std::cout << "##### undefined sample size";
		break;
	}


	// disable trigger after the first data block received
	if(!mFirstDataPacketReceived)
	{
		// added for cov
		UlLock lock(mCtrSelectMutex);

		mFirstDataPacketReceived = true;
		mDisableHwTrigger = true;
	}
}

void CtrUsbQuad08::processScanData16(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned short* buffer = (unsigned short*)transfer->buffer;

	unsigned int rawVal;
	unsigned long long* dataBuf = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui16_to_cpu(buffer[numOfSampleCopied]);

		dataBuf[mScanInfo.currentDataBufferIdx] = rawVal;

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}
}


void CtrUsbQuad08::processScanData32(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned int* buffer = (unsigned int*)transfer->buffer;

	unsigned int rawVal;
	unsigned long long* dataBuf = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui32_to_cpu(buffer[numOfSampleCopied]);

		dataBuf[mScanInfo.currentDataBufferIdx] = rawVal;

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}
}

void CtrUsbQuad08::processScanData64(libusb_transfer* transfer)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	int requestSampleCount = transfer->actual_length / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	unsigned long long* buffer = (unsigned long long*)transfer->buffer;

	unsigned long long rawVal;
	unsigned long long* dataBuf = (unsigned long long*) mScanInfo.dataBuffer;

	while(numOfSampleCopied < requestSampleCount)
	{
		rawVal = Endian::le_ui64_to_cpu(buffer[numOfSampleCopied]);

		dataBuf[mScanInfo.currentDataBufferIdx] = rawVal;

		mScanInfo.currentDataBufferIdx++;
		mScanInfo.currentCalCoefIdx++;
		numOfSampleCopied++;

		mScanInfo.totalSampleTransferred++;

		if(mScanInfo.currentDataBufferIdx == mScanInfo.dataBufferSize)
		{
			mScanInfo.currentDataBufferIdx = 0;
			if(!mScanInfo.recycle)
			{
				mScanInfo.allSamplesTransferred = true;
				break;
			}
		}

		if(mScanInfo.currentCalCoefIdx == mScanInfo.chanCount)
			mScanInfo.currentCalCoefIdx = 0;
	}
}

void CtrUsbQuad08::updateScanParam(int param)
{
	if(mDisableHwTrigger)
	{
		// disable hardware triggers
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, TrigAnalog | TrigDisable, UsbIotech::HWRegTrigControl, NULL, 0);
		daqDev().sendCmd(UsbIotech::VR_FPGA_REGIO, TrigTTL | TrigDisable, UsbIotech::HWRegTrigControl, NULL, 0);

		mDisableHwTrigger = false;
	}
}

} /* namespace ul */
