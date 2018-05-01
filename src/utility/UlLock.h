/*
 * UlLock.h
 *
 *  Created on: Aug 29, 2015
 *     Author: Measurement Computing Corporation
 */

#ifndef ULLOCK_H_
#define ULLOCK_H_

#include "../ul_internal.h"

namespace ul
{

class UL_LOCAL UlLock
{
public:
	explicit UlLock(pthread_mutex_t& mutex);
	virtual ~UlLock();

	static void initMutex(pthread_mutex_t& mutex, int mutexType);
	static void destroyMutex(pthread_mutex_t& mutex);

private:
	pthread_mutex_t& mMutex;
};

} /* namespace ul */

#endif /* ULLOCK_H_ */
