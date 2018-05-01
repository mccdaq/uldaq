/*
 * UlTmrInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULTMRINFO_H_
#define INTERFACES_ULTMRINFO_H_

#include "../uldaq.h"

namespace ul
{

class UlTmrInfo
{
public:
	virtual ~UlTmrInfo() {};

	virtual int getNumTimers() const = 0;
	virtual TimerType getTimerType(int tmrNum) const = 0;
	virtual double getMinFrequency() const = 0;
	virtual double getMaxFrequency() const = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULTMRINFO_H_ */
