/*
 * DioHidBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioHidBase.h"

namespace ul
{

DioHidBase::DioHidBase(const HidDaqDevice& daqDevice) : DioDevice(daqDevice), mHidDevice(daqDevice)
{


}

DioHidBase::~DioHidBase()
{

}

} /* namespace ul */
