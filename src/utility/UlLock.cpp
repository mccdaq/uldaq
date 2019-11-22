/*
 * UlLock.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UlLock.h"
#include "FnLog.h"

namespace ul
{

UlLock::UlLock(pthread_mutex_t& mutex): mMutex(mutex)
{
	pthread_mutex_lock(&mMutex);

	//UL_LOG(">>>>>>>>> Lock: "<< &mMutex << " acquired");
}

UlLock::~UlLock()
{
	pthread_mutex_unlock(&mMutex);

	//UL_LOG("<<<<<<<<< Lock: "<< &mMutex << " released");
}

void UlLock::initMutex(pthread_mutex_t& mutex, int mutexType)
{
	FnLog log("UlLock::initMutex");

	pthread_mutexattr_t mutexAttr;

	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_settype(&mutexAttr, mutexType);
	pthread_mutex_init(&mutex, &mutexAttr);
	pthread_mutexattr_destroy(&mutexAttr);
}

void UlLock::destroyMutex(pthread_mutex_t& mutex)
{
	FnLog log("UlLock::destroyMutex");

	pthread_mutex_destroy(&mutex);
}

} /* namespace ul */
