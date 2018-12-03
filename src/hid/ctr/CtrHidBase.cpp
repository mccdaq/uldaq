/*
 * CtrHidBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrHidBase.h"

namespace ul
{

CtrHidBase::CtrHidBase(const HidDaqDevice& daqDevice): CtrDevice(daqDevice), mHidDevice(daqDevice)
{


}

CtrHidBase::~CtrHidBase()
{

}

} /* namespace ul */
