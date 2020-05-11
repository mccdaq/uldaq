/*
 * ETc32.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_ETC32_H_
#define NET_ETC32_H_

#include "NetDaqDevice.h"

namespace ul
{

class UL_LOCAL ETc32: public NetDaqDevice
{
public:
	ETc32(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~ETc32();

protected:
	virtual unsigned char getMemCmd(MemRegion memRegionType, bool writeAccess) const;

private:
	virtual void initilizeHardware() const;

	void readMeasurementFwVersions() const;

private:
	enum { CMD_VERSION = 0x53 };
	enum { CMD_USER_MEM_R = 0x30, CMD_USER_MEM_W = 0x31, CMD_SETTINGS_MEM_R = 0x32, CMD_SETTINGS_MEM_W = 0x33, CMD_CAL_MEM_R = 0x34};
};
} /* namespace ul */

#endif /* NET_ETC32_H_ */
