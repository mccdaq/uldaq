/*
 * CtrUsb1208hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrUsb1208hs.h"

namespace ul
{

CtrUsb1208hs::CtrUsb1208hs(const UsbDaqDevice& daqDevice, int numCtrs) : CtrUsbBase(daqDevice)
{
	mCtrInfo.hasPacer(false);
	mCtrInfo.setResolution(32);

	for(int ctr = 0; ctr < numCtrs; ctr++)
		mCtrInfo.addCtr(CMT_COUNT);

	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD);
}

CtrUsb1208hs::~CtrUsb1208hs()
{

}

unsigned long long CtrUsb1208hs::cIn(int ctrNum)
{
	unsigned long long count = 0;

	check_CIn_Args(ctrNum);

	int numCtrs = mCtrInfo.getNumCtrs();

	unsigned int counterVals[4] = {0, 0, 0, 0};

	daqDev().queryCmd(CMD_CTR, 0, 0, (unsigned char*) &counterVals, numCtrs * sizeof(unsigned int));

	count = Endian::le_ui32_to_cpu(counterVals[ctrNum]);

	return count;
}

void CtrUsb1208hs::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	if(loadValue != 0)
		throw UlException(ERR_BAD_CTR_VAL);

	daqDev().sendCmd(CMD_CTR, ctrNum, 0, NULL, 0);

}

void CtrUsb1208hs::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrUsb1208hs::cRead(int ctrNum, CounterRegisterType regType)
{
	check_CRead_Args(ctrNum, regType);

	return cIn(ctrNum);
}


} /* namespace ul */
