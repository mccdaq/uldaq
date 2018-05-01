/*
 * UlDevMemInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDEVMEMINFO_H_
#define INTERFACES_ULDEVMEMINFO_H_

#include "../uldaq.h"
#include "UlMemRegionInfo.h"

namespace ul
{

class UlDevMemInfo
{
public:
	virtual ~UlDevMemInfo() {};

	virtual MemRegion getMemRegionTypes() const = 0;
	virtual UlMemRegionInfo& getMemRegionInfo(MemRegion memRegionType) const = 0;
};

}  /* namespace ul */



#endif /* INTERFACES_ULDEVMEMINFO_H_ */
