/*
 * DaqDeviceId.h
 *
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
		USB_1608HS_2AO = 0x99,
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
		USB_1608HS = 0xbd,


		USB_1208HS = 0xc4,
		USB_1208HS_2AO = 0xc5,
		USB_1208HS_4AO = 0xc6,
		USB_QUAD08 = 0xca,
		USB_2416 = 0xd0,
		USB_2416_4AO = 0xd1,
		USB_1208FS_PLUS = 0xe8,
		USB_1408FS_PLUS = 0xe9,
		USB_1608FS_PLUS = 0xea,
		USB_2001_TC = 0xf9,
		USB_2408 = 0xfd,
		USB_2408_2AO = 0xfe,
		USB_1608G = 0x110,
		USB_1608GX = 0x111,
		USB_1608GX_2AO = 0x112,
		USB_2633 = 0x118,
		USB_2637 = 0x119,
		USB_2623 = 0x120,
		USB_2627 = 0x121,

		USB_2020 = 0x11c,

		USB_201 = 0x113,
		USB_202 = 0x12b,
		USB_204 = 0x114,
		USB_205 = 0x12c,
		USB_CTR08 = 0x127,
		USB_CTR04 = 0x12e,
		E_1608 = 0x12f,
		USB_TC_32 = 0x131,
		E_TC_32 = 0x132,
		USB_DIO32HS = 0x133,
		USB_1608G_2 = 0x134,
		USB_1608GX_2 = 0x135,
		USB_1608GX_2AO_2 = 0x136,
		E_DIO24 = 0x137,
		E_TC = 0x138,
		USB_1808 = 0x13d,
		USB_1808X = 0x13e,

		USB_TO_ETH = 0x202,
		E_1808 = 0x203,
		E_1808X = 0x204,

		PDAQ3KLD = 0x0470,
		DT9837_ABC_LD = 0x9839,
		DT9837_ABC = 0x3998,

		// the DT9837A DT9837B and DT9837C devices all have the same PID added the following IDs to be able identify
		// products

		UL_DT9837_A = 0x3998A,
		UL_DT9837_B = 0x3998B,
		UL_DT9837_C = 0x3998C
	};
};

} /* namespace ul */

#endif /* DAQDEVICEID_H_ */
