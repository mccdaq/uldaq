/*
 * CtrNet.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrNet.h"

namespace ul
{
CtrNet::CtrNet(const NetDaqDevice& daqDevice, int numCtrs): CtrDevice(daqDevice), mNetDevice(daqDevice)
{
	mCtrInfo.hasPacer(false);
	mCtrInfo.setResolution(32);

	for(int ctr = 0; ctr < numCtrs; ctr++)
		mCtrInfo.addCtr(CMT_COUNT);

	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD);
}

CtrNet::~CtrNet()
{

}

unsigned long long CtrNet::cIn(int ctrNum)
{
	unsigned long long count = 0;

	check_CIn_Args(ctrNum);

	unsigned int ctrValue = 0;

	daqDev().queryCmd(CMD_CTR_R, NULL, 0, (unsigned char*) &ctrValue, sizeof(unsigned int));

	count = Endian::le_ui32_to_cpu(ctrValue);

	return count;
}

void CtrNet::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	if(loadValue != 0)
		throw UlException(ERR_BAD_CTR_VAL);

	daqDev().queryCmd(CMD_CTR_W);

}

void CtrNet::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrNet::cRead(int ctrNum, CounterRegisterType regType)
{
	check_CRead_Args(ctrNum, regType);

	return cIn(ctrNum);
}
} /* namespace ul */
