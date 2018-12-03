/*
 * CtrHid.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrHid.h"

namespace ul
{

CtrHid::CtrHid(const HidDaqDevice& daqDevice, int numCtrs) : CtrHidBase(daqDevice)
{
	mCtrInfo.hasPacer(false);
	mCtrInfo.setResolution(32);

	for(int ctr = 0; ctr < numCtrs; ctr++)
		mCtrInfo.addCtr(CMT_COUNT);

	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD);
}

CtrHid::~CtrHid()
{

}

unsigned long long CtrHid::cIn(int ctrNum)
{
	unsigned int count = 0;

	check_CIn_Args(ctrNum);

	daqDev().queryCmd(CMD_CIN, &count);

	return count;
}

void CtrHid::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	if(loadValue != 0)
		throw UlException(ERR_BAD_CTR_VAL);

	daqDev().sendCmd(CMD_CINIT);

}

void CtrHid::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrHid::cRead(int ctrNum, CounterRegisterType regType)
{
	check_CRead_Args(ctrNum, regType);

	return cIn(ctrNum);
}



} /* namespace ul */
