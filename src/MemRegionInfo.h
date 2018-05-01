/*
 * MemRegionInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef MEMREGIONINFO_H_
#define MEMREGIONINFO_H_


#include "interfaces/UlMemRegionInfo.h"
#include "ul_internal.h"

namespace ul
{

class UL_LOCAL MemRegionInfo: public UlMemRegionInfo
{
public:
	MemRegionInfo(MemRegion regionType, unsigned long long address, unsigned long long size, MemAccessType accessTypes)
	{
		mRegionType = regionType;
		mAddress = address;
		mSize = size;
		mAccessTypes = accessTypes;
	};

	virtual MemRegion getRegionType () const { return mRegionType; }
	virtual unsigned long long getAddress() const { return mAddress; }
	virtual unsigned long long getSize() const { return mSize; }
	virtual MemAccessType getAccessTypes() const { return mAccessTypes;}


private:
	MemRegion mRegionType;
	unsigned long long mAddress;
	unsigned long long mSize;
	MemAccessType mAccessTypes;
};

} /* namespace ul */



#endif /* MEMREGIONINFO_H_ */
