/*
 * SuspendMonitor.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef UTILITY_SUSPENDMONITOR_H_
#define UTILITY_SUSPENDMONITOR_H_

#include "../ul_internal.h"
#include "ThreadEvent.h"

namespace ul
{

class UL_LOCAL SuspendMonitor
{
public:

	static SuspendMonitor& instance()
	{
		static SuspendMonitor mInstance;
		return mInstance;
	}

	~SuspendMonitor();

	static void init() { instance().start(); }
	inline unsigned long long getCurrentSystemSuspendCount() { return mSystemSuspendCount;}

protected:
	SuspendMonitor();

private:
	void start();
	void terminate();
	static void* suspendDetectionThread(void *arg);

private:
	pthread_t mSuspendDetectionThread;
	bool mTerminateSuspendDetectionThread;
	unsigned long long mSystemTimeRecorded;
	unsigned long long mSystemSuspendCount;

	ThreadEvent mEvent;

};

} /* namespace ul */

#endif /* UTILITY_SUSPENDMONITOR_H_ */
