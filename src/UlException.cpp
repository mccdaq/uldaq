/*
 * UlException.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "./UlException.h"

#include "./utility/ErrorMap.h"

namespace ul
{

UlException::UlException(UlError err)
{
	mError = err;
	mStr = ErrorMap::instance().getErrorMsg(err);
}



const char* UlException::what() const throw()
{
	return mStr.c_str();
}

} /* namespace ul */
