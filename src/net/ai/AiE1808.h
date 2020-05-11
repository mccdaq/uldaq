/*
 * AiE1808.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_AI_AIE1808_H_
#define NET_AI_AIE1808_H_

#include "AiVirNetBase.h"

namespace ul
{

class UL_LOCAL AiE1808: public AiVirNetBase
{
public:
	AiE1808(const NetDaqDevice& daqDevice);
	virtual ~AiE1808();

private:
	int getCalCoefIndex(int channel, AiInputMode chanMode, Range range) const { return 0;}
	void addSupportedRanges();
	void addQueueInfo();

private:
	enum { FIFO_SIZE = 8 * 4 * 1024 }; // samples size is 4
};

} /* namespace ul */

#endif /* NET_AI_AIE1808_H_ */
