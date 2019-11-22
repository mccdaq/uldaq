/*
 * ThreadEvent.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef UTILITY_THREADEVENT_H_
#define UTILITY_THREADEVENT_H_

#include "../ul_internal.h"
#include <errno.h>

namespace ul
{

class ThreadEvent
{
public:
	ThreadEvent();
	virtual ~ThreadEvent();

	void wait_for_signal();
	int wait_for_signal(unsigned long long timeout_us);
	void signal();
	void reset();

private:
	pthread_cond_t m_cond_var;
	pthread_mutex_t m_cond_val_mutex;

	bool m_signaled;
};

} /* namespace ul */

#endif /* UTILITY_THREADEVENT_H_ */
