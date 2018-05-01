/*
 * CtrUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "CtrUsbBase.h"

namespace ul
{

CtrUsbBase::CtrUsbBase(const UsbDaqDevice& daqDevice): CtrDevice(daqDevice), mUsbDevice(daqDevice)
{


}

CtrUsbBase::~CtrUsbBase()
{

}

} /* namespace ul */
