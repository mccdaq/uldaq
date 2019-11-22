/*
 * Usb9837x.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef DT_USB9837XDEFS_H_
#define DT_USB9837XDEFS_H_

namespace ul
{

class Usb9837xDefs
{
public:
	/********************************************************************


	   USB Pipe Definitions

	 ********************************************************************/

	typedef enum
	{
	   READ_MSG_PIPE = 0x88,           	// Async messages to driver
	   WRITE_CMD_PIPE = 0x01,        	// Driver sends commands to firmware
	   READ_CMD_PIPE = 0x81,           	// Firmware returns data in response to cmd
	   WRITE_STREAM_PIPE = 0x06,      	// DAC output stream
	   READ_STREAM_PIPE = 0x82        	// AD input stream
	 } DT9837A_PIPES;

	 // Maximum size of packets on the WRITE_CMD_PIPE
	 const static int MAX_WRITE_CMD_PIPE_SIZE =  64;
	 // Maximum size of packets on the READ_MSG_PIPE
	 const static int MAX_READ_MSG_PIPE_SIZE = 64;

	typedef enum
	{
	   LEAST_USB_FIRMWARE_CMD_CODE = 0,

	         // USB registers read/write commands
	   R_BYTE_USB_REG = 0,			// Read a single byte of USB memory
	   W_BYTE_USB_REG = 1,			// Write a single byte of USB memory
	   R_MULTI_BYTE_USB_REG = 2,	// Multiple Reads of USB memory
	   W_MULTI_BYTE_USB_REG = 3,	// Multiple Writes of USB memory
	   RMW_BYTE_USB_REG = 4,		// Read, (AND) with mask, OR value, then write (single)
	   RMW_MULTI_BYTE_USB_REG = 5,	// Read, (AND) with mask, OR value, then write (multiple)

	      // I2C Register read/write commands
	   R_BYTE_I2C_REG = 10,			// Read a single byte of an I2C device
	   W_BYTE_I2C_REG = 11,			// Write a single byte of an I2C device
	   R_MULTI_BYTE_I2C_REG = 12,	// Multiple Reads of a device
	   W_MULTI_BYTE_I2C_REG = 13,	// Multiple Writes of a device

	     // read/write commands for Local Bus
	   R_SINGLE_WORD_LB = 20,		// Read single word of registers mapped through the local bus
	   W_SINGLE_WORD_LB = 21,       // Write a single word of registers mapped through the local bus
	   R_MULTI_WORD_LB = 22,        // Multiple Reads of registers mapped through the local bus
	   W_MULTI_WORD_LB = 23,        // Multiple Writes of registers mapped through the local bus
	   RMW_SINGLE_WORD_LB = 24,     // Read, (AND) with mask, OR value, then write (single word)
	   RMW_MULTI_WORD_LB = 25,      // Read, (AND) with mask, OR value, then write (multiple word)

	   // Subsystem commands
	   START_SUBSYSTEM = 30,		// Issue a start command to a given subsystem
	   STOP_SUBSYSTEM = 31,         // Issue a stop command to a given subsystem
	   R_SINGLE_VALUE_CMD = 32,		// Read a single value from a subsystem
	   R_SINGLE_VALUES_CMD = 33,	// Read a single value from all chans in subsystem

		// Miscellaneous
	   W_DATA_BLOCK_GPIF_BUS = 34,		// Write a FIFO block size for D/A convertr
	   W_MULTI_BYTE_PLL_REG = 40,		// Write PLL Osc to program A/D frequency
	   W_PMC1737_DAC_REG = 41,		// Write PMC1737 registers
	   R_PMC1737_DAC_REG= 42,		// Read PMC1737 registers
	   START_8051_DEBUGGER = 43,	// Start the debugger on the USB processor

	   // Calibration Pots
	   WRITE_CAL_POT = 50,			// Write to a calibration pot
	   READ_CAL_POT = 51,			// Read a calibration pot

	   DATA_ACQ_POWER = 52,      	// Command for turning on/off data acquisition board power, +/-15V etc.
	   W_SINGLE_VALUE_CMD = 53,	// Write a single value to a subsystem
	   W_SINGLE_BYTE_PLL_REG = 54,
	   SIM_START_SUBSYSTEM = 55,		// Issue a start command to a given subsystem
	   MAX_USB_FIRMWARE_CMD_CODE    // Valid USB_FIRMWARE_CMD_CODE's will be less than
	                                // this number
	 } USB_FIRMWARE_CMD_CODE;

