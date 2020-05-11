/*
 * AiNetBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AI_AINETBASE_H_
#define NET_AI_AINETBASE_H_

#include "../NetDaqDevice.h"
#include "../../AiDevice.h"
#include "../NetScanTransferIn.h"

namespace ul
{

class UL_LOCAL AiNetBase: public AiDevice
{
public:
	AiNetBase(const NetDaqDevice& daqDevice);
	virtual ~AiNetBase();

	const NetDaqDevice& daqDev() const {return mNetDevice;}

	virtual UlError checkScanState(bool* scanDone = NULL) const;

protected:
	virtual void loadAdcCoefficients();
	virtual unsigned int processScanData(void* transfer, unsigned int stageSize);
	virtual void readCalDate();

private:
	enum {STATUS_DATA_OVERRUN = 4};

	void virtual processScanData16(unsigned char* xferBuf, unsigned int xferLength);
	void virtual processScanData32(unsigned char* xferBuf, unsigned int xferLength) {};
	void virtual processScanData64(unsigned char* xferBuf, unsigned int xferLength) {};

private:

	const NetDaqDevice&  mNetDevice;
};

} /* namespace ul */

#endif /* NET_AI_AINETBASE_H_ */
