/*
 * DaqEventHandler.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQEVENTHANDLER_H_
#define DAQEVENTHANDLER_H_

#include <bitset>

#include "ul_internal.h"
#include "DaqDevice.h"
#include "DaqEvent.h"
#include "./utility/ThreadEvent.h"
#include "UlException.h"

namespace ul
{

class UL_LOCAL DaqEventHandler
{
public:
	DaqEventHandler(const DaqDevice& daqDevice);
	virtual ~DaqEventHandler();

	DaqEventType getEnabledEventTypes() const { return mEnabledEventsTypes; }

	void enableEvent(DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCallbackFunc, void* userData);
	void disableEvent(DaqEventType eventTypes);

	void resetInputEvents(DaqEventType eventTypes);
	void resetOutputEvents(DaqEventType eventTypes);
	unsigned long long getEventParameter(DaqEventType eventType);

	void setCurrentEventAndData(DaqEventType eventType, unsigned long long eventData);

	void start();
	void stop();

private:
	void startEventThread();
	void terminateEventThread();
	static void* eventThread(void* arg);
	void waitForEvent();

	unsigned int getEventIndex(DaqEventType eventType);

	void addEnabledEvents(DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCalbackFunc, void* userData);
	void getCurrentEventsAndData(DaqEvent* daqEvents, int& eventCount);

	void check_EnableEvent_Args(DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCalbackFunc);
	void check_DisableEvent_Args(DaqEventType eventTypes);

private:
	enum {MAX_EVENT_TYPE_COUNT = 5};

	const DaqDevice& mDaqDevice;
	DaqEventType  mEnabledEventsTypes;
	DaqEvent mDaqEvents[MAX_EVENT_TYPE_COUNT];

	pthread_mutex_t mEventHandlerMutex;
	pthread_mutex_t mEventMutex;

	pthread_t mEventThreadHandle;
	bool mTerminateEventThread;
	ThreadEvent mEventThreadInitEvent;
	ThreadEvent mNotifier;
	bool mDaqEventOccured;
	int mLastEventIndex;
};

} /* namespace ul */

#endif /* DAQEVENTHANDLER_H_ */
