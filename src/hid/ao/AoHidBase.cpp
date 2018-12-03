/*
 * AoHidBase.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoHidBase.h"

namespace ul
{

AoHidBase::AoHidBase(const HidDaqDevice& daqDevice): AoDevice(daqDevice), mHidDevice(daqDevice)
{


}

AoHidBase::~AoHidBase()
{

}

} /* namespace ul */
