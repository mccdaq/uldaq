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

	virtual void setPortInitialOutputVal(int portNum, unsigned long long val);

	virtual void setPortIsoMask(int portNum, unsigned long long mask);
	virtual unsigned long long getPortIsoMask(int portNum);
	virtual unsigned long long getPortLogic(int portNum);

private:
	DioDevice& mDioDevice;
};

} /* namespace ul */

#endif /* DIOCONFIG_H_ */
