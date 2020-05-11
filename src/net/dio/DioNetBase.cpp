/*
 * DioNetBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DioNetBase.h"

namespace ul
{

DioNetBase::DioNetBase(const NetDaqDevice& daqDevice) : DioDevice(daqDevice), mNetDevice(daqDevice)
{

}

DioNetBase::~DioNetBase()
{

}

} /* namespace ul */
