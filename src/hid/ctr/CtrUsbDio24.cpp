/*
 * CtrUsbDio24.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "CtrUsbDio24.h"
#include "../UsbDio24.h"

namespace ul
{
CtrUsbDio24::CtrUsbDio24(const HidDaqDevice& daqDevice, int numCtrs) : CtrHidBase(daqDevice)
{
	mCtrInfo.hasPacer(false);
	mCtrInfo.setResolution(32);

	for(int ctr = 0; ctr < numCtrs; ctr++)
		mCtrInfo.addCtr(CMT_COUNT);

	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD);
}

CtrUsbDio24::~CtrUsbDio24()
{

}

unsigned long long CtrUsbDio24::cIn(int ctrNum)
{
	unsigned int count = 0;

	check_CIn_Args(ctrNum);

	unsigned char outBuffer[UsbDio24::MAX_PACKET_SIZE] = {0};
	unsigned char inBuffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	outBuffer[1] = CMD_CIN;

	size_t outBufLen = sizeof(outBuffer);
	size_t inBufLen = sizeof(inBuffer);

	daqDev().queryRawCmd(outBuffer, outBufLen, inBuffer, &inBufLen);

	count = Endian::Instance().le_ptr_to_cpu_ui32(inBuffer);

	return count;
}

void CtrUsbDio24::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	if(loadValue != 0)
		throw UlException(ERR_BAD_CTR_VAL);

	unsigned char buffer[UsbDio24::MAX_PACKET_SIZE] = {0};

	buffer[1] = CMD_CINIT;

	size_t length = sizeof(buffer);

	daqDev().sendRawCmd(buffer, &length);

}

void CtrUsbDio24::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrUsbDio24::cRead(int ctrNum, CounterRegisterType regType)
{
	check_CRead_Args(ctrNum, regType);

	return cIn(ctrNum);
}

} /* namespace ul */
