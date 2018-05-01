/*
 * UlMemRegionInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULMEMREGIONINFO_H_
#define INTERFACES_ULMEMREGIONINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlMemRegionInfo
{
public:
	virtual ~UlMemRegionInfo() {};

	virtual MemRegion getRegionType () const = 0;
	virtual unsigned long long getAddress() const= 0;
	virtual unsigned long long getSize() const = 0;
	virtual MemAccessType getAccessTypes() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULMEMREGIONINFO_H_ */
