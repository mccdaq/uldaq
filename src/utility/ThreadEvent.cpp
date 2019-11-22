/*
 * ThreadEvent.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include <stdint.h>

#include "ThreadEvent.h"

namespace ul
{

ThreadEvent::ThreadEvent()
{
	pthread_mutex_init(&m_cond_val_mutex, NULL);
	pthread_cond_init(&m_cond_var, NULL);

	m_signaled = false;
}

ThreadEvent::~ThreadEvent()
{
	pthread_mutex_destroy(&m_cond_val_mutex);
	pthread_cond_destroy(&m_cond_var);
}

void ThreadEvent::wait_for_signal()
{
	pthread_mutex_lock(&m_cond_val_mutex);

	while(!m_signaled)
		pthread_cond_wait(&m_cond_var, &m_cond_val_mutex);

	m_signaled = false;

	pthread_mutex_unlock(&m_cond_val_mutex);
}

int ThreadEvent::wait_for_signal(unsigned long long timeout_us)
{
	int result = 0;

	pthread_mutex_lock(&m_cond_val_mutex);

	if(!m_signaled)
	{
		struct timespec now, wait_until;

		ul_clock_realtime(&now);

		uint64_t nanoseconds = ((uint64_t) now.tv_sec) * 1000 * 1000 * 1000 + timeout_us * 1000 + ((uint64_t) now.tv_nsec);

		wait_until.tv_sec = nanoseconds / 1000 / 1000 / 1000;
		wait_until.tv_nsec = (nanoseconds - ((uint64_t) wait_until.tv_sec) * 1000 * 1000 * 1000);

		do
		{
			result = pthread_cond_timedwait(&m_cond_var, &m_cond_val_mutex, &wait_until);
		}
		while (result == 0 && !m_signaled);

		if(result == 0)
			m_signaled = false;

	}
	else
		m_signaled = false;

	pthread_mutex_unlock(&m_cond_val_mutex);

	return result;
}

void ThreadEvent::signal()
{
	pthread_mutex_lock(&m_cond_val_mutex);

	m_signaled = true;

	pthread_cond_signal(&m_cond_var);

	pthread_mutex_unlock(&m_cond_val_mutex);

}

void ThreadEvent::reset()
{
	pthread_mutex_lock(&m_cond_val_mutex);

	m_signaled = false;

	pthread_mutex_unlock(&m_cond_val_mutex);
}

} /* namespace ul */


