/*
 * AiVirNetBase.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AI_AIVIRNETBASE_H_
#define NET_AI_AIVIRNETBASE_H_

#include "AiNetBase.h"
#include "../../virnet.h"

namespace ul
{

class UL_LOCAL AiVirNetBase: public AiNetBase
{
public:
	AiVirNetBase(const NetDaqDevice& daqDevice);
	virtual ~AiVirNetBase();

	virtual double aIn(int channel, AiInputMode inputMode, Range range, AInFlag flags);
	double aInScan(int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double rate, ScanOption options, AInScanFlag flags, double data[]);
	virtual void aInLoadQueue(AiQueueElement queue[], unsigned int numElements);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	virtual UlError checkScanState(bool* scanDone = NULL) const;

protected:
	virtual void loadAdcCoefficients() {};

private:
	void virtual processScanData64(unsigned char* xferBuf, unsigned int xferLength);
};

} /* namespace ul */

#endif /* NET_AI_AIVIRNETBASE_H_ */
