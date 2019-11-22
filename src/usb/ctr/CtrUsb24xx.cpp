/*
 * CtrUsb24xx.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "CtrUsb24xx.h"

namespace ul
{

CtrUsb24xx::CtrUsb24xx(const UsbDaqDevice& daqDevice, int numCtrs) : CtrUsbBase(daqDevice)
{
	mCtrInfo.hasPacer(false);
	mCtrInfo.setResolution(32);

	for(int ctr = 0; ctr < numCtrs; ctr++)
		mCtrInfo.addCtr(CMT_COUNT);

	mCtrInfo.setRegisterTypes(CRT_COUNT | CRT_LOAD);
}

CtrUsb24xx::~CtrUsb24xx()
{

}

unsigned long long CtrUsb24xx::cIn(int ctrNum)
{
	unsigned long long count = 0;

	check_CIn_Args(ctrNum);

	int numCtrs = mCtrInfo.getNumCtrs();

	unsigned int counterVals[2] = {0, 0};

	daqDev().queryCmd(CMD_CTR, 0, 0, (unsigned char*) &counterVals, numCtrs * sizeof(unsigned int));

	count = Endian::le_ui32_to_cpu(counterVals[ctrNum]);

	return count;
}

void CtrUsb24xx::cLoad(int ctrNum, CounterRegisterType regType, unsigned long long loadValue)
{
	check_CLoad_Args(ctrNum, regType, loadValue);

	if(loadValue != 0)
		throw UlException(ERR_BAD_CTR_VAL);

	unsigned char counter = ctrNum;

	daqDev().sendCmd(CMD_CTR, 0, 0, &counter, sizeof(counter));

}

void CtrUsb24xx::cClear(int ctrNum)
{
	cLoad(ctrNum, CRT_LOAD, 0);
}

unsigned long long CtrUsb24xx::cRead(int ctrNum, CounterRegisterType regType)
{
	check_CRead_Args(ctrNum, regType);

	return cIn(ctrNum);
}


} /* namespace ul */
