/*
 * DaqDeviceId.h
 *
 *  Created on: Jul 30, 2015
 *     Author: Measurement Computing Corporation
 */

#ifndef DAQDEVICEID_H_
#define DAQDEVICEID_H_

#include <string>

namespace ul
{

class DaqDeviceId
{

public:
	enum
	{
		USB_TC = 0x90,
		USB_1208FS_PLUS = 0xe8,
		USB_1408FS_PLUS = 0xe9,
		USB_1608FS_PLUS = 0xea,
		USB_1208HS = 0xc4,
		USB_1208HS_2AO = 0xc5,
		USB_1208HS_4AO = 0xc6,
		USB_1608G = 0x110,
		USB_1608GX = 0x111,
		USB_1608GX_2AO = 0x112,
		USB_2633 = 0x118,
		USB_2637 = 0x119,
		USB_2623 = 0x120,
		USB_2627 = 0x121,

		USB_201 = 0x113,
		USB_202 = 0x12b,
		USB_204 = 0x114,
		USB_205 = 0x12c,
		USB_CTR08 = 0x127,
		USB_CTR04 = 0x12e,
		USB_DIO32HS = 0x133,
		USB_1608G_2 = 0x134,
		USB_1608GX_2 = 0x135,
		USB_1608GX_2AO_2 = 0x136,
		USB_1808 = 0x13d,
		USB_1808X = 0x13e
	};
};

} /* namespace ul */

#endif /* DAQDEVICEID_H_ */
