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
		USB_1024LS = 0x76,
		USB_1024HLS = 0x7f,
		USB_SSR24 = 0x85,
		USB_SSR08 = 0x86,
		USB_ERB24 = 0x8a,
		USB_ERB08 = 0x8b,
		USB_PDISO8 = 0x8c,
		USB_TEMP = 0x8d,
		USB_TC = 0x90,
		USB_DIO96H = 0x92,
		USB_DIO24 = 0x93,
		USB_DIO24H = 0x94,
		USB_DIO96H_50 = 0x95,
		USB_PDISO8_40 = 0x96,
		USB_3101 =	0x9a,
		USB_3102 =	0x9b,
		USB_3103 =	0x9c,
		USB_3104 =	0x9d,
		USB_3105 =	0x9e,
		USB_3106 =	0x9f,
		USB_3110 =	0xa2,
		USB_3112 =	0xa3,
		USB_3114 =	0xa4,
		USB_TC_AI = 0xbb,
		USB_TEMP_AI = 0xbc,

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
