/*
 * VirNet.h
 *
 *  Created on: Oct 28, 2019
 *      Author: root
 */

#ifndef VIRNET_H_
#define VIRNET_H_

#define VIR_NET_SAMPLE_SIZE 8 // regardless of the board and subsystem the same size is always is 8 bytes, since it is from the UL buffer

#pragma pack(1)
	typedef struct
	{
		unsigned char delimiter;
		unsigned short command;
		unsigned char frameId;
		unsigned char status;
		unsigned short count;
		unsigned char data[1];

	}TNetFrameVir;
#pragma pack()

typedef enum
{
	DSS_MAIN = 0x100,
	DSS_AIN  = 0x200

}DevSubSystem;

typedef enum
{
	VNC_TEST = DSS_MAIN | 0x01,
	VNC_XFER_IN_STATE = DSS_MAIN | 0x02,
	VNC_XFER_OUT_STATE = DSS_MAIN | 0x03,
	VNC_FLASH_LED = DSS_MAIN | 0x04,

	VNC_AIN = DSS_AIN | 0x01,
	VNC_AINSCAN = DSS_AIN | 0x02,
	VNC_AINSCAN_STOP = DSS_AIN | 0x03,
	VNC_AIN_LOAD_QUEUE = DSS_AIN | 0x04,

	//// last command is 0x7FFF since bit 15 is set when device responds
}VirNetCmd;

#pragma pack(1)
	typedef struct
	{
		unsigned char flashCount;
	}TFlashLedParams;


/////////////// Analog input functions
	typedef struct
	{
		unsigned char channel;
		unsigned char inputMode;
		unsigned char range;
		unsigned char flags;
	}TAInParams;

	typedef struct
	{
		unsigned char lowChan;
		unsigned char highChan;
		unsigned char inputMode;
		unsigned char range;
		int samplesPerChan;
		double rate;
		unsigned int options;
		unsigned char flags;
	}TAInScanParams;

	typedef struct
	{
		unsigned char channel;
		unsigned char inputMode;
		unsigned char range;
	}TALoadQueueElement;

	typedef struct
	{
		unsigned char numElements;
		TALoadQueueElement elements[256];
	}TALoadQueueParams;


	typedef struct
	{
		unsigned char dataSocketReady;
		unsigned char active;
		unsigned char error;

	}TXferInState;


//////////////////////////////////////////////

#pragma pack()



#endif /* VIRNET_H_ */
