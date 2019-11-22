/*
 * AiChanInfo.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "AiChanInfo.h"

namespace ul
{
AiChanInfo::AiChanInfo(int chan)
{
	mChanNum = chan;
	mTypes = (AiChanType) 0;
}

AiChanInfo::~AiChanInfo()
{
}

void AiChanInfo::addChanMode(AiInputMode mode)
{
	mMode.push_back(mode);
}

void AiChanInfo::setChanTypes(long long type)
{
	mTypes = (AiChanType) type;
}

AiChanType AiChanInfo::getChanTypes() const
{
	return mTypes;
}

int AiChanInfo::getChanNum() const
{
	return mChanNum;
}

std::vector<AiInputMode> AiChanInfo::getChanModes() const
{
	return mMode;
}

/*
public TcInfo getTcInfo()
{
	return mTcInfo;
}*/

} /* namespace ul */
