/*
 * ErrorMap.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef ERRORMAP_H_
#define ERRORMAP_H_

#include <map>
#include <string>

#include "../ul_internal.h"

namespace ul
{

class UL_LOCAL ErrorMap
{
public:
	static ErrorMap& instance()
	{
		static ErrorMap mInstance;
		return mInstance;
	}

	static void init()
	{
		instance();
	}

	std::string getErrorMsg(int errNum);

protected:
	ErrorMap();

private:
	std::map<int,std::string> mErrMap;
};

} /* namespace ul */

#endif /* ERRORMAP_H_ */
