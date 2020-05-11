/*
 * AiVirNetBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AiVirNetBase.h"
#include "../VirNetDaqDevice.h"

namespace ul
{

AiVirNetBase::AiVirNetBase(const NetDaqDevice& daqDevice) : AiNetBase(daqDevice)
{
	// TODO Auto-generated constructor stub

}

AiVirNetBase::~AiVirNetBase()
{
	// TODO Auto-generated destructor stub
}

double AiVirNetBase::aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags)
{
	UlLock lock(mIoDeviceMutex);

	check_AIn_Args(channel, inputMode, range, flags);

	double data = 0.0;

	unsigned char remote_ul_err = 0;
	TAInParams params;
	params.channel = channel;
	params.inputMode = inputMode;
	params.range = range;
	params.flags = flags;

	daqDev().queryCmdVir(VNC_AIN, (unsigned char*)&params, sizeof(params), (unsigned char*) &data, sizeof(data), &remote_ul_err);

	if(remote_ul_err)
		throw UlException((UlError) remote_ul_err);

	return data;
}

double AiVirNetBase::aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[])
{
	UlLock lock(mIoDeviceMutex);

	check_AInScan_Args(lowChan, highChan, inputMode, range, samplesPerChan, rate, options, flags, data);

	stopBackground(); // call to close data socket from server side if it is still open

	int chanCount = queueEnabled() ? queueLength() :  highChan - lowChan + 1;

	std::vector<CalCoef> calCoefs;// = getScanCalCoefs(lowChan, highChan, inputMode, range, flags);
	std::vector<CustomScale> customScales;// = getCustomScales(lowChan, highChan);

	setScanInfo(FT_AI, chanCount, samplesPerChan, VIR_NET_SAMPLE_SIZE/*mAiInfo.getSampleSize()*/, mAiInfo.getResolution(), options, flags, calCoefs, customScales, data);

	int timeout = 1000 +  (1000 / (chanCount * rate));
	if(timeout < 1000)
		timeout = 1000;

	daqDev().scanTranserIn()->initilizeTransfer(this, VIR_NET_SAMPLE_SIZE/*mAiInfo.getSampleSize()*/, timeout);

	//usleep(500);

	unsigned char remote_ul_err = 0;
	double actualRate = 0;

	TAInScanParams params;
	params.lowChan = lowChan;
	params.highChan = highChan;
	params.inputMode = inputMode;
	params.range = range;
	params.samplesPerChan = samplesPerChan;
	params.rate = rate;
	params.options = options;
	params.flags = flags;

	daqDev().queryCmdVir(VNC_AINSCAN, (unsigned char*)&params, sizeof(params), (unsigned char*) &actualRate, sizeof(actualRate), &remote_ul_err);

	if(!remote_ul_err)
	{
		// TODO: investigate flushCmdSocket() below and add if necessary

		// don't remove. Added to eliminates 200 ms of ack. This forces the ACK to be sent immediately for the start command since scan won't start
		// until host ACKs the start response from the host
		//daqDev().flushCmdSocket();

		daqDev().scanTranserIn()->start();

		setScanState(SS_RUNNING);
	}
	else
	{
		stopBackground();
		throw UlException((UlError) remote_ul_err);
	}


	return actualRate;
}

void AiVirNetBase::aInLoadQueue(AiQueueElement queue[], unsigned int numElements)
{
	check_AInLoadQueue_Args(queue, numElements);

	unsigned char remote_ul_err = 0;

	TALoadQueueParams params;
	params.numElements = 0;

	if(queue != NULL && numElements > 0) // disable loadqueue
	{
		params.numElements = numElements;

		for(unsigned int i = 0; i < numElements; i++)
		{
			params.elements[i].channel = queue[i].channel;
			params.elements[i].inputMode = queue[i].inputMode;
			params.elements[i].range = queue[i].range;
		}
	}

	int paramsSize = sizeof(params.numElements) + sizeof(TALoadQueueElement) * params.numElements;

	daqDev().queryCmdVir(VNC_AIN_LOAD_QUEUE, (unsigned char*)&params, paramsSize, NULL, 0, &remote_ul_err);

	if(remote_ul_err)
		throw UlException((UlError) remote_ul_err);
}

UlError AiVirNetBase::getStatus(ScanStatus* status, TransferStatus* xferStatus)
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

void AiVirNetBase::stopBackground()
{
	//UlError err = terminateScan();

	UlLock lock(mIoDeviceMutex);

	unsigned char remote_ul_err = 0;

	//double actualRate = 0;

	daqDev().queryCmdVir(VNC_AINSCAN_STOP, NULL, 0, NULL, 0, &remote_ul_err);

	daqDev().scanTranserIn()->terminate();

	setScanState(SS_IDLE);

	if(remote_ul_err)
		throw UlException((UlError) remote_ul_err);
}

UlError AiVirNetBase::checkScanState(bool* scanDone) const
{
	UlError err = ERR_NO_ERROR;
	//unsigned char remote_ul_err = 0;

	try
	{
		//daqDev().queryCmdVir(ULF_AINSCAN_STATE, NULL, 0, NULL, 0, &remote_ul_err);

		//err = (UlError) remote_ul_err;

		TXferInState state = ((const VirNetDaqDevice&)daqDev()).getXferInState();

		err = (UlError) state.error;
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

void AiVirNetBase::processScanData64(unsigned char* xferBuf, unsigned int xferLength)
{
	UlLock lock(mProcessScanDataMutex);  // added the lock since mScanInfo.totalSampleTransferred is not updated atomically and is accessed from different thread when user invokes the getStatus function

	int numOfSampleCopied = 0;
	//int sampeSize = sizeof(double); // sample size for virtual net devies is always size of double since data is coming from the UL buffer
	int requestSampleCount = xferLength / mScanInfo.sampleSize;  // last packet in the finite mode might be less
	double* buffer = (double*)xferBuf;

	//double data;
	//long rawVal;

	double* dataBuffer = (double*) mScanInfo.dataBuffer; //Note: ctr and dio subsystems change to unsigned long long from double

	while(numOfSampleCopied < requestSampleCount)
	{
		//rawVal = Endian::le_ui16_to_cpu(buffer[numOfSampleCopied]);

		/*if((mScanInfo.flags & NOCALIBRATEDATA) && (mScanInfo.flags & NOSCALEDATA))
			data = rawVal;
		else
		{
			data = (mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].slope * rawVal) + mScanInfo.calCoefs[mScanInfo.currentCalCoefIdx].offset;

		}

		dataBuffer[mScanInfo.currentDataBufferIdx] = mScanInfo.customScales[mScanInfo.currentCalCoefIdx].slope * data + mScanInfo.customScales[mScanInfo.currentCalCoefIdx].offset;*/

		dataBuffer[mScanInfo.currentDataBufferIdx] = buffer[numOfSampleCopied];

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



} /* namespace ul */