#pragma pack(1)
	 //
	 // This structure converts a 4 byte array to unsigned long
	 // to ease conversion between big and little endian values
	 //
	 typedef union _DWORD_STRUCT
	 {
	    unsigned char ByteVals[4];
	    unsigned int DWordVal;
	 } DWORD_STRUCT, *PDWORD_STRUCT;

	 /******************************************************************************
	  Structures used for I/O to USB registers
	  ******************************************************************************/
	 /* This structure is used with the W_SINGLE_BYTE_DEV */
	 typedef struct
	 {
	    unsigned char        DevAddress;    // Device address
	    unsigned char        Register;      // A register or entity within the device
	    unsigned char        DataVal;       // Data value to write to the Register
	 } WRITE_DEV_BYTE_INFO, *pWRITE_DEV_BYTE_INFO;


	 /* This structure is used with the W_BYTE_USB_REG */
	 typedef struct
	 {
	    unsigned char             Address;
	    unsigned char             DataVal;
	 } WRITE_BYTE_INFO, *pWRITE_BYTE_INFO;


	 /* This structure is used with the R_BYTE_USB_REG */
	 typedef struct
	 {
	    unsigned char             Address;
	 } READ_BYTE_INFO, *pREAD_BYTE_INFO;


	 const static int MAX_NUM_MULTI_BYTE_WRTS =  (( MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (WRITE_BYTE_INFO));

	 /* This structure is used with the R_MULTI_BYTE_USB_REG */
	 typedef struct
	 {
	    unsigned char             NumWrites;
	    WRITE_BYTE_INFO  Write[MAX_NUM_MULTI_BYTE_WRTS];
	 } WRITE_MULTI_INFO, *pWRITE_MULTI_INFO;


	 const static int MAX_NUM_MULTI_BYTE_RDS = ((MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (unsigned char));
	 /* This structure is used with the R_MULTI_BYTE_USB_REG */
	 typedef struct
	 {
	    unsigned char             NumReads;
	    unsigned char             Addresses[MAX_NUM_MULTI_BYTE_RDS];
	 } READ_MULTI_INFO, *pREAD_MULTI_INFO;



	 /* This structure is used with the RMW_BYTE_USB_REG,
	    and RMW_MULTI_BYTE_USB_REG */
	 typedef struct
	 {
	    unsigned char             Address;
	    unsigned char             AndMask;
	    unsigned char             OrVal;
	 } RMW_BYTE_INFO, *pRMW_BYTE_INFO;

	    /* This structure is used with the RMW_MULTI_BYTE_USB_REG and
	    RMW_MULTI_BYTE_IOP_REG  */
	 const static int MAX_NUM_MULTI_BYTE_RMWS =  ((MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (RMW_BYTE_INFO));
	 typedef struct
	 {
	    unsigned char             NumRMWs;
	    RMW_BYTE_INFO    ByteInfo[MAX_NUM_MULTI_BYTE_RMWS];
	 } RMW_MULTI_INFO, *pRMW_MULTI_INFO;



	  /******************************************************************************
	  Structures used for I/O to I2C devices
	  ******************************************************************************/

	 /* This structure is used with the W_BYTE_I2C_REG */
	 typedef struct
	 {
	    unsigned char        DevAddress;    // Device address
	    unsigned char        Register;      // A register or entity within the device
	    unsigned char        DataVal;       // Data value to write to the Register
	 } WRITE_I2C_BYTE_INFO, *pWRITE_I2C_BYTE_INFO;

	 /* This structure is used with the R_BYTE_I2C_REG */
	 typedef struct
	 {
	    unsigned char        DevAddress;    // Device address
	    unsigned char        Register;      // A register or entity within the device
	 } READ_I2C_BYTE_INFO, *pREAD_I2C_BYTE_INFO;

	 const static int MAX_NUM_I2C_MULTI_BYTE_WRTS = (( MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (WRITE_I2C_BYTE_INFO));

	 /* This structure is used with the W_MULTI_BYTE_I2C_REG */
	 typedef struct
	 {
	    unsigned char             NumWrites;
	    WRITE_I2C_BYTE_INFO  Write[MAX_NUM_I2C_MULTI_BYTE_WRTS];
	 } WRITE_I2C_MULTI_INFO, *pWRITE_I2C_MULTI_INFO;

	 const static int MAX_NUM_I2C_MULTI_BYTE_RDS = (( MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (READ_I2C_BYTE_INFO));

	 /* This structure is used with the R_MULTI_BYTE_I2C_REG */
	 typedef struct
	 {
	    unsigned char             NumReads;
	    READ_I2C_BYTE_INFO  Read[MAX_NUM_I2C_MULTI_BYTE_RDS];
	 } READ_I2C_MULTI_INFO, *pREAD_I2C_MULTI_INFO;



	  /******************************************************************************
	  Structures used for reading and writing to the USB external memory
	  ******************************************************************************/

	 /* This structure is used with the W_SINGLE_BYTE_USB_EXT_MEM_ */
	 typedef struct
	 {
	    unsigned short             Address;
	    unsigned short             DataVal;
	 } WRITE_BYTE_USB_MEM_INFO, *pWRITE_BYTE_USB_MEM_INFO;

	 /* This structure is used with the R_SINGLE_BYTE_USB_EXT_MEM */
	 typedef struct
	 {
	    unsigned short             Address;
	 } READ_BYTE_USB_MEM_INFO, *pREAD_BYTE_USB_MEM_INFO;

	 const static int MAX_NUM_MULTI_BYTE_WRTS_USB_MEM  = (( MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (WRITE_BYTE_USB_MEM_INFO));
	 /* This structure is used with the W_MULTI_BYTE_USB_EXT_MEM */

	 typedef struct
	 {
	    unsigned char             NumWrites;
	    WRITE_BYTE_USB_MEM_INFO  Write[MAX_NUM_MULTI_BYTE_WRTS_USB_MEM];
	 } WRITE_MULTI_BYTE_USB_MEM_INFO, *pWRITE_MULTI_BYTE_USB_MEM_INFO;


	 const static int MAX_NUM_MULTI_BYTE_RDS_USB_MEM = ((MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (unsigned short));
	 /* This structure is used with the R_MULTI_BYTE_USB_EXT_MEM */

	 typedef struct
	 {
	    unsigned char             NumReads;
	    unsigned short             Addresses[MAX_NUM_MULTI_BYTE_RDS_USB_MEM];
	 } READ_MULTI_BYTE_USB_MEM_INFO, *pREAD_MULTI_BYTE_USB_MEM_INFO;

	 /* This structure is used with the RMW_BYTE_USB_EXT_MEM,
	    and RMW_MULTI_BYTE_USB_EXT_MEM */

	 typedef struct
	 {
	    unsigned short             Address;
	    unsigned short             AndMask;
	    unsigned short             OrVal;
	 } RMW_BYTE_USB_MEM_INFO, *pRMW_BYTE_USB_MEM_INFO;

	    /* This structure is used with the RMW_MULTI_WORD_LB and
	    RMW_MULTI_BYTE_IOP_REG
	 */
	 const static int MAX_NUM_MULTI_BYTE_RMWS_USB_MEM  = ((MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (RMW_BYTE_USB_MEM_INFO));
	 typedef struct
	 {
	    unsigned char             NumRMWs;
	    RMW_BYTE_USB_MEM_INFO    WordInfo[MAX_NUM_MULTI_BYTE_RMWS_USB_MEM];
	 } RMW_MULTI_BYTE_USB_MEM_INFO, *pRMW_MULTI_BYTE_USB_MEM_INFO;

	 /******************************************************************************
	  Structures used for I/O to device registers on Local Bus
	  ******************************************************************************/

	 /* This structure is used with the R_SINGLE_WORD_LB */

	 typedef struct
	 {
	    unsigned short             Address;
	    unsigned short             DataVal;
	 } WRITE_WORD_INFO, *pWRITE_WORD_INFO;

	 /* This structure is used with the W_SINGLE_WORD_LB */

	 typedef struct
	 {
	    unsigned short             Address;
	 } READ_WORD_INFO, *pREAD_WORD_INFO;

	 const static int MAX_NUM_MULTI_WORD_WRTS = (( MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (WRITE_WORD_INFO));
	 /* This structure is used with the W_MULTI_WORD_LB */

	 typedef struct
	 {
	    unsigned char             NumWrites;
	    WRITE_WORD_INFO  Write[MAX_NUM_MULTI_WORD_WRTS];
	 } WRITE_MULTI_WORD_INFO, *pWRITE_MULTI_WORD_INFO;


	 const static int MAX_NUM_MULTI_WORD_RDS  = ((MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (unsigned short));
	 /* This structure is used with the R_MULTI_BYTE_REG */

	 typedef struct
	 {
	    unsigned char             NumReads;
	    unsigned short             Addresses[MAX_NUM_MULTI_WORD_RDS];
	 } READ_MULTI_WORD_INFO, *pREAD_MULTI_WORD_INFO;

	 /* This structure is used with the RMW_SINGLE_WORD_LB,
	    and RMW_MULTI_WORD_LB */

	 typedef struct
	 {
	    unsigned short             Address;
	    unsigned short             AndMask;
	    unsigned short             OrVal;
	 } RMW_WORD_INFO, *pRMW_WORD_INFO;

	    /* This structure is used with the RMW_MULTI_WORD_LB and
	    RMW_MULTI_BYTE_IOP_REG  */

	 const static int MAX_NUM_MULTI_WORD_RMWS = ((MAX_WRITE_CMD_PIPE_SIZE - 4 - 1) / sizeof (RMW_WORD_INFO));
	 typedef struct
	 {
	    unsigned char             NumRMWs;
	    RMW_WORD_INFO    WordInfo[MAX_NUM_MULTI_WORD_RMWS];
	 } RMW_MULTI_WORD_INFO, *pRMW_MULTI_WORD_INFO;


	  /******************************************************************************
	  Structures used for subsystem commands
	  ******************************************************************************/

	 //  This enum is a mirror image of OLSS enumerated type declared in oldacfg.h
	  typedef enum
	 {
	    SS_AD,
	    SS_DA,
	    SS_DIN,
	    SS_DOUT,
	    SS_SRL,
	    SS_CT
	 } SUBSYSTEM_TYPE;

	 // Structure used by board drivers to pass Subsystem information to the firmware.
	 typedef struct
	 {
		 unsigned int    SubsystemType; //SUBSYSTEM_TYPE    SubsystemType; // Specifies  the subsystem type

	    // If compiling USB firmware then add filler since Windows compiler
	    // uses 4 byte enums rather than Keil's 1 byte
	    /*#ifndef WIN_DRIVER
	    unsigned char                  Filler[3];
	    #endif*/
	    unsigned short             ExtTrig;          // board/subsystem  specific flags

	 } SUBSYSTEM_INFO, *pSUBSYSTEM_INFO;

	 //   Structure used for accessing ReadSingleValue commands
	 typedef struct
	 {
	   unsigned char			Channel;
	   unsigned char   		Gain;
	 } READ_SINGLE_VALUE_INFO, *pREAD_SINGLE_VALUE_INFO;


	 //   Structure used for accessing ReadSingleValue commands
	 typedef struct
	 {
	   unsigned char			NumChans;
	   unsigned char			Gain;
	 } READ_SINGLE_VALUES_INFO, *pREAD_SINGLE_VALUES_INFO;



	 //   Structure used for accessing WriteSingleValue commands
	 typedef struct
	 {
		unsigned int    SubsystemType; // SB: actual type is  SUBSYSTEM_TYPE    SubsystemType; // Specifies  the subsystem type

	   /*#ifndef WIN_DRIVER
	   unsigned char                  Filler[3];
	   #endif*/

	   unsigned char  Channel;
	   unsigned int DataValue;
	 } WRITE_SINGLE_VALUE_INFO, *pWRITE_SINGLE_VALUE_INFO;

	 /* This structure is used with the R_MULTI_BYTE_USB_REG */
	 typedef struct
	 {
		unsigned int    SubsystemType; // SB: actual type is  SUBSYSTEM_TYPE    SubsystemType; // Specifies  the subsystem type

	    // If compiling USB firmware then add filler since Windows compiler
	    // uses 4 byte enums rather than Keil's 1 byte
	 	// Due to the address SubsystemType and 3 fillers,
	 	// we further decrement the write array by 4 bytes in order to bring back
	 	// the size to 64 bytes.
	    /*#ifndef WIN_DRIVER
	    unsigned char                  Filler[3];
	    #endif*/
	    unsigned char				DevAddr;
	    unsigned char             NumWrites;
	    WRITE_BYTE_INFO  Write[MAX_NUM_MULTI_BYTE_WRTS-2];
	 } WRITE_MULTI_PLL_INFO, *pWRITE_PLL_INFO;


	 /* This structure is used with the R_MULTI_BYTE_USB_REG */
	 typedef struct
	 {
		unsigned int    SubsystemType; //SUBSYSTEM_TYPE    SubsystemType; // Specifies  the subsystem type

	    // If compiling USB firmware then add filler since Windows compiler
	    // uses 4 byte enums rather than Keil's 1 byte
	 	// Due to the address SubsystemType and 3 fillers,
	 	// we further decrement the write array by 4 bytes in order to bring back
	 	// the size to 64 bytes.
	   /* #ifndef WIN_DRIVER
	    unsigned char                  Filler[3];
	    #endif*/
	    unsigned char				  DevAddr;
	    unsigned char             Address;
	    unsigned char             DataVal;
	 } WRITE_SINGLE_PLL_INFO, *pWRITE_SINGLE_INFO;


	  /******************************************************************************
	  Structures used for misc commands
	  ******************************************************************************/

	 // This structure is used with the W_DAC_THRESHOLD command
	 typedef struct
	 {
	   DWORD_STRUCT  BlockSize;
	 } DAC_FIFO_INFO, *pDAC_FIFO_INFO;


	 // This structure is used with the W_DAC_FIFO_SIZE and W_ADC_FIFO_SIZE command
	 typedef struct
	 {
	   DWORD_STRUCT    FifoSize;
	 } FIFO_SIZE_INFO, *pFIFO_SIZE_INFO;


	 // This structure is used with the W_INT_ON_CHANGE_MASK
	 typedef struct
	 {
	    unsigned char             PortNum;
	    unsigned char             MaskVal;
	 } INT_ON_CHANGE_MASK_INFO, *pINT_ON_CHANGE_MASK_INFO;

	 /* This structure is used with the DATA_ACQ_POWER */
	 typedef struct
	 {
	    unsigned char             PowerStatus;	// "1" = on, "0" = off
	 } WRITE_BOARD_POWER_INFO, *pWRITE_BOARD_POWER_INFO;

	  /******************************************************************************
	  Structure used for accessing the calibration potentiometers
	  ******************************************************************************/

	 typedef struct
	 {
	     unsigned char    ChipNum;
	     unsigned char    PotNum;
	     unsigned char    RegNum;
	     unsigned char    DataVal;
	 } WRITE_CAL_POT_INFO;

	 typedef struct
	 {
	     unsigned char    ChipNum;
	     unsigned char    PotNum;
	     unsigned char    RegNum;
	 } READ_CAL_POT_INFO;


	 /* This structure is used with the W_PMC1737_DAC_REG */
	 typedef struct
	 {
	    unsigned short             Address;
	    unsigned char             DataVal;
	 } WRITE_BYTE_PMC1737_INFO, *pWRITE_BYTE_PMC1737_INFO;


	 /* This structure is used with the R_PMC1737_DAC_REG */
	 typedef struct
	 {
	    unsigned short             Address;
	 } READ_BYTE_PMC1737_INFO, *pREAD_BYTE_PMC1737_INFO;

	/***********************************************************************

	  USB_CMD - This structure defines all of the Commands that can be
	  sent via USB to the firmware.

	  Each command contains at minimum a CmdType.  Most commands also
	  include associated data that are defined in the "d" union within
	  the USB_CMD STRUCTURE.

	  The R_ (READ) commands cause the firmware to send back data via the
	  COMMAND_READ_PIPE.  Refer to DT9837a DriverFirmwareAPI.doc for the
	  details of what data comes back from each command.

	  ************************************************************************/

	typedef struct
	{
	   unsigned int 				CmdCode;   //USB_FIRMWARE_CMD_CODE        CmdCode;

	   // If compiling USB firmware then add filler since Windows compiler
	   // uses 4 byte enums rather than Keil's 2 byte. See note at the begining
	   // of this file

	   union
	   {
	      WRITE_BYTE_INFO           WriteByteInfo;
	      READ_BYTE_INFO            ReadByteInfo;
	      WRITE_MULTI_INFO          WriteMultiInfo;
	      READ_MULTI_INFO           ReadMultiInfo;
	      RMW_BYTE_INFO             RMWByteInfo;
	      RMW_MULTI_INFO            RMWMultiInfo;

	      WRITE_I2C_BYTE_INFO       WriteI2CByteInfo;
	      READ_I2C_BYTE_INFO        ReadI2CByteInfo;
	      WRITE_I2C_MULTI_INFO      WriteI2CMultiInfo;
	      READ_I2C_MULTI_INFO       ReadI2CMultiInfo;

	      SUBSYSTEM_INFO            SubsystemInfo;
	      READ_SINGLE_VALUE_INFO    ReadSingleValueInfo;
	      READ_SINGLE_VALUES_INFO   ReadSingleValuesInfo;
	      WRITE_SINGLE_VALUE_INFO   WriteSingleValueInfo;

	      DAC_FIFO_INFO        	DacFifoInfo;
	      INT_ON_CHANGE_MASK_INFO   IntOnChangeMaskInfo;
		  FIFO_SIZE_INFO			FifoSizeInfo;

		  WRITE_CAL_POT_INFO        WriteCalPotInfo;
	      READ_CAL_POT_INFO         ReadCalPotInfo;

		  WRITE_WORD_INFO           WriteWordInfo;
	      READ_WORD_INFO            ReadWordInfo;
	      WRITE_MULTI_WORD_INFO     WriteMultiWordInfo;
	      READ_MULTI_WORD_INFO      ReadMultiWordInfo;
	      RMW_WORD_INFO             RMWWordInfo;
	      RMW_MULTI_WORD_INFO       RMWMultiWordInfo;

		  WRITE_BYTE_PMC1737_INFO	WritePMC1737Info;
		  READ_BYTE_PMC1737_INFO	ReadPMC1737Info;
		  WRITE_MULTI_PLL_INFO		WriteMultiPllInfo;
		  WRITE_BOARD_POWER_INFO	WriteBoardPowerInfo;
		  WRITE_SINGLE_PLL_INFO		WriteSinglePllInfo;


		} d;

	} USB_CMD, *PUSB_CMD;

	/******************************************************************

	  USB_FIRMWARE_MSG_CODE - defines all of the messages that the
	  DT9837a firmware can asynchronously send to the driver via USB,

	  *****************************************************************/
	typedef enum
	{
	    CTR_OVERFLOW_MSG = 0x01,	// Fill out CtrOverflowInfo
	    DIN_CHANGED_MSG,			// Fill out DinChangedInfo
	    DAC_THRESHOLD_REACHED_MSG,
	    ADC_OVER_SAMPLE_MSG,
		ADC_OVERRUN_MSG,
		OUTPUT_FIFO_UNDERFLOW_MSG,
	    DAC_OVER_SAMPLE_MSG,
		OUTPUT_DONE_MSG,
	    MAX_USB_FIRMWARE_MSG_CODE   //Valid USB_FIRMWARE_MSG_CODE's will be less than
	                                //this number
	} USB_FIRMWARE_MSG_CODE;


	// This structure is used with the INT_STATUS_MSG
	typedef struct
	{
	   unsigned char             CtrNum;
	} CTR_OVERFLOW_INFO, *pCTR_OVERFLOW_INFO;


	// This structure is used with the INT_ON_CHANGE_MSG
	typedef struct
	{
		unsigned char             CurVal;
	} DIN_CHANGED_INFO, *pDIN_CHANGED_INFO;

	/***********************************************************************

	  USB_MSG - This structure defines all of the messages along with
	  their associated data that can be sent from the firmware to the driver.

	  Each message contains at minimum a MsgType followed by associated data.



	  ************************************************************************/

	typedef struct
	{
	   unsigned int MsgType; // SB: actual type is USB_FIRMWARE_MSG_CODE MsgType;

	   // If compiling USB firmware then add filler since Windows compiler
	   // uses 4 byte enums rather than Keil's 1 byte
	   /*#ifndef WIN_DRIVER
	      BYTE                  Filler[3];
	   #endif*/

	   union
	   {
	      CTR_OVERFLOW_INFO     CtrOverflowInfo;
	      DIN_CHANGED_INFO      DinChangedInfo;
	   } d;
	} USB_MSG, *pUSB_MSG;


#pragma pack()

	// SB: from #defines
	enum
	{
		DAC_DEV_ADR = 0x4C,
		EEPROM_DEV_ADR = 0x50
	};

	// from Dt9837aHwRegs.h

	// The values stored in the coupling locations are the OL Coupling Type AC / DC
	enum
	{
		EEPROM_OFFSET_COUPLING_0 = 0x0020,
		EEPROM_OFFSET_COUPLING_1 = 0x0021,
		EEPROM_OFFSET_COUPLING_2 = 0x0022,
		EEPROM_OFFSET_COUPLING_3 = 0x0023
	};

	// Type Disabled / Internal / External
	enum
	{
		EEPROM_OFFSET_CURRENT_SOURCE_0 =	0x0024,
		EEPROM_OFFSET_CURRENT_SOURCE_1 =	0x0025,
		EEPROM_OFFSET_CURRENT_SOURCE_2 =	0x0026,
		EEPROM_OFFSET_CURRENT_SOURCE_3 =	0x0027,

		EEPROM_OFFSET_POWER_OVERRIDE_REG =	0x0040
	};

	// Operation Control Registers ( resides in external memory)
	enum
	{
		GENERAL_CNTRL_REG0 = 	0x0,
		GENERAL_CNTRL_REG1 = 	0x1,
		GENERAL_CNTRL_REG2 = 	0x2,
		GENERAL_CNTRL_REG3 = 	0x4,
		GENERAL_CNTRL_REG4 = 	0x8,
		GENERAL_CNTRL_REG5 = 	0x10,
		GENERAL_CNTRL_REG6 = 	0x20
	};

	enum { PLL_DEV_ADDR = 0x69};
	enum { INPUT_FIFO_FLAG_MASK = 0x82 };

// from PLLDefs.h
	// CY22150F slave & register addresses
	enum
	{
		CY22150F_SLAVE_ADDR	= 0x69,
		CLKOE_REG_ADDR = 0x09,
		DIV1SRC_REG_ADDR = 0x0C,
		INPUT_CRYS_OSC_CTRL_ADDR = 0x12,
		INPUT_LOAD_CAP_REG_ADDR = 0x13,
		CHARGE_PUMP_REG_ADDR = 0x40,
		PB_COUNTER_REG_ADDR	= 0x41,
		PO_Q_COUNTER_REG_ADDR = 0x42,
		CROSSPOINT_SWITCH_MATRIX_CNTRL_0_REG_ADDR = 0x44,
		CROSSPOINT_SWITCH_MATRIX_CNTRL_1_REG_ADDR = 0x45,
		CROSSPOINT_SWITCH_MATRIX_CNTRL_2_REG_ADDR = 0x46,
		DIV2SRC_REG_ADDR = 0x47
	};


	typedef struct
	{
		unsigned char  ClkOe;
		unsigned char  Div1Src;
		unsigned char  InputCrysOscCtrl;
		unsigned char  InputLoadCap;
		unsigned char  ChargePump;
		unsigned char  PBCounter;
		unsigned char  POQCounter;
		unsigned char  CrossPointSw0;
		unsigned char  CrossPointSw1;
		unsigned char  CrossPointSw2;
		unsigned char  Div2Src;
	} CY22150REGISTERS;

	enum
	{
		FIFO_SIZE_IN_SAMPLES  = 2048,

		/* The maximum and minimum number of samples to read on A FIFO not empty Interrupt*/
		MAX_SAMPLES_PER_FIFO_NOT_EMPTY_INT = 1024, /* max number of samples to read from A/D FIFO */
		MIN_SAMPLES_PER_FIFO_NOT_EMPTY_INT = 2	   /* min number of samples to read from A/D FIFO */

	};

	/* **** General Control Register 6 */

	enum
	{
		ADC_OVERRUN_EN	 = 1 << 0,
		DAC_UNDERRUN_EN	 = 1 << 1,
		ADC_OVERRUN_ERR = 1 << 2,
		DAC_UNDERRUN_ERR = 1 << 3
	};

	static const int GRP_DELAY_SIZE_IN_SAMPLES	= 39;			// Number of samples the A/D lags behind tach data

	static const unsigned char SUBSYS_FLG_POS_EXTTRIG			= 1 << 0;	// External positive edge Trigger
	static const unsigned char SUBSYS_FLG_POS_THRESHOLDTRIG		= 1 << 1;	// Threshold positive edge Trigger
	static const unsigned char SUBSYS_FLG_NEG_EXTTRIG			= 1 << 2;	// External negative Trigger
	static const unsigned char SUBSYS_FLG_NEG_THRESHOLDTRIG		= 1 << 3;	// Threshold negative Trigger

	// Threshold related registers used by DT9837C only

	static const unsigned short THRSHOLD_CNTRL_REG = 0x80;	// only the first 4-bit are used for channel selection
	static const unsigned short THRSHOLD_CNTRL_REG_MASK = 0x000F;
	static const unsigned short THRSHOLD_TRIGLEVEL_REG = 0x81;  // this will hold a 16-bit raw voltage data

};

} /* namespace ul */

#endif /* USB_USB9837X_H_ */
