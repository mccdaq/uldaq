/*
 * EuScale.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef UTILITY_EUSCALE_H_
#define UTILITY_EUSCALE_H_

#include "../uldaq.h"
namespace ul
{

class EuScale
{
public:
	static void getEuScaling(Range range, double &scale, double &offset);
};

} /* namespace ul */

#endif /* UTILITY_EUSCALE_H_ */
