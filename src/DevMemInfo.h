/*
 * DevMemInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DEVMEMINFO_H_
#define DEVMEMINFO_H_

#include <map>

#include "interfaces/UlDevMemInfo.h"
#include "ul_internal.h"
#include "MemRegionInfo.h"

namespace ul
{

class UL_LOCAL DevMemInfo: public UlDevMemInfo
{
public:
	DevMemInfo();
	virtual ~DevMemInfo();

	void addMemRegion(MemRegion memRegionType, unsigned long long address, unsigned long long size, MemAccessType accessTypes);

	virtual MemRegion getMemRegionTypes() const;
	virtual UlMemRegionInfo& getMemRegionInfo(MemRegion memRegionType) const;

private:
	mutable std::map<MemRegion, MemRegionInfo> mMemRegionMap;
};

} /* namespace ul */

#endif /* DEVMEMINFO_H_ */
