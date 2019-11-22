/*
 * Endian.cpp
 *
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
