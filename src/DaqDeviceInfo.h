/*
 * DaqDeviceInfo.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef DAQDEVICEINFO_H_
#define DAQDEVICEINFO_H_

#include "ul_internal.h"
#include "DevMemInfo.h"
#include "interfaces/UlDaqDeviceInfo.h"

namespace ul
{

class UL_LOCAL DaqDeviceInfo: public UlDaqDeviceInfo
{
public:
	virtual ~DaqDeviceInfo();
	DaqDeviceInfo();

	void setProductId(unsigned int productId) { mProductId = productId; }
	unsigned int getProductId() const { return mProductId; }
	void hasAiDevice(bool hasAiDevice) { mHasAiDevice = hasAiDevice; }
	bool hasAiDevice() const { return mHasAiDevice; }
	void hasAoDevice(bool hasAoDevice) { mHasAoDevice = hasAoDevice; }
	bool hasAoDevice() const { return mHasAoDevice; }
	void hasDioDevice(bool hasDioDevice) { mHasDioDevice = hasDioDevice; }
	bool hasDioDevice() const { return mHasDioDevice; }
	void hasCtrDevice(bool hasCioDevice) { mHasCioDevice = hasCioDevice; }
	bool hasCtrDevice() const { return mHasCioDevice; }
	void hasTmrDevice(bool hasTmrDevice) { mHasTmrDevice = hasTmrDevice; }
	bool hasTmrDevice() const { return mHasTmrDevice; }
	void hasDaqIDevice(bool hasDaqIDevice) { mHasDaqIDevice = hasDaqIDevice; }
	bool hasDaqIDevice() const { return mHasDaqIDevice; }
	void hasDaqODevice(bool hasDaqODevice) { mHasDaqODevice = hasDaqODevice; }
	bool hasDaqODevice() const { return mHasDaqODevice; }

	void setClockFreq(double freq) { mClockFreq = freq; };
	double getClockFreq() const { return mClockFreq;}

	void setEventTypes(long long eventTypes) { mEventTypes = (DaqEventType) eventTypes;}
	DaqEventType getEventTypes() const { return mEventTypes;}

	DevMemInfo* memInfo() const { return mMemInfo;};
	UlDevMemInfo& getMemInfo() const { return *mMemInfo;};

private:
	unsigned int mProductId;
	bool mHasAiDevice;
	bool mHasAoDevice;
	bool mHasDioDevice;
	bool mHasCioDevice;
	bool mHasTmrDevice;
	bool mHasDaqIDevice;
	bool mHasDaqODevice;

	double mClockFreq;
	DaqEventType mEventTypes;
	DevMemInfo* mMemInfo;

};

} /* namespace ul */

#endif /* DAQDEVICEINFO_H_ */
