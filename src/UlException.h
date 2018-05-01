/*
 * UlException.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef UlException_H_
#define UlException_H_

#include <exception>
#include <string>

#include "uldaq.h"

namespace ul
{

class UlException: public std::exception
{
public:
	UlException(UlError err);
	virtual ~UlException () throw() {};

	UlError getError() {return mError;}

	virtual const char* what() const throw();

private:
	std::string mStr;
	UlError		mError;
};

} /* namespace ul */

#endif /* UlException_H_ */
