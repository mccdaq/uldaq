/*
 * Endian.cpp
 *
 *  Created on: Dec 4, 2015
 *     Author: Measurement Computing Corporation
 */

#include "Endian.h"

namespace ul
{

Endian::Endian()
{
	mLittleEndian = isLittleEndian();
}

} /* namespace ul */
