/*
 * IntelHexRec.h
 *
 *  Author: Measurement Computing Corporation
 */

#ifndef USB_FW_INTELHEXREC_H_
#define USB_FW_INTELHEXREC_H_

#define MAX_INTEL_HEX_RECORD_LENGTH 16

typedef struct _INTEL_HEX_RECORD
{
	unsigned char  Length;
	unsigned short Address;
	unsigned char  Type;
	unsigned char  Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;



#endif /* USB_FW_INTELHEXREC_H_ */
