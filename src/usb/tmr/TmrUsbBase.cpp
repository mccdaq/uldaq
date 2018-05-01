/*
 * TmrUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "TmrUsbBase.h"

namespace ul
{

TmrUsbBase::TmrUsbBase(const UsbDaqDevice& daqDevice) : TmrDevice(daqDevice), mUsbDevice(daqDevice)
{

}

TmrUsbBase::~TmrUsbBase()
{

}
}
