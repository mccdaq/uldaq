/*
 * DaqEvent.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQEVENT_H_
#define DAQEVENT_H_

struct DaqEvent
{
	DaqEventType type;
	unsigned long long eventParameter;
	DaqEventCallback callbackFunction;
	void* userData;
	unsigned long long eventData;
	bool eventOccured;

	DaqEvent()
	{
		type = (DaqEventType) 0;
		eventParameter = 0;
		callbackFunction = NULL;
		userData = NULL;
		eventData = 0;
		eventOccured = false;
	}
};

typedef struct 	DaqEvent DaqEvent;


#endif /* DAQEVENT_H_ */
