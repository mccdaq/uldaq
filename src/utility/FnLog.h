/*
 * FLog.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef FNLOG_H_
#define FNLOG_H_

# include <string>

#include "../ul_internal.h"
#include <sys/time.h>

//#define FNLOG

namespace ul
{

class UL_LOCAL FnLog
{
public:
	FnLog(std::string log);
	virtual ~FnLog();
private:
	std::string mLog;
#ifdef FNLOG
	timeval mEnterTime;
#endif
};

} /* namespace ul */

#endif /* FLOG_H_ */
