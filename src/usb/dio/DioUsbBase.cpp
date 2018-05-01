/*
 * DioUsbBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioUsbBase.h"

namespace ul
{

DioUsbBase::DioUsbBase(const UsbDaqDevice& daqDevice) : DioDevice(daqDevice), mUsbDevice(daqDevice)
{

}

DioUsbBase::~DioUsbBase()
{

}

} /* namespace ul */
