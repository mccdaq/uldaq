/*
 * FLog.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "FnLog.h"
#include <iostream>

namespace ul
{

FnLog::FnLog(std::string log)
{
#ifdef FNLOG
	std::cout << log << " <----" << std::endl;
	mLog = log;

	//mEnterTime = clock();
	gettimeofday(&mEnterTime, NULL);

#endif
}

FnLog::~FnLog()
{
#ifdef FNLOG

	timeval exitTime;
	gettimeofday(&exitTime, NULL);

	double t0 = (mEnterTime.tv_sec * 1000000.0 + mEnterTime.tv_usec) / 1000000.0;
	double t1 = (exitTime.tv_sec * 1000000.0 + exitTime.tv_usec) / 1000000.0;

	double  duration = t1 - t0;

	//double  duration = (double)(clock() - mEnterTime) / CLOCKS_PER_SEC;

	std::cout << mLog << " ---->  function duration: " << duration * 1000.0 << " ms"<< std::endl;
#endif
}

} /* namespace ul */
