/*
 * UlDioConfig.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef INTERFACES_ULDIOCONFIG_H_
#define INTERFACES_ULDIOCONFIG_H_

namespace ul
{

class UlDioConfig
{
public:
	virtual ~UlDioConfig() {};

	virtual unsigned long long getPortDirectionMask(int portNum) = 0;
	virtual void setPortInitialOutputVal(int portNum, unsigned long long val) = 0;
	virtual void setPortIsoMask(int portNum, unsigned long long mask) = 0;
	virtual unsigned long long getPortIsoMask(int portNum) = 0;
	virtual unsigned long long getPortLogic(int portNum) = 0;
};

} /* namespace ul */

#endif /* INTERFACES_ULDIOCONFIG_H_ */
