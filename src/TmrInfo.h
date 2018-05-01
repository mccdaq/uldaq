/*
 * TmrInfo.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef TMRINFO_H_
#define TMRINFO_H_

#include "ul_internal.h"
#include <vector>
#include "interfaces/UlTmrInfo.h"

namespace ul
{

class UL_LOCAL TmrInfo: public UlTmrInfo
{
public:
	TmrInfo();
	virtual ~TmrInfo();

	virtual void setNumTimers(int numChans);
	virtual int getNumTimers() const;
	virtual void setTimerType(TimerType type);
	virtual TimerType getTimerType(int tmrNum) const;
	virtual void setMinFrequency(double minFreq);
	virtual double getMinFrequency() const;
	virtual void setMaxFrequency(double maxFreq);
	virtual double getMaxFrequency() const;
	void setScanOptions(long long options);
	ScanOption getScanOptions() const;
	bool supportsTrigger() const;

	void setTriggerTypes(long long triggerTypes);
	TriggerType getTriggerTypes() const;

private:
	int mNumTimers;
	double mMinFreq;
	double mMaxFreq;
	TimerType mType;
	ScanOption mScanOptions;
	TriggerType mTriggerTypes;
};

} /* namespace ul */

#endif /* TMRINFO_H_ */
