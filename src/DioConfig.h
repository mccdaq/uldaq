/*
 * DioConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef DIOCONFIG_H_
#define DIOCONFIG_H_

#include "interfaces/UlDioConfig.h"
#include "ul_internal.h"

namespace ul
{
class DioDevice;
class UL_LOCAL DioConfig: public UlDioConfig
{
public:
	DioConfig(DioDevice& dioDevice);
	virtual ~DioConfig();

	virtual unsigned long long getPortDirectionMask(int portNum);

private:
	DioDevice& mDioDevice;
};

} /* namespace ul */

#endif /* DIOCONFIG_H_ */
