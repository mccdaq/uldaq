/*
 * DevMemInfo.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DevMemInfo.h"

#include "UlException.h"

namespace ul
{

DevMemInfo::DevMemInfo()
{
}

DevMemInfo::~DevMemInfo()
{
}

void DevMemInfo::addMemRegion(MemRegion memRegionType, unsigned long long address, unsigned long long size, MemAccessType accessTypes)
{
	mMemRegionMap.insert(std::pair<MemRegion, MemRegionInfo>(memRegionType, MemRegionInfo(memRegionType, address, size, accessTypes)));
}

MemRegion DevMemInfo::getMemRegionTypes() const
{
	MemRegion regionTypes = (MemRegion) 0;

	for(std::map<MemRegion, MemRegionInfo>::iterator itr = mMemRegionMap.begin(); itr != mMemRegionMap.end(); itr++)
	{
		regionTypes = (MemRegion) (regionTypes | itr->first);
	}

	return regionTypes;
}
UlMemRegionInfo& DevMemInfo::getMemRegionInfo(MemRegion memRegionType) const
{
	MemRegionInfo* info = NULL;

	std::map<MemRegion, MemRegionInfo>::iterator itr = mMemRegionMap.find(memRegionType);

	if(itr != mMemRegionMap.end())
		info = &itr->second;
	else
		throw UlException(ERR_BAD_MEM_REGION);

	return *info;
}

} /* namespace ul */
