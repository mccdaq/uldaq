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
};

} /* namespace ul */

#endif /* INTERFACES_ULDIOCONFIG_H_ */
