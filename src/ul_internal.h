/*
 * ul_internal.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERNAL_H_
#define INTERNAL_H_

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <sys/time.h>

#include "uldaq.h"

#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace ul
{

#define UL_VERSION		"1.2.0"

#ifdef DEBUG
//	#define TRACE
#endif

#ifndef _WIN32
	#define UL_LOCAL __attribute__((visibility("hidden")))
#else
	#define UL_LOCAL
#endif


static inline char *time_now();

#ifdef TRACE

#define UL_LOG(msg) do {char* t = time_now(); std::cout << t << " " << msg << std::endl; } while(false)
#else
#define UL_LOG(x)
#endif

static inline char *time_now()
{
	const int buffer_size = 100;
	    static char buffer[buffer_size];
	    static char buffer_msec[buffer_size];
	    struct tm *timeinfo;

	    timeval now;
	    gettimeofday(&now, NULL);
	    int micro_sec = now.tv_usec;

	    timeinfo = localtime(&now.tv_sec);

	    strftime(buffer, buffer_size, "%Y-%b-%d %H:%M:%S", timeinfo);

	    snprintf(buffer_msec, buffer_size, "[%s:%06d]", buffer, micro_sec);

	    return buffer_msec;
}


static inline void ul_clock_realtime (struct timespec* ts)
{
#ifdef __APPLE__
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;

#else
	  clock_gettime(CLOCK_REALTIME, ts);
#endif
}


	typedef struct
	{
		double slope;
		double offset;
	} CalCoef;

	typedef struct
	{
		double slope;
		double offset;
	} CustomScale;


	typedef enum
	{
		MT_EEPROM = 1
		//FLASH
	} MemoryType;


	typedef enum FunctionType
	{
		FT_AI = 1,
		FT_AO = 2,
		FT_DI = 3,
		FT_DO = 4,
		FT_CTR = 5,
		FT_TMR = 6,
		FT_DAQI = 7,
		FT_DAQO = 8
	} FunctionType;

	typedef enum
	{
		TS_IDLE = 0,
		TS_RUNNING = 1
	} XferState;

	typedef enum
	{
		SD_INPUT = 1,
		SD_OUTPUT = 2
	} ScanDirection;

	typedef struct
	{
		TriggerType type;
		int trigChan;
		double level;
		double variance;
		unsigned int retrigCount;
	} TriggerConfig;

	typedef enum
	{
		DATA_UINT64 = 1,
		DATA_DBL	= 2
	}ScanDataBufferType;

	typedef enum
	{
		DAQI_CTR64_INTERNAL = 1 << 30
	}DaqIInternalChanType;
}


#endif /* INTERNAL_H_ */
