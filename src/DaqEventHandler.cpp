/*
 * DaqEventHandler.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "DaqEventHandler.h"
#include <sys/resource.h>

#include "./utility/UlLock.h"

namespace ul
{
DaqEventHandler::DaqEventHandler(const DaqDevice& daqDevice) : mDaqDevice(daqDevice)
{
	mEnabledEventsTypes = (DaqEventType) 0;
	UlLock::initMutex(mEventHandlerMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mEventMutex, PTHREAD_MUTEX_RECURSIVE);

	mEventThreadHandle = 0;
	mTerminateEventThread = false;
	mDaqEventOccured = false;
	mLastEventIndex = 0;
}

DaqEventHandler::~DaqEventHandler()
{
	if(mDaqDevice.getDevInfo().getEventTypes())
		disableEvent(mDaqDevice.getDevInfo().getEventTypes());

	UlLock::destroyMutex(mEventMutex);
	UlLock::destroyMutex(mEventHandlerMutex);
}

void DaqEventHandler::start()
{
	UlLock lock(mEventHandlerMutex);

	if(!mEventThreadHandle)
		startEventThread();
}

void DaqEventHandler::stop()
{
	UlLock lock(mEventHandlerMutex);

	if(mEventThreadHandle)
		terminateEventThread();
}

void DaqEventHandler::enableEvent(DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCallbackFunc, void* userData)
{
	check_EnableEvent_Args(eventTypes, eventParameter, eventCallbackFunc);

	UlLock lock(mEventHandlerMutex);

	addEnabledEvents(eventTypes, eventParameter, eventCallbackFunc, userData);

	if(!mEventThreadHandle)
		startEventThread();
}

void DaqEventHandler::disableEvent(DaqEventType eventTypes)
{
	check_DisableEvent_Args(eventTypes);

	UlLock lock(mEventHandlerMutex);

	mEnabledEventsTypes = (DaqEventType)(mEnabledEventsTypes & (~eventTypes));

	if(!mEnabledEventsTypes && mEventThreadHandle)
		terminateEventThread();
}

void DaqEventHandler::addEnabledEvents(DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCalbackFunc, void* userData)
{
	std::bitset<MAX_EVENT_TYPE_COUNT> events(eventTypes);
	DaqEventType eventType;
	int eventIndex;

	for(unsigned int i = 0; i < events.size(); i++)
	{
		if(events[i])
		{
			eventType = (DaqEventType) (1 << i);
			eventIndex = getEventIndex(eventType);

			mDaqEvents[eventIndex].type = eventType;
			mDaqEvents[eventIndex].eventOccured = false;
			mDaqEvents[eventIndex].callbackFunction = eventCalbackFunc;
			mDaqEvents[eventIndex].userData = userData;

			if( eventType ==  DE_ON_DATA_AVAILABLE)
				mDaqEvents[eventIndex].eventParameter = eventParameter;
		}
	}

	mEnabledEventsTypes = (DaqEventType)(mEnabledEventsTypes | eventTypes);
}

void DaqEventHandler::resetInputEvents(DaqEventType eventTypes)
{
	DaqEventType inputEventTypes = (DaqEventType) (eventTypes & (DE_ON_DATA_AVAILABLE | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_INPUT_SCAN));
	std::bitset<MAX_EVENT_TYPE_COUNT> events(inputEventTypes);

	DaqEventType eventType;
	int eventIndex;

	for(unsigned int i = 0; i < events.size(); i++)
	{
		if(events[i])
		{
			eventType = (DaqEventType) (1 << i);
			eventIndex = getEventIndex(eventType);

			mDaqEvents[eventIndex].eventOccured = false;
		}
	}
}

void DaqEventHandler::resetOutputEvents(DaqEventType eventTypes)
{
	DaqEventType inputEventTypes = (DaqEventType) (eventTypes & (DE_ON_OUTPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN ));
	std::bitset<MAX_EVENT_TYPE_COUNT> events(inputEventTypes);

	DaqEventType eventType;
	int eventIndex;

	for(unsigned int i = 0; i < events.size(); i++)
	{
		if(events[i])
		{
			eventType = (DaqEventType) (1 << i);
			eventIndex = getEventIndex(eventType);

			mDaqEvents[eventIndex].eventOccured = false;
		}
	}
}

unsigned long long DaqEventHandler::getEventParameter(DaqEventType eventType)
{
	int eventIndex = getEventIndex(eventType);

	return mDaqEvents[eventIndex].eventParameter;
}

void DaqEventHandler::setCurrentEventAndData(DaqEventType eventType, unsigned long long eventData)
{
	UlLock lock(mEventMutex);

	if(mEnabledEventsTypes & eventType) // check if this event type is disabled after scan started
	{
		int eventIndex = getEventIndex(eventType);

		mDaqEvents[eventIndex].eventData = eventData;
		mDaqEvents[eventIndex].eventOccured = true;

		mDaqEventOccured = true;
		mNotifier.signal();
	}
}

void DaqEventHandler::getCurrentEventsAndData(DaqEvent* daqEvents, int& eventCount)
{
	//std::vector<DaqEvent> currentEvents;

	UlLock lock(mEventMutex);

	eventCount = 0;

	for(int eventIndex = 0; eventIndex < MAX_EVENT_TYPE_COUNT; eventIndex++ )
	{
		if(mDaqEvents[eventIndex].eventOccured)
		{
			daqEvents[eventCount] = mDaqEvents[eventIndex];
			mDaqEvents[eventIndex].eventOccured = false;
			eventCount++;
		}
	}

	mDaqEventOccured = false;
}

void DaqEventHandler::startEventThread()
{
	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(!status)
	{
		mTerminateEventThread = false;
		mEventThreadInitEvent.reset();

		status = pthread_create(&mEventThreadHandle, &attr, &eventThread, this);

#ifndef __APPLE__
		pthread_setname_np(mEventThreadHandle, "event_td");
#endif

		if(status)
			UL_LOG("#### Unable to start the daq event handler thread");
		else
		{
			mEventThreadInitEvent.wait_for_signal(100);
		}

		status = pthread_attr_destroy(&attr);
	}
	else
		UL_LOG("#### Unable to initialize attributes for the daq event handler thread");
}

void DaqEventHandler::terminateEventThread()
{
	mTerminateEventThread = true;
	mNotifier.signal();

	if(mEventThreadHandle)
		pthread_join(mEventThreadHandle, NULL);

	mEventThreadHandle = 0;
}

void* DaqEventHandler::eventThread(void *arg)
{
	DaqEventHandler* This = (DaqEventHandler*) arg;

	int niceVal = 0;  // make sure this thread does not get a high priority if the parent thread is running with high priority
	setpriority(PRIO_PROCESS, 0, niceVal);

	This->mEventThreadInitEvent.signal();

	DaqEvent currentEvents[MAX_EVENT_TYPE_COUNT];
	int eventCount = 0;

	while (!This->mTerminateEventThread)
	{
		This->waitForEvent();

		if(This->mDaqEventOccured)
		{
			This->getCurrentEventsAndData(currentEvents, eventCount);

			for(int i = 0; i < eventCount; i++)
			{
				currentEvents[i].callbackFunction(This->mDaqDevice.getDeviceNumber(), currentEvents[i].type, currentEvents[i].eventData, currentEvents[i].userData);
			}
		}
	}

	return NULL;
}

void DaqEventHandler::waitForEvent()
{
	if(!mDaqEventOccured && !mTerminateEventThread)
	{
		mNotifier.wait_for_signal();
	}
}


void DaqEventHandler::check_EnableEvent_Args(DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCalbackFunc)
{
	if((eventTypes == 0) || (eventTypes & ~mDaqDevice.getDevInfo().getEventTypes()))
		throw UlException(ERR_BAD_EVENT_TYPE);

	if(eventTypes & mEnabledEventsTypes)
		throw UlException(ERR_EVENT_ALREADY_ENABLED);

	if((eventTypes & DE_ON_DATA_AVAILABLE) && (eventParameter == 0))
		throw UlException(ERR_BAD_EVENT_PARAMETER);

	if(mDaqDevice.isScanRunning())
		throw UlException(ERR_ALREADY_ACTIVE);

	if(eventCalbackFunc == NULL)
		throw UlException(ERR_BAD_CALLBACK_FUCNTION);
}

void DaqEventHandler::check_DisableEvent_Args(DaqEventType eventTypes)
{
	//if(eventTypes & ~mDaqDevice.getDevInfo().getEventTypes())
	//	throw UlException(ERR_BAD_EVENT_TYPE);
}

unsigned int DaqEventHandler::getEventIndex(DaqEventType eventType)
{
	int index = 0;
	switch(eventType)
	{
	case DE_ON_DATA_AVAILABLE:
		index =	0;
		break;
	case DE_ON_INPUT_SCAN_ERROR:
		index =	1;
		break;
	case DE_ON_END_OF_INPUT_SCAN:
		index =	2;
		break;
	case DE_ON_OUTPUT_SCAN_ERROR:
		index =	3;
		break;
	case DE_ON_END_OF_OUTPUT_SCAN:
		index =	4;
		break;
	default:
		std::cout << "**** getEventIndex(), Invalid event type specified";
		break;
	}
	return index;
}

} /* namespace ul */
