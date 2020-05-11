/*
 * AoE1608.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AO_AOE1608_H_
#define NET_AO_AOE1608_H_

#include "AoNetBase.h"

#include "AoNetBase.h"

namespace ul
{

class UL_LOCAL AoE1608: public AoNetBase
{
public:
	AoE1608(const NetDaqDevice& daqDevice);
	virtual ~AoE1608();

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);

protected:
	virtual int getCalCoefIndex(int channel, Range range) const { return channel;}

private:
	enum { CMD_AOUT = 0x21};
};

} /* namespace ul */

#endif /* NET_AO_AOE1608_H_ */
