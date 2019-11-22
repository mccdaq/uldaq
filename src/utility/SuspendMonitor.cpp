/*
 * SuspendMonitor.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include <unistd.h>
#include <stdint.h>
#include <sys/resource.h>

#include "SuspendMonitor.h"
#include "FnLog.h"

namespace ul
{

SuspendMonitor::SuspendMonitor()
{
	mSuspendDetectionThread = 0;
	mTerminateSuspendDetectionThread = false;
	mSystemTimeRecorded = 0;
	mSystemSuspendCount = 0;
}

SuspendMonitor::~SuspendMonitor()
{
	FnLog log("SuspendMonitor::~SuspendMonitor()");

	terminate();
}

void SuspendMonitor::start()
{
	FnLog log("SuspendMonitor::startSuspendDetectionThread");

	mEvent.reset();

	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(!status)
	{
		status = pthread_create(&mSuspendDetectionThread, &attr, &suspendDetectionThread, this);

#ifndef __APPLE__
		pthread_setname_np(mSuspendDetectionThread, "suspend_td");
#endif

		if(status)
			UL_LOG("#### Unable to start suspend detection thread");

		status = pthread_attr_destroy(&attr);
	}
	else
		UL_LOG("#### Unable to initialize attributes for the suspend detection thread");
}

void* SuspendMonitor::suspendDetectionThread(void *arg)
{
	UL_LOG("Suspend detection handler started");

	SuspendMonitor* This = (SuspendMonitor*) arg;

	int niceVal = 10;  // make sure this thread does not get a high priority if the parent thread is running with high priority
	setpriority(PRIO_PROCESS, 0, niceVal);

	unsigned long long currentTime;
	const unsigned int MAX_TIME = 1000; //ms

	struct timespec now;

	ul_clock_realtime(&now);

	This->mSystemTimeRecorded = ((uint64_t) now.tv_sec) * 1000 + ((uint64_t) now.tv_nsec / 1000000);

	while (!This->mTerminateSuspendDetectionThread && This->mEvent.wait_for_signal(100000) == ETIMEDOUT)
	{
		ul_clock_realtime(&now);

		currentTime = ((uint64_t) now.tv_sec) * 1000 + ((uint64_t) now.tv_nsec / 1000000);

		if((currentTime > This->mSystemTimeRecorded) && (currentTime - This->mSystemTimeRecorded > MAX_TIME))
		{
			//UL_LOG("System suspended " << currentTime - mSystemTimeRecorded);

			This->mSystemSuspendCount++;
		}

		This->mSystemTimeRecorded = currentTime;

		usleep(100000);
	}

	UL_LOG("Suspend Detection Thread terminated");

	return NULL;
}

void SuspendMonitor::terminate()
{
	FnLog log("terminateSuspendDetectionThread");

	mTerminateSuspendDetectionThread = true;
	mEvent.signal();

	if(mSuspendDetectionThread)
		pthread_join(mSuspendDetectionThread, NULL);

	mSuspendDetectionThread = 0;
}

} /* namespace ul */
