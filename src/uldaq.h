/*
 * uldaq.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef UL_DAQ_H_
#define UL_DAQ_H_

#ifdef __cplusplus
extern "C"
{
#endif


/** A bitmask defining the physical connection interface used to communicate with a DAQ device.
 * Used with ulGetDaqDeviceInventory() as an \p interfaceTypes argument value. */
typedef enum
{
	/** USB interface */
	USB_IFC			= 1 << 0,

	/** Bluetooth interface */
	BLUETOOTH_IFC	= 1 << 1,

	/** Ethernet interface */
	ETHERNET_IFC	= 1 << 2,

	/** Any interface */
	ANY_IFC = USB_IFC | BLUETOOTH_IFC | ETHERNET_IFC

}DaqDeviceInterface;

/** \brief A structure that defines a particular DAQ device, usually obtained using ulGetDaqDeviceInventory().
 *
 * The struct contains fields with the product name, ID, and interface.
 */
struct DaqDeviceDescriptor
{
	/** The generic (unqualified) product name of the device referenced by the DaqDeviceDescriptor */
	char productName[64];

	/**The numeric string indicating the product type referenced by the DaqDeviceDescriptor.*/
	unsigned int productId;

	/** The enumeration indicating the type of interface in use by the device referenced by the DaqDeviceDescriptor. */
	DaqDeviceInterface devInterface;

	/** Similar to \p productname, but may contain additional information. */
	char devString[64];

	/** A string that uniquely identifies a specific device, usually with a serial number or MAC address. */
	char uniqueId[64];

	/** Reserved for future use */
	char reserved[512];
};

/** \brief A structure that defines a particular DAQ device, usually obtained using ulGetDaqDeviceInventory(). */
typedef struct 	DaqDeviceDescriptor DaqDeviceDescriptor;

/**
 * The DAQ device
 */
typedef long long DaqDeviceHandle;


/** \brief A structure containing information about the progress of the specified scan operation.
 *
 * The struct contains fields for the current and total scan count and index value.
 */
struct TransferStatus
{
	/** The number of samples per channel transferred since the scan started. This is the same as \p currentTotalCount for single channel scans. */
	unsigned long long currentScanCount;

	/** The total number of samples transferred since the scan started. This is the same as \p currentScanCount
	 * multiplied by the number of channels in the scan. */
	unsigned long long currentTotalCount;

	/** This marks the location in the buffer where the last scan of data values are stored.
	 * For continuous scans, this value increments up to (buffer size - number of channels) and restarts from 0. */
	long long currentIndex;

	/** Reserved for future use */
	char reserved[64];
};

/** \brief A structure containing information about the progress of the specified scan operation. */
typedef struct 	TransferStatus TransferStatus;

#define ERR_MSG_LEN				512

/** UL error codes */
typedef enum
{
	/** No error has occurred */
	ERR_NO_ERROR 					= 0,

	/** Unhandled internal exception */
	ERR_UNHANDLED_EXCEPTION 		= 1,

	/** Invalid device handle */
	ERR_BAD_DEV_HANDLE 				= 2,

	/** This function cannot be used with this device */
	ERR_BAD_DEV_TYPE 				= 3,

	/** Insufficient permission to access this device */
	ERR_USB_DEV_NO_PERMISSION		= 4,

	/** USB interface is already claimed */
	ERR_USB_INTERFACE_CLAIMED 		= 5,

	/** Device not found */
	ERR_DEV_NOT_FOUND 				= 6,

	/** Device not connected or connection lost */
	ERR_DEV_NOT_CONNECTED 			= 7,

	/** Device no longer responding */
	ERR_DEAD_DEV 					= 8,

	/** Buffer too small for operation */
	ERR_BAD_BUFFER_SIZE 			= 9,

	/** Invalid buffer */
	ERR_BAD_BUFFER 					= 10,

	/** Invalid memory type */
	ERR_BAD_MEM_TYPE 				= 11,

	/** Invalid memory region */
	ERR_BAD_MEM_REGION 				= 12,

	/** Invalid range */
	ERR_BAD_RANGE					= 13,

	/** Invalid analog input channel specified */
	ERR_BAD_AI_CHAN					= 14,

	/** Invalid input mode specified */
	ERR_BAD_INPUT_MODE				= 15,

	/** A background process is already in progress */
	ERR_ALREADY_ACTIVE				= 16,

	/** Invalid trigger type specified */
	ERR_BAD_TRIG_TYPE				= 17,

	/** FIFO overrun, data was not transferred from device fast enough */
	ERR_OVERRUN						= 18,

	/** FIFO underrun, data was not transferred to device fast enough */
	ERR_UNDERRUN					= 19,

	/** Operation timed out */
	ERR_TIMEDOUT					= 20,

	/** Invalid option specified */
	ERR_BAD_OPTION					= 21,

	/** Invalid sampling rate specified */
	ERR_BAD_RATE					= 22,

	/** Sample count cannot be greater than FIFO size for BURSTIO scans */
	ERR_BAD_BURSTIO_COUNT			= 23,

	/** Configuration not supported */
	ERR_CONFIG_NOT_SUPPORTED		= 24,

	/** Invalid configuration value */
	ERR_BAD_CONFIG_VAL				= 25,

	/** Invalid analog input channel type specified */
	ERR_BAD_AI_CHAN_TYPE			= 26,

	/** ADC overrun occurred */
	ERR_ADC_OVERRUN					= 27,

	/** Invalid thermocouple type specified */
	ERR_BAD_TC_TYPE					= 28,

	/** Invalid unit specified */
	ERR_BAD_UNIT					= 29,

	/** Invalid queue size */
	ERR_BAD_QUEUE_SIZE				= 30,

	/** Invalid config item specified */
	ERR_BAD_CONFIG_ITEM				= 31,

	/** Invalid info item specified */
	ERR_BAD_INFO_ITEM				= 32,

	/** Invalid flag specified */
	ERR_BAD_FLAG					= 33,

	/** Invalid sample count specified */
	ERR_BAD_SAMPLE_COUNT			= 34,

	/** Internal error */
	ERR_INTERNAL					= 35,

	/** Invalid coupling mode */
	ERR_BAD_COUPLING_MODE			= 36,

	/** Invalid sensor sensitivity */
	ERR_BAD_SENSOR_SENSITIVITY		= 37,

	/** Invalid IEPE mode */
	ERR_BAD_IEPE_MODE				= 38,

	/** Invalid channel queue specified */
	ERR_BAD_AI_CHAN_QUEUE			= 39,

	/** Invalid gain queue specified */
	ERR_BAD_AI_GAIN_QUEUE			= 40,

	/** Invalid mode queue specified */
	ERR_BAD_AI_MODE_QUEUE			= 41,

	/** FPGA file not found */
	ERR_FPGA_FILE_NOT_FOUND			= 42,

	/** Unable to read FPGA file */
	ERR_UNABLE_TO_READ_FPGA_FILE	= 43,

	/** FPGA not loaded */
	ERR_NO_FPGA						= 44,

	/** Invalid argument */
	ERR_BAD_ARG						= 45,

	/** Minimum slope value reached */
	ERR_MIN_SLOPE_VAL_REACHED		= 46,

	/** Maximum slope value reached */
	ERR_MAX_SLOPE_VAL_REACHED		= 47,

	/** Minimum offset value reached */
	ERR_MIN_OFFSET_VAL_REACHED		= 48,

	/** Maximum offset value reached */
	ERR_MAX_OFFSET_VAL_REACHED		= 49,

	/** Invalid port type specified */
	ERR_BAD_PORT_TYPE				= 50,

	/** Digital I/O is configured incorrectly */
	ERR_WRONG_DIG_CONFIG			= 51,

	/** Invalid bit number */
	ERR_BAD_BIT_NUM					= 52,

	/** Invalid port value specified */
	ERR_BAD_PORT_VAL				= 53,

	/** Invalid re-trigger count */
	ERR_BAD_RETRIG_COUNT			= 54,

	/** Invalid analog output channel specified */
	ERR_BAD_AO_CHAN					= 55,

	/** Invalid D/A output value specified */
	ERR_BAD_DA_VAL					= 56,

	/** Invalid timer specified */
	ERR_BAD_TMR						= 57,

	/** Invalid frequency specified */
	ERR_BAD_FREQUENCY				= 58,

	/** Invalid duty cycle specified */
	ERR_BAD_DUTY_CYCLE				= 59,

	/** Invalid initial delay specified */
	ERR_BAD_INITIAL_DELAY			= 60,

	/** Invalid counter specified */
	ERR_BAD_CTR						= 61,

	/** Invalid counter value specified */
	ERR_BAD_CTR_VAL					= 62,

	/** Invalid DAQ input channel type specified */
	ERR_BAD_DAQI_CHAN_TYPE			= 63,

	/** Invalid number of channels specified */
	ERR_BAD_NUM_CHANS				= 64,

	/** Invalid counter register specified */
	ERR_BAD_CTR_REG					= 65,

	/** Invalid counter measurement type specified */
	ERR_BAD_CTR_MEASURE_TYPE		= 66,

	/** Invalid counter measurement mode specified */
	ERR_BAD_CTR_MEASURE_MODE		= 67,

	/** Invalid debounce time specified */
	ERR_BAD_DEBOUNCE_TIME			= 68,

	/** Invalid debounce mode specified */
	ERR_BAD_DEBOUNCE_MODE			= 69,

	/** Invalid edge detection mode specified */
	ERR_BAD_EDGE_DETECTION			= 70,

	/** Invalid tick size specified */
	ERR_BAD_TICK_SIZE				= 71,

	/** Invalid DAQ output channel type specified */
	ERR_BAD_DAQO_CHAN_TYPE			= 72,

	/** No connection established */
	ERR_NO_CONNECTION_ESTABLISHED	= 73,

	/** Invalid event type specified */
	ERR_BAD_EVENT_TYPE				= 74,

	/** An event handler has already been enabled for this event type */
	ERR_EVENT_ALREADY_ENABLED		= 75,

	/** Invalid event parameter specified */
	ERR_BAD_EVENT_PARAMETER			= 76,

	/** Invalid callback function specified */
	ERR_BAD_CALLBACK_FUCNTION		= 77,

	/** Invalid memory address */
	ERR_BAD_MEM_ADDRESS				= 78,

	/** Memory access denied */
	ERR_MEM_ACCESS_DENIED			= 79,

	/** Device is not available at time of request */
	ERR_DEV_UNAVAILABLE				= 80,

	/** Re-trigger option is not supported for the specified trigger type */
	ERR_BAD_RETRIG_TRIG_TYPE		= 81,

	/** This function cannot be used with this version of the device */
	ERR_BAD_DEV_VER 				= 82,

	/** This digital operation is not supported on the specified port */
	ERR_BAD_DIG_OPERATION			= 83,

	/** Invalid digital port index specified */
	ERR_BAD_PORT_INDEX				= 84,

	/** Temperature input has open connection */
	ERR_OPEN_CONNECTION				= 85,

	/** Device is not ready to send data */
	ERR_DEV_NOT_READY				= 86,

	/** Pacer overrun, external clock rate too fast. */
	ERR_PACER_OVERRUN				= 87,

	/** Invalid trigger channel specified */
	ERR_BAD_TRIG_CHANNEL			= 88,

	/** Invalid trigger level specified */
	ERR_BAD_TRIG_LEVEL				= 89,

	/** Invalid channel order */
	ERR_BAD_CHAN_ORDER				= 90,

	/** Temperature input is out of range */
	ERR_TEMP_OUT_OF_RANGE			= 91,

	/** Trigger threshold is out of range */
	ERR_TRIG_THRESHOLD_OUT_OF_RANGE	= 92,

	/** Incompatible firmware version, firmware update required */
	ERR_INCOMPATIBLE_FIRMWARE 		= 93,

	/** Specified network interface is not available or disconnected */
	ERR_BAD_NET_IFC 				= 94,

	/** Invalid host specified */
	ERR_BAD_NET_HOST 				= 95,

	/** Invalid port specified */
	ERR_BAD_NET_PORT 				= 96,

	/** Network interface used to obtain the device descriptor not available or disconnected */
	ERR_NET_IFC_UNAVAILABLE			= 97,

	/** Network connection failed */
	ERR_NET_CONNECTION_FAILED		= 98,

	/** Invalid connection code */
	ERR_BAD_CONNECTION_CODE			= 99,

	/** Connection code ignored */
	ERR_CONNECTION_CODE_IGNORED		= 100,

	/** Network device already in use */
	ERR_NET_DEV_IN_USE				= 101,

	/** Invalid network frame  */
	ERR_BAD_NET_FRAME				= 102,

	/** Network device did not respond within expected time */
	ERR_NET_TIMEOUT					= 103,

	/** Data socket connection failed */
	ERR_DATA_SOCKET_CONNECTION_FAILED = 104,

	/** One or more bits on the specified port are used for alarm */
	ERR_PORT_USED_FOR_ALARM 		= 105,

	/** The specified bit is used for alarm */
	ERR_BIT_USED_FOR_ALARM 			= 106,

	/** Common-mode voltage range exceeded */
	ERR_CMR_EXCEEDED 				= 107,

	/** Network buffer overrun, data was not transferred from buffer fast enough */
	ERR_NET_BUFFER_OVERRUN 			= 108,

	/** Invalid network buffer */
	ERR_BAD_NET_BUFFER 				= 109
} UlError;

/** A/D channel input modes */
typedef enum
{
	/** Differential */
	AI_DIFFERENTIAL = 1,

	/** Single-ended */
	AI_SINGLE_ENDED = 2,

	/** Pseudo-differential */
	AI_PSEUDO_DIFFERENTIAL = 3
}AiInputMode;

/** Analog input channel types
 *
 * Bitmask indicating all supported channel types. Returned to the \p infoValue argument by ulAIGetInfo() using
 * AiInfoItem #AI_INFO_CHAN_TYPES.
 *
*/
typedef enum
{
	/** Voltage */
	AI_VOLTAGE 			= 1 << 0,

	/** Thermocouple */
	AI_TC 				= 1 << 1,

	/** Resistance Temperature Detector (RTD) */
	AI_RTD 				= 1 << 2,

	/** Thermistor */
	AI_THERMISTOR 		= 1 << 3,

	/** Semiconductor */
	AI_SEMICONDUCTOR 	= 1 << 4,

	/** Disabled */
	AI_DISABLED 		= 1 << 30
}AiChanType;

/** Thermocouple types */
typedef enum
{
	/** Type J */
	TC_J				= 1,

	/** Type K */
	TC_K				= 2,

	/** Type T */
	TC_T				= 3,

	/** Type E */
	TC_E				= 4,

	/** Type R */
	TC_R				= 5,

	/** Type S */
	TC_S				= 6,

	/** Type B */
	TC_B				= 7,

	/** Type N */
	TC_N				= 8
}TcType;

/** Sensor connection types */
typedef enum
{
	/** 2-wire with a single sensor per differential channel pair **/
	SCT_2_WIRE_1	= 1,

	/** 2-wire with two sensors per differential channel pair **/
	SCT_2_WIRE_2	= 2,

	/** 3-wire with a single sensor per differential channel pair **/
	SCT_3_WIRE		= 3,

	/** 4-wire with a single sensor per differential channel pair **/
	SCT_4_WIRE		= 4
}SensorConnectionType;

/** Used with many analog input and output functions, as well as a return value for the \p infoValue argument
 * to ulAIGetInfo() when used with ::AI_INFO_DIFF_RANGE or ::AI_INFO_SE_RANGE, <br> and the \p infoValue argument
 * to ulAOGetInfo() when used with ::AO_INFO_RANGE. */
typedef enum
{
	/** -60 to +60 Volts */
	BIP60VOLTS		= 1,

	/** -30 to +30 Volts */
	BIP30VOLTS		= 2,

	/** -15 to +15 Volts */
	BIP15VOLTS		= 3,

	/** -20 to +20 Volts */
	BIP20VOLTS      = 4,

	/** -10 to +10 Volts */
	BIP10VOLTS      = 5,

	/** -5 to +5 Volts */
	BIP5VOLTS       = 6,

	/** -4 to +4 Volts */
	BIP4VOLTS       = 7,

	/** -2.5 to +2.5 Volts */
	BIP2PT5VOLTS    = 8,

	/** -2.0 to +2.0 Volts */
	BIP2VOLTS       = 9,

	/** -1.25 to +1.25 Volts */
	BIP1PT25VOLTS   = 10,

	/** -1 to +1 Volts */
	BIP1VOLTS       = 11,

	/** -.625 to +.625 Volts */
	BIPPT625VOLTS   = 12,

	/** -.5 to +.5 Volts */
	BIPPT5VOLTS     = 13,

	/** -0.25 to +0.25 Volts */
	BIPPT25VOLTS    = 14,

	/** -0.125 to +0.125 Volts */
	BIPPT125VOLTS   = 15,

	/** -0.2 to +0.2 Volts */
	BIPPT2VOLTS     = 16,

	/** -.1 to +.1 Volts */
	BIPPT1VOLTS     = 17,

	/** -0.078 to +0.078 Volts */
	BIPPT078VOLTS   = 18,

	/** -.05 to +.05 Volts */
	BIPPT05VOLTS    = 19,

	/** -.01 to +.01 Volts */
	BIPPT01VOLTS    = 20,

	/** -.005 to +.005 Volts */
	BIPPT005VOLTS   = 21,

	/** -3.0 to +3.0 Volts */
	BIP3VOLTS       = 22,

	/** -.312 to +.312 Volts */
	BIPPT312VOLTS   = 23,

	/** -.156 to +.156 Volts */
	BIPPT156VOLTS   = 24,




	/** 0 to +60 Volts */
	UNI60VOLTS		= 1001,

	/** 0 to +30 Volts */
	UNI30VOLTS		= 1002,

	/** 0 to +15 Volts */
	UNI15VOLTS		= 1003,

	/** 0 to +20 Volts */
	UNI20VOLTS      = 1004,

	/** 0 to +10 Volts */
	UNI10VOLTS      = 1005,

	/** 0 to +5 Volts */
	UNI5VOLTS       = 1006,

	/** 0 to +4 Volts */
	UNI4VOLTS       = 1007,

	/** 0 to +2.5 Volts */
	UNI2PT5VOLTS    = 1008,

	/** 0 to +2.0 Volts */
	UNI2VOLTS       = 1009,

	/** 0 to +1.25 Volts */
	UNI1PT25VOLTS   = 1010,

	/** 0 to +1 Volts */
	UNI1VOLTS       = 1011,

	/** 0 to +.625 Volts */
	UNIPT625VOLTS   = 1012,

	/** 0 to +.5 Volts */
	UNIPT5VOLTS     = 1013,

	/** 0 to +0.25 Volts */
	UNIPT25VOLTS    = 1014,

	/** 0 to +0.125 Volts */
	UNIPT125VOLTS   = 1015,

	/** 0 to +0.2 Volts */
	UNIPT2VOLTS     = 1016,

	/** 0 to +.1 Volts */
	UNIPT1VOLTS     = 1017,

	/** 0 to +0.078 Volts */
	UNIPT078VOLTS   = 1018,

	/** 0 to +.05 Volts */
	UNIPT05VOLTS    = 1019,

	/** 0 to +.01 Volts */
	UNIPT01VOLTS    = 1020,

	/** 0 to +.005 Volts */
	UNIPT005VOLTS   = 1021,

	/** 0 to 20 Milliamps */
	MA0TO20 = 2000
}Range;

/** Temperature units */
typedef enum
{
	/** Celcius */
	TU_CELSIUS  	= 1,

	/** Fahrenheit */
	TU_FAHRENHEIT 	= 2,

	/** Kelvin */
	TU_KELVIN 		= 3
}TempUnit;

/** Temperature units */
typedef enum
{
	/** Celcius */
	TS_CELSIUS  	= TU_CELSIUS,

	/** Fahrenheit */
	TS_FAHRENHEIT 	= TU_FAHRENHEIT,

	/** Kelvin */
	TS_KELVIN 		= TU_KELVIN,

	/** Volts */
	TS_VOLTS 		= 4,

	/** No scale (Raw) */
	TS_NOSCALE 		= 5
}TempScale;

#ifndef doxy_skip
/** Auto zero modes */
typedef enum
{
	/** Disabled */
	AZM_NONE = 1,

	/** Perform auto zero on every thermocouple reading. */
	AZM_EVERY_SAMPLE = 2,

	/** Perform auto zero before every scan. */
	AZM_ONCE = 3
}AutoZeroMode;
#endif /* doxy_skip */

#ifndef doxy_skip
/** ADC timing modes */
typedef enum
{
	/** The timing mode is set automatically. */
	ADC_TM_AUTO 		= 1,

	/** Acquires data in samples per 1000 seconds per channel. */
	ADC_TM_HIGH_RES 	= 2,

	/** High speed timing mode. */
	ADC_TM_HIGH_SPEED 	= 3
}AdcTimingMode;
#endif /* doxy_skip */

/** IEPE modes */
typedef enum
{
	/** IEPE excitation current is disabled. */
	IEPE_DISABLED = 1,

	/** IEPE excitation current is enabled. */
	IEPE_ENABLED = 2
}IepeMode;

/** Coupling modes */
typedef enum
{
	/** DC coupling */
	CM_DC = 1,

	/** AC coupling */
	CM_AC = 2
}CouplingMode;

/** Open Thermocouple detection modes */
typedef enum
{
	/** Open Thermocouple detection modes is disabled. */
	OTD_DISABLED = 1,

	/** Open Thermocouple detection modes is enabled. */
	OTD_ENABLED = 2
}OtdMode;

/** Bitmask indicating supported queue types. Returned to the \p infoValue argument by ulAIGetInfo() using #AiInfoItem ::AI_INFO_QUEUE_TYPES. */
typedef enum
{
	/** The AI subsystem supports a channel queue. */
	CHAN_QUEUE = 1 << 0,

	/** The AI subsystem supports a gain queue. */
	GAIN_QUEUE = 1 << 1,

	/** The AI subsystem supports a mode queue. */
	MODE_QUEUE = 1 << 2
}AiQueueType;

/** Device queue limitations
 *
 * Bitmask indicating all queue limitations. Returned to the \p infoValue argument by ulAIGetInfo() using
 * AiInfoItem #AI_INFO_QUEUE_LIMITS. \n
 * See also #AI_INFO_QUEUE_TYPES and #AI_INFO_MAX_QUEUE_LENGTH_BY_MODE to determine queue capabilities.
 */
typedef enum
{
	/** A particular channel number cannot appear more than once in the queue. */
	UNIQUE_CHAN = 1 << 0,

	/** Channel numbers must be listed in ascending order within the queue. */
	ASCENDING_CHAN = 1 << 1,

	/** Channel numbers must be listed in contiguous order within the queue. */
	CONSECUTIVE_CHAN = 1 << 2
} AiChanQueueLimitation;

/** Analog input calibration table types */
typedef enum
{
	/** Factory calibration table */
	AI_CTT_FACTORY		= 1,

	/** Field calibration table */
	AI_CTT_FIELD		= 2
}AiCalTableType;

/** Analog input rejection frequency types */
typedef enum
{
	/** 60 Hz rejection frequency */
	AI_RFT_60HZ		= 1,

	/** 50 Hz rejection frequency */
	AI_RFT_50HZ		= 2
}AiRejectFreqType;


/** Used with all digital I/O functions and with ulDIOGetInfo() as the \p infoValue argument value when used with ::DIO_INFO_PORT_IO_TYPE. */
typedef enum
{
	/** AuxPort */
	AUXPORT = 1,

	/** AuxPort0 */
	AUXPORT0 = 1,

	/** AuxPort1 */
	AUXPORT1 = 2,

	/** AuxPort2 */
	AUXPORT2 = 3,

	/** FirstPortA */
	FIRSTPORTA = 10,

	/** FirstPortB */
	FIRSTPORTB = 11,

	/** FirstPortC */
	FIRSTPORTC = 12,

	/** FirstPortC Low */
	FIRSTPORTCL = 12,

	/** FirstPortC High */
	FIRSTPORTCH = 13,

	/** SecondPortA */
	SECONDPORTA = 14,

	/** SecondPortB */
	SECONDPORTB = 15,

	/** SecondPortC Low */
	SECONDPORTCL = 16,

	/** SecondPortC High */
	SECONDPORTCH = 17,

	/** ThirdPortA */
	THIRDPORTA = 18,

	/** ThirdPortB */
	THIRDPORTB = 19,

	/** ThirdPortC Low */
	THIRDPORTCL = 20,

	/** ThirdPortC High */
	THIRDPORTCH = 21,

	/** FourthPortA */
	FOURTHPORTA = 22,

	/** FourthPortB */
	FOURTHPORTB = 23,

	/** FourthPortC Low */
	FOURTHPORTCL = 24,

	/** FourthPortC High */
	FOURTHPORTCH = 25,

	/** FifthPortA */
	FIFTHPORTA = 26,

	/** FifthPortB */
	FIFTHPORTB = 27,

	/** FifthPortC Low */
	FIFTHPORTCL = 28,

	/** FifthPortC High */
	FIFTHPORTCH = 29,

	/** SixthPortA */
	SIXTHPORTA = 30,

	/** SixthPortB */
	SIXTHPORTB = 31,

	/** SixthPortC Low */
	SIXTHPORTCL = 32,

	/** SixthPortC High */
	SIXTHPORTCH = 33,

	/** SeventhPortA */
	SEVENTHPORTA = 34,

	/** SeventhPortB */
	SEVENTHPORTB = 35,

	/** SeventhPortC Low */
	SEVENTHPORTCL = 36,

	/** SeventhPortC High */
	SEVENTHPORTCH = 37,

	/** EighthPortA */
	EIGHTHPORTA = 38,

	/** EighthPortB */
	EIGHTHPORTB = 39,

	/** EighthPortC Low */
	EIGHTHPORTCL = 40,

	/** EighthPortC High */
	EIGHTHPORTCH = 41
}DigitalPortType;

/** Used with ulDIOGetInfo() as the \p infoValue argument value when used with ::DIO_INFO_PORT_IO_TYPE. */
typedef enum
{
	/** Fixed input port */
	DPIOT_IN = 1,

	/** Fixed output port */
	DPIOT_OUT = 2,

	/** Bidirectional (input or output) port */
	DPIOT_IO = 3,

	/** Bitwise configurable */
	DPIOT_BITIO = 4,

	/** Bidirectional (input or output) port; configuration is not required. */
	DPIOT_NONCONFIG = 5
} DigitalPortIoType;

/** Used with ulDConfigPort() and ulDConfigBit() as the \p direction argument value. */
typedef enum
{
	/** Input */
	DD_INPUT = 1,

	/** Output */
	DD_OUTPUT = 2
}DigitalDirection;

/** Types of timer channels */
typedef enum
{
	/** Programmable frequency timer */
	TMR_STANDARD = 1,

	/** Programmable frequency timer, plus other attributes such as pulse width. */
	TMR_ADVANCED = 2
}TimerType;

/** Timer idle state */
typedef enum
{
	/** Idle low */
	TMRIS_LOW = 1,

	/** Idle high */
	TMRIS_HIGH = 2
}TmrIdleState;

/** Used with ulTmrPulseOutStatus() as the \p status argument value returned (if supported) for the specified device. */
typedef enum
{
	/** The timer is currently idle. */
	TMRS_IDLE = 0,

	/** The timer is currently running. */
	TMRS_RUNNING = 1
}TmrStatus;

/** Used as an individual value with the subsystem SetTrigger functions as the \p type argument value,
 *  or as a bitmask value with the subsystem GetInfo functions as the \p infoValue argument value.
 *  The trigger input is specified by the \p trigChan argument, or the \p channel field of the DaqInChanDescriptor struct,
 *   when using the subsystem SetTrigger functions.
 *
 */
typedef enum
{
	/** No trigger type. Valid for subsystem GetInfo functions; not a valid value for subsystem SetTrigger functions. */
	TRIG_NONE = 0,	
	
	/** A digital trigger. The trigger condition is met when the trigger input transitions from a logic low level to a
	 * logic high level. This is the default condition used when triggering is enabled. All others require configuration
	 * using the subsystem SetTrigger functions. */
	TRIG_POS_EDGE =  1 << 0,

	/** A digital trigger. The trigger condition is met when the trigger input transitions from a logic high level
	 * to a logic low level.*/
	TRIG_NEG_EDGE = 1 << 1,

	/** A digital trigger. The trigger condition is met when the trigger input is at a logic high level. */
	TRIG_HIGH = 1 << 2,

	/** A digital trigger. The trigger condition is met when the trigger input is at a logic low level. */
	TRIG_LOW = 1 << 3,

	/** A digital gate. The operation is enabled only when the trigger input is at a logic high level. */
	GATE_HIGH = 1 << 4,

	/** A digital gate. The operation is enabled only when the trigger input is at a logic low level. */
	GATE_LOW = 1 << 5,

	/** An analog trigger. The trigger condition is met when the trigger input transitions from
	 * below the threshold specified by (the \p level argument value minus the \p variance argument value) to
	 * above the threshold specified by the \p level argument value. */
	TRIG_RISING = 1 << 6,

	/** An analog trigger. The trigger condition is met when the trigger input transitions from
	 * above the threshold specified by (the \p level argument value plus the \p variance argument value) to
	 * below the threshold specified by the \p level argument value. */
	TRIG_FALLING = 1 << 7,

	/** An analog trigger. The trigger condition is met when the trigger input is above the threshold
	 * specified by the \p level argument value. */
	TRIG_ABOVE = 1 << 8,

	/** An analog trigger. The trigger condition is met when the trigger input is below the threshold
	 * specified by the \p level argument value. */
	TRIG_BELOW = 1 << 9,

	/** An analog trigger. The operation is enabled only when the trigger input is above the threshold
	 * specified by the \p level argument value. */
	GATE_ABOVE = 1 << 10,

	/** An analog trigger. The operation is enabled only when the trigger input is below the threshold
	 * specified by the \p level argument value. */
	GATE_BELOW = 1 << 11,

	/** Scanning is enabled as long as the external analog trigger is inside
	 * the region defined by the \p level argument value and the \p variance argument value. */
	GATE_IN_WINDOW = 1 << 12,

	/** Scanning is enabled as long as the external analog trigger is outside
	 * the region defined by the \p level argument value and the \p variance argument value. */
	GATE_OUT_WINDOW = 1 << 13,

	/** A digital pattern trigger. The trigger condition is met when the digital pattern
	 * at the trigger input is equal to the pattern specified by the \p level argument value
	 * ANDed with bitwise mask specified by the \p variance argument value of the SetTrigger function
	 * for each subsystem. */
	TRIG_PATTERN_EQ = 1 << 14,

	/** A digital pattern trigger. The trigger condition is met when the digital pattern
	 * at the trigger input is not equal to the pattern specified by the \p level argument value
	 * ANDed with bitwise mask specified by the variance \p argument value of the SetTrigger function
	 * for each subsystem. */
	TRIG_PATTERN_NE = 1 << 15,

	/** A digital pattern trigger. The trigger condition is met when the digital pattern
	 * at the trigger input is greater than the pattern specified by the \p level argument value
	 * ANDed with bitwise mask specified by the \p variance argument value of the SetTrigger function
	 * for each subsystem. Value is determined by additive bit weights. */
	TRIG_PATTERN_ABOVE = 1 << 16,

	/** A digital pattern trigger. The trigger condition is met when the digital pattern
	 * at the trigger input is less than the pattern specified by the \p level argument value
	 * ANDed with bitwise mask specified by the \p variance argument value of the SetTrigger function
	 * for each subsystem. Value is determined by additive bit weights. */
	TRIG_PATTERN_BELOW = 1 << 17
}TriggerType;

/** \brief A structure that defines an analog input queue element
 *
 * The struct contains fields defining the number, mode, and range of the A/D channel.
 */
struct AiQueueElement
{
	/** The analog input channel number for the queue element. */
	int channel;

	/** The input mode to use for the specified channel for the queue element. */
	AiInputMode inputMode;

	/** The range to use for the specified channel for the queue element. */
	Range range;

	/** Reserved for future use */
	char reserved[64];
};

/** \brief A structure that defines an analog input queue element */
typedef struct 	AiQueueElement AiQueueElement;

/** Scan status */
typedef enum
{
	/** Scan is idle */
	SS_IDLE = 0,

	/** Scan is running */
	SS_RUNNING = 1
}ScanStatus;

#ifndef doxy_skip
#define NOSCALEDATA 		1 << 0
#define NOCALIBRATEDATA 	1 << 1
#define SIMULTANEOUS		1 << 2
#define NOCLEAR				1 << 3
#endif /*doxy_skip */

/** Used with many analog input and output functions, as well as a return value for the \p infoValue argument
 * to many of the subsystem GetInfo functions when used with the \p infoItem argument values set to
 * one of the subsystem INFO_SCAN_OPTIONS values. */
typedef enum
{
	/** Transfers A/D data based on the board type and sampling speed. */
	SO_DEFAULTIO	= 0,

	/** Transfers one packet of data at a time. */
	SO_SINGLEIO		= 1 << 0,

	/** Transfers A/D data in blocks. */
	SO_BLOCKIO		= 1 << 1,

	/** Transfers A/D data from the FIFO after the scan completes.
	 * Allows maximum rates for finite scans up to the full capacity of the FIFO. Not recommended for slow acquisition rates. */
	SO_BURSTIO		= 1 << 2,

	/** Scans data in an endless loop. The only way to stop the operation is with ulAInScanStop(). */
	SO_CONTINUOUS 	= 1 << 3,

	/** Data conversions are controlled by an external clock signal. */
	SO_EXTCLOCK		= 1 << 4,

	/** Sampling begins when a trigger condition is met. */
	SO_EXTTRIGGER 	= 1 << 5,

	/** Re-arms the trigger after a trigger event is performed. */
	SO_RETRIGGER 	= 1 << 6,

	/** Enables burst mode sampling, minimizing the channel skew. */
	SO_BURSTMODE 	= 1 << 7,

	/** Enables or disables the internal pacer output on a DAQ device. */
	SO_PACEROUT		= 1 << 8,

	/** Changes the internal clock's timebase to an external timebase source. This can allow synchronization of multiple DAQ devices. */
	SO_EXTTIMEBASE	= 1 << 9,

	/** Enables or disables the internal timebase output on a DAQ device. */
	SO_TIMEBASEOUT	= 1 << 10

}ScanOption;

/** Use as the \p flags argument value for the ulAInScan() function to set properties of data returned. */
typedef enum
{
	/** Data is returned with scaling and calibration factors applied. */
	AINSCAN_FF_DEFAULT 				= 0,

	/** Data is returned in native format, without scaling applied. */
	AINSCAN_FF_NOSCALEDATA 			= NOSCALEDATA, 		

	/** Data is returned without calibration factors applied. */
	AINSCAN_FF_NOCALIBRATEDATA 		= NOCALIBRATEDATA 	
}AInScanFlag;

/** Use as the \p flags argument value for ulAIn() to set the properties of data returned. */
typedef enum
{
	/** Data is returned with scaling and calibration factors applied. */
	AIN_FF_DEFAULT = 0,

	/** Data is returned in native format, without scaling applied. */
	AIN_FF_NOSCALEDATA 			= NOSCALEDATA, 		

	/** Data is returned without calibration factors applied. */
	AIN_FF_NOCALIBRATEDATA 		= NOCALIBRATEDATA 	
}AInFlag;

/** Use as the \p flags argument value for ulAOutScan() to set the properties of data supplied to the function. */
typedef enum
{
	/** Scaled data is supplied and calibration factors are applied to output. */
	AOUTSCAN_FF_DEFAULT					= 0,

	/** Data is supplied in native format (usually, values ranging from 0 to 2<sup>resolution</sup> - 1. */
	AOUTSCAN_FF_NOSCALEDATA 			= NOSCALEDATA, 		

	/** Data is output without calibration factors applied. */
	AOUTSCAN_FF_NOCALIBRATEDATA 		= NOCALIBRATEDATA 	
}AOutScanFlag;

/** Use as the \p flags argument value for ulTIn() to set the properties of data returned; reserved for future use. */
typedef enum
{
	/** Placeholder value. Standard functionality. */
	TIN_FF_DEFAULT = 0,

	/** Wait for new data before returning. */
	TIN_FF_WAIT_FOR_NEW_DATA = 1
}TInFlag;

/** Use as the \p flags argument value for ulTInArray() to set the properties of data returned; reserved for future use. */
typedef enum
{
	/** Placeholder value. Standard functionality. */
	TINARRAY_FF_DEFAULT = 0,

	/** Wait for new data before returning. */
	TINARRAY_FF_WAIT_FOR_NEW_DATA = 1
}TInArrayFlag;

/** Use as the \p flags argument value for ulAOut() to set the properties of data supplied to the function. */
typedef enum
{
	/** Scaled data is supplied and calibration factors are applied to output. */
	AOUT_FF_DEFAULT	= 0,

	/** Data is supplied in native format (usually, values ranging from 0 to 2<sup>resolution</sup> - 1). */
	AOUT_FF_NOSCALEDATA 			= NOSCALEDATA, 		

	/** Data is output without calibration factors applied. */
	AOUT_FF_NOCALIBRATEDATA 		= NOCALIBRATEDATA 	
}AOutFlag;

/** Use as the \p flags argument value for ulAOutArray() to set the properties of data supplied to the function. */
typedef enum
{
	/** Scaled data is supplied and calibration factors are applied to output. */
	AOUTARRAY_FF_DEFAULT	= 0,

	/** Data is supplied in native format (usually, values ranging from 0 to 2<sup>resolution</sup> - 1). */
	AOUTARRAY_FF_NOSCALEDATA 			= NOSCALEDATA,

	/** Data is output without calibration factors applied. */
	AOUTARRAY_FF_NOCALIBRATEDATA 		= NOCALIBRATEDATA ,

	/** All of the specified channels will be updated simultaneously. */
	AOUTARRAY_FF_SIMULTANEOUS			= SIMULTANEOUS
}AOutArrayFlag;

/** Use with #AoConfigItem to set configuration options at runtime. */
typedef enum
{
	/** Receive the D/A Load signal from an external source */
	AOSM_SLAVE	= 0,

	/** Output the internal D/A Load signal */
	AOSM_MASTER	= 1
}AOutSyncMode;

/** Use with #AoConfigItem to set configuration options at runtime. */
typedef enum
{
	/** Sense mode is disabled. */
	AOSM_DISABLED	= 1,

	/** Sense mode is enable. */
	AOSM_ENABLED		= 2
}AOutSenseMode;



/** Use as the \p flags argument value for ulCInScan() to set counter properties. */
typedef enum
{
	/** Default counter behavior */
	CINSCAN_FF_DEFAULT 			= 0,

	/** Sets up the counter as a 16-bit counter channel */
	CINSCAN_FF_CTR16_BIT 		= 1 << 0,

	/** Sets up the counter as a 32-bit counter channel */
	CINSCAN_FF_CTR32_BIT 		= 1 << 1,

	/** Sets up the counter as a 64-bit counter channel */
	CINSCAN_FF_CTR64_BIT 		= 1 << 2,

	/** Does not clear the counter to 0 at the start of each scan. */
	CINSCAN_FF_NOCLEAR			= NOCLEAR,

	/** Sets up the counter as a 48-bit counter channel */
	CINSCAN_FF_CTR48_BIT 		= 1 << 4
}CInScanFlag;

/** Use as the \p flags argument value for ulDInScan() to set the properties of data returned. */
typedef enum
{
	/** Standard scan properties. Placeholder for future values */
	DINSCAN_FF_DEFAULT 			= 0,
}DInScanFlag;

/** Use as the \p flags argument value for ulDOutScan() to set properties of data sent. */
typedef enum
{
	/** Standard scan properties. Placeholder for future values. */
	DOUTSCAN_FF_DEFAULT 			= 0,
}DOutScanFlag;

/** Use as the \p flags argument value for ulDaqInScan() to set the properties of data returned. */
typedef enum
{
	/** Data is returned with scaling and calibration factors applied to analog channel data. */
	DAQINSCAN_FF_DEFAULT			= 0,

	/** Data for analog channels is returned in native format, without scaling applied. */
	DAQINSCAN_FF_NOSCALEDATA 		= NOSCALEDATA, 		

	/** Data for analog channels is returned without calibration factors applied. */
	DAQINSCAN_FF_NOCALIBRATEDATA 	= NOCALIBRATEDATA, 	

	/** Counters are not cleared (set to 0) when a scan starts. */
	DAQINSCAN_FF_NOCLEAR			= NOCLEAR			
}DaqInScanFlag;

/** Use as the \p flags argument value for ulDaqOutScan() to set the properties of data sent. */
typedef enum
{
	/** The data buffer contains scaled data for analog channels, and calibration factors are applied to analog outputs. */
	DAQOUTSCAN_FF_DEFAULT			= 0,

	/** Data for analog channels is in native format, without scaling applied. */
	DAQOUTSCAN_FF_NOSCALEDATA 		= NOSCALEDATA, 		

	/** Data for analog channels is output without calibration factors applied. */
	DAQOUTSCAN_FF_NOCALIBRATEDATA 	= NOCALIBRATEDATA 	
}DaqOutScanFlag;

/** Use as the value for the \p type argument for ulCConfigScan(). Use ulCtrGetInfo() with the ::CTR_INFO_MEASUREMENT_TYPES \p infoItem
 * to check compatibility.
 */
typedef enum
{
	/** Counter measurement. The counter increments on the active edge of the input. */
	CMT_COUNT =			1 << 0,

	/** Period measurement. Measures the number of ticks between active edges of the input, with the
	 * granularity of measurement set by the \p tickSize argument of ulCConfigScan(). */
	CMT_PERIOD = 		1 << 1,

	/** Pulsewidth measurement. Measures the number of ticks between the active edge of the counter input
	 * and the following edge of the counter input, with the granularity of measurement set by the
	 * \p tickSize argument of ulCConfigScan(). */
	CMT_PULSE_WIDTH = 	1 << 2,

	/** Timing measurement. Measures the number of ticks between the active edge of the counter input and
	 * the active edge of the gate input, with granularity of measurement set by the \p tickSize argument
	 * of ulCConfigScan(). */
	CMT_TIMING = 		1 << 3,

	/** Encoder measurement. Configures the counter as an encoder, if supported. */
	CMT_ENCODER =		1 << 4
}CounterMeasurementType;

/** Use as the value for the \p mode argument for ulCConfigScan(). This value should be set consistent with the \p type argument value. */
typedef enum
{
	/** Configures the counter for default counting modes for the ::CMT_COUNT measurement type. */
	CMM_DEFAULT =						0,

	/** Configures the counter to clear after every read for the ::CMT_COUNT measurement type. */
	CMM_CLEAR_ON_READ =					1 << 0,

	/** Configures the counter to count down for the ::CMT_COUNT measurement type. */
	CMM_COUNT_DOWN =					1 << 1,

	/** Configures the counter to increment when the gate pin is high, and decrement when the gate pin is low
	 * for the ::CMT_COUNT measurement type. */
	CMM_GATE_CONTROLS_DIR =				1 << 2,

	/** Configures the counter to clear when the gate input is high for the ::CMT_COUNT measurement type. */
	CMM_GATE_CLEARS_CTR =				1 << 3,

	/** Configures the counter to start counting when the gate input goes active for the ::CMT_COUNT measurement type.
	 * By default, active is on the rising edge. The gate is re-armed when the counter is loaded and when ulCConfigScan() is called. */
	CMM_GATE_TRIG_SRC =					1 << 4,

	/** Configures the counter output to go high when the counter reaches the value of output register 0 for
	 * the ::CMT_COUNT measurement type, and go low when the counter reaches the value of output register 1.
	 * Use ulCLoad() to set or read the value of the output registers. */
	CMM_OUTPUT_ON =						1 << 5,

	/** Configures the initial state of the counter output pin high for the ::CMT_COUNT measurement type. */
	CMM_OUTPUT_INITIAL_STATE_HIGH =		1 << 6,

	/** Configures the counter to restart when a clear or load operation is performed, or the count direction changes
	 * for the ::CMT_COUNT measurement type. */
	CMM_NO_RECYCLE = 					1 << 7,

	/** When counting up, configures the counter to roll over to the min limit when the max limit is reached for the
	 * ::CMT_COUNT measurement type. When counting down, configures the counter to roll over to max limit when the min limit
	 * is reached. When counting up with ::CMM_NO_RECYCLE enabled, the counter freezes whenever the count reaches
	 * the value that was loaded into the max limit register. When counting down with ::CMM_NO_RECYCLE enabled, the counter freezes
	 * whenever the count reaches the value that was loaded into the min limit register.
	 * Counting resumes if the counter is reset or the direction changes. */
	CMM_RANGE_LIMIT_ON =				1 << 8,

	/** Enables the counter when the mapped channel or gate pin is high for the ::CMT_COUNT measurement type. */
	CMM_GATING_ON = 					1 << 9,

	/** Inverts the polarity of the gate input for the ::CMT_COUNT measurement type. */
	CMM_INVERT_GATE =					1 << 10,

	/** Latches the counter measurement each time 1 complete period is observed for the ::CMT_PERIOD measurement type. */
	CMM_PERIOD_X1 =						0,

	/** Latches the counter measurement each time 10 complete periods are observed for the ::CMT_PERIOD measurement type. */
	CMM_PERIOD_X10 =					1 << 11,

	/** Latches the counter measurement each time 100 complete periods are observed for the ::CMT_PERIOD measurement type. */
	CMM_PERIOD_X100 =					1 << 12,

	/** Latches the counter measurement each time 1000 complete periods are observed for the ::CMT_PERIOD measurement type. */
	CMM_PERIOD_X1000 =					1 << 13,

	/** Enables the counter when the mapped channel or gate pin is high for the ::CMT_PERIOD measurement type. */
	CMM_PERIOD_GATING_ON =				1 << 14,

	/** Inverts the polarity of the gate input for the ::CMT_PERIOD measurement type. */
	CMM_PERIOD_INVERT_GATE =			1 << 15,

	/** Configures the counter for default pulse width modes for the ::CMT_PULSE_WIDTH measurement type. */
	CMM_PULSE_WIDTH_DEFAULT =			0,

	/** Enables the counter when the mapped channel or gate pin is high for the ::CMT_PULSE_WIDTH measurement type. */
	CMM_PULSE_WIDTH_GATING_ON =			1 << 16,

	/** Inverts the polarity of the gate input for the ::CMT_PULSE_WIDTH measurement type. */
	CMM_PULSE_WIDTH_INVERT_GATE =		1 << 17,

	/** Configures the counter for default timing modes for the ::CMT_TIMING  measurement type. */
	CMM_TIMING_DEFAULT	=				0,

	/** Inverts the polarity of the gate input for the ::CMT_TIMING measurement type.*/
	CMM_TIMING_MODE_INVERT_GATE =		1 << 18,

	/** Sets the encoder measurement mode to X1 for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_X1 =					0,

	/** Sets the encoder measurement mode to X2 for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_X2 =					1 << 19,

	/** Sets the encoder measurement mode to X4 for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_X4	=					1 << 20,

	/** Configures the encoder Z mapped signal to latch the counter outputs for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_LATCH_ON_Z =			1 << 21,

	/** Clears the counter when the index (Z input) goes active for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_CLEAR_ON_Z =			1 << 22,

	/** Disables the counter when a count overflow or underflow occurs for the ::CMT_ENCODER measurement type; re-enables when
	 * a clear or load operation is performed on the counter. */
	CMM_ENCODER_NO_RECYCLE = 			1 << 23,

	/** Enables upper and lower limits for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_RANGE_LIMIT_ON	=		1 << 24,

	/** Sets the encoder Z signal as the active edge for the ::CMT_ENCODER measurement type. */
	CMM_ENCODER_Z_ACTIVE_EDGE	=		1 << 25,

	/** Configures the counter to be latched by the signal on the index pin for the ::CMT_COUNT measurement type. */
	CMM_LATCH_ON_INDEX	= 				1 << 26,

	/** Configures the counter to increment when the phase B pin is high, and decrement when the phase B pin is low
	 * for the ::CMT_COUNT measurement type. */
	CMM_PHB_CONTROLS_DIR 	= 			1 << 27,

	/** Configures the counter to decrement by the signal on the mapped channel for the ::CMT_COUNT measurement type. */
	CMM_DECREMENT_ON = 					1 << 28
}CounterMeasurementMode;

/** Use as the value for the \p debounceTime argument for ulCConfigScan() when #CounterDebounceMode is not ::CDM_NONE. */
typedef enum
{
	/** Disables debounce. Valid only when \p debounceMode is set to ::CDM_NONE. */
	CDT_DEBOUNCE_0ns =		0,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 500ns. */
	CDT_DEBOUNCE_500ns =	1,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 1500 ns. */
	CDT_DEBOUNCE_1500ns =   2,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 3500 ns. */
	CDT_DEBOUNCE_3500ns =   3,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 7500 ns. */
	CDT_DEBOUNCE_7500ns =   4,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 15500 ns. */
	CDT_DEBOUNCE_15500ns =  5,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 31500 ns. */
	CDT_DEBOUNCE_31500ns =  6,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 63500 ns. */
	CDT_DEBOUNCE_63500ns =  7,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 127500 ns. */
	CDT_DEBOUNCE_127500ns = 8,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 100 us. */
	CDT_DEBOUNCE_100us =    9,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 300 us. */
	CDT_DEBOUNCE_300us =    10,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 700 us. */
	CDT_DEBOUNCE_700us =    11,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 1500 us. */
	CDT_DEBOUNCE_1500us =   12,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 3100 us. */
	CDT_DEBOUNCE_3100us =   13,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 6300 us. */
	CDT_DEBOUNCE_6300us =   14,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 12700 us. */
	CDT_DEBOUNCE_12700us =  15,

	/** Sets the time period that the counter input must be stable when using ::CDM_TRIGGER_AFTER_STABLE or
	 * ::CDM_TRIGGER_BEFORE_STABLE CounterDebounceModes to 25500 us. */
	CDT_DEBOUNCE_25500us =  16
}CounterDebounceTime;

/** Use as the value for the \p debounceTime argument for ulCConfigScan() to set the glitch rejection properties of a counter. */
typedef enum
{
	/** Disables the debounce feature. */
	CDM_NONE					= 0,

	/** The counter is incremented only after the counter input is stable for a period of a length defined by #CounterDebounceTime. */
	CDM_TRIGGER_AFTER_STABLE 	= 1,

	/** The counter is incremented on the first edge at the counter input, then waits for a stable period of a length defined by
	 * #CounterDebounceTime before counting the next edge. */
	CDM_TRIGGER_BEFORE_STABLE 	= 2
}CounterDebounceMode;

/** Use as the value for the \p edgeDetection argument for ulCConfigScan(). */
typedef enum
{
	/** Rising edge */
	CED_RISING_EDGE 			= 1,

	/** Falling edge */
	CED_FALLING_EDGE 			= 2
}CounterEdgeDetection;

/** Use as the value for the \p tickSize argument for ulCConfigScan() when #CounterDebounceMode is
 * ::CMT_PERIOD, ::CMT_PULSE_WIDTH, or ::CMT_TIMING. Refer to the device hardware manual to determine
 * which sizes are compatible with your device. */
typedef enum
{
	/** Sets the tick size to 20.83 ns. */
	CTS_TICK_20PT83ns			= 1,

	/** Sets the tick size to 208.3 ns. */
	CTS_TICK_208PT3ns			= 2,

	/** Sets the tick size to 2083.3 ns. */
	CTS_TICK_2083PT3ns	 		= 3,

	/** Sets the tick size to 20833.3 ns. */
	CTS_TICK_20833PT3ns 		= 4,

	/** Sets the tick size to 20 ns. */
	CTS_TICK_20ns 				= 11,

	/** Sets the tick size to 200 ns. */
	CTS_TICK_200ns 				= 12,

	/** Sets the tick size to 2000 ns. */
	CTS_TICK_2000ns 			= 13,

	/** Sets the tick size to 20000 ns. */
	CTS_TICK_20000ns 			= 14
}CounterTickSize;

/** Use as the \p flags argument value for ulCConfigScan(). Reserved for future use. */
typedef enum
{
	/** Placeholder value. Standard functionality. */
	CF_DEFAULT = 0
}
CConfigScanFlag;

/** Used for the ulCLoad() \p registerType argument, and as the value returned by ulCtrGetInfo() for the ::CTR_INFO_REGISTER_TYPES infoItem. */
typedef enum
{
	/** Counter register */
	CRT_COUNT 		= 1 << 0,

	/** Load register */
	CRT_LOAD 		= 1 << 1,

	/** Max Limit register */
	CRT_MIN_LIMIT	= 1 << 2,

	/** Min Limit register */
	CRT_MAX_LIMIT	= 1 << 3,

	/** The register that sets the count value at which the counter output will change state from its original state. */
	CRT_OUTPUT_VAL0	= 1 << 4,

	/** The register that sets the count value at which the counter output will reset to its original state. */
	CRT_OUTPUT_VAL1	= 1 << 5
}CounterRegisterType;

/** A bitmask used with synchronous input scanning operations as a field in the DaqInChanDescriptor struct,
 * and as a value returned in the \p infoValue argument for ulDaqIGetInfo() used with ::DAQI_INFO_CHAN_TYPES.
 */
typedef enum
{
	/** Analog input channel, differential mode */
	DAQI_ANALOG_DIFF 	= 1 << 0,

	/** Analog input channel, single-ended mode */
	DAQI_ANALOG_SE 		= 1 << 1,

	/** Digital channel */
	DAQI_DIGITAL 		= 1 << 2,

	/** 16-bit counter channel. */
	DAQI_CTR16			= 1 << 3,

	/** 32-bit counter channel. */
	DAQI_CTR32			= 1 << 4,

	/** 48-bit counter channel. */
	DAQI_CTR48			= 1 << 5,
	/** DAQI_CTR64 */

	DAQI_DAC			= 1 << 7
}DaqInChanType;

/** \brief A structure that defines an input channel and its properties. Used with ulDaqInScan().
 *
 * The struct contains fields for the channel number, type, and range.
 */
struct DaqInChanDescriptor
{
	/** The channel number. */
	int channel;

	/** The type of input for the specified channel, such as analog, digital, or counter. */
	DaqInChanType type;

	/** The range to be used for the specified channel; ignored if not analog. */
	Range range;

	/** Reserved for future use */
	char reserved[64];
};

/** A structure that defines an input channel and its properties. Used with ulDaqInScan() */
typedef struct 	DaqInChanDescriptor DaqInChanDescriptor;

/** A bitmask used with synchronous output scanning operations as a field in the DaqOutChanDescriptor struct,
 * and as a value returned in the \p infoValue argument for ulDaqOGetInfo() used with ::DAQO_INFO_CHAN_TYPES. */
typedef enum
{
	/** Analog output */
	DAQO_ANALOG			= 1 << 0,

	/** Digital output */
	DAQO_DIGITAL 		= 1 << 1
}DaqOutChanType;


/** \brief A structure that defines an output channel and its properties. Used with ulDaqOutScan().
 *
 * The struct contains fields for the channel number, type, and range.
*/
struct DaqOutChanDescriptor
{
	/** The channel number. */
	int channel;

	/** The type of the specified channel, such as analog or digital. */
	DaqOutChanType type;

	/** The range to be used for the specified channel; ignored if not analog. */
	Range range;

	/** Reserved for future use */
	char reserved[64];
};

/** \brief A structure that defines an output channel and its properties. Used with ulDaqOutScan(). */
typedef struct 	DaqOutChanDescriptor DaqOutChanDescriptor;

/** Used with ulTmrPulseOutStart() as the \p options argument value to set advanced options for the specified device. */
typedef enum
{
	/** No PulseOut options are applied. */
	PO_DEFAULT = 0,

	/** The output operation is held off until the specified trigger condition is met.
	 * Trigger conditions may be modified using ulTmrSetTrigger(). */
	PO_EXTTRIGGER = 1 << 5,

	/** The output operation is held off until the specified trigger condition is met.
	 * The trigger is re-armed when the output operation completes. Intended to be used with
	 * a non-zero value set for the \p pulseCount argument (non-continuous timer output).
	 * Trigger conditions may be modified using ulTmrSetTrigger(). **/
	PO_RETRIGGER = 1 << 6
} PulseOutOption;

/** A bitmask defining the types of conditions that trigger an event.
 * Used with ulDevGetInfo() as return values for ::DEV_INFO_DAQ_EVENT_TYPES,
 * and as the \p eventType argument value for ulEnableEvent() and ulDisableEvent(). */
typedef enum
{
	/** No event type. Possible return value for ulDevGetInfo(). Not a valid value for
	 * ulEnableEvent() and ulDisableEvent(). */
	DE_NONE 						= 0, 

	/** Defines an event trigger condition that occurs when a specified
	 * number of samples are available. */
	DE_ON_DATA_AVAILABLE =			1 << 0,

	/** Defines an event trigger condition that occurs when an input scan error occurs. */
	DE_ON_INPUT_SCAN_ERROR =		1 << 1,

	/** Defines an event trigger condition that occurs upon completion of an input scan operation
	 * such as ulAInScan(). */
	DE_ON_END_OF_INPUT_SCAN =		1 << 2,

	/** Defines an event trigger condition that occurs when an output scan error occurs. */
	DE_ON_OUTPUT_SCAN_ERROR =		1 << 3,

	/**  Defines an event trigger condition that occurs upon completion of an output scan operation
	 * such as ulAOutScan(). */
	DE_ON_END_OF_OUTPUT_SCAN =		1 << 4

}DaqEventType;

/** Used with ulMemGetInfo() as the \p memRegion argument value to specify the memory location on the specified device. */
typedef enum
{
	/** Specifies the calibration data region information returned to the MemDescriptor struct */
	MR_CAL =		1 << 0,

	/** Specifies the user data region information returned to the MemDescriptor struct */
	MR_USER = 		1 << 1,

	/** Specifies the data settings region information returned to the MemDescriptor struct */
	MR_SETTINGS = 	1 << 2,

	/** Specifies the first reserved region information returned to the MemDescriptor struct */
	MR_RESERVED0 = 	1 << 3
}MemRegion;

/** A bitmask used with ulMemGetInfo() as one of the field types returned in the MemDescriptor struct.
 * Indicates access permissions for the location specified by #MemRegion for the specified device. */
typedef enum
{
	/** Indicates read access for the location specified by the \p memRegion argument */
	MA_READ =		1 << 0,

	/** Indicates write access for the location specified by the \p memRegion argument */
	MA_WRITE = 		1 << 1
}MemAccessType;

/** The callback function called in response to an event condition. */
typedef void (*DaqEventCallback)(DaqDeviceHandle, DaqEventType, unsigned long long, void*);

/** Used with the subsystem ScanWait functions as the \p waitType argument value for the specified device. */
typedef enum
{
	/** Function returns when the scan operation completes or the time specified by the \p timeout argument value elapses. */
	WAIT_UNTIL_DONE = 1 << 0
}WaitType;

#ifndef doxy_skip
/** Library version */
typedef enum
{
	UL_INFO_VER_STR = 2000
}UlInfoItemStr;

typedef enum
{	
	UL_CFG_USB_XFER_PRIORITY = 1
}UlConfigItem;
#endif /* doxy_skip */

/** Use with ulDevGetInfo() as an \p infoItem argument value to obtain information for the specified device. */
typedef enum
{
	/** Returns a non-zero value to \p infoValue if analog input is supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_HAS_AI_DEV = 1,

	/** Returns a non-zero value to \p infoValue if analog output is supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_HAS_AO_DEV = 2,

	/** Returns a non-zero value to \p infoValue if digital I/O is supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_HAS_DIO_DEV = 3,

	/** Returns a non-zero value to \p infoValue if counter input is supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_HAS_CTR_DEV = 4,

	/** Returns a non-zero value to \p infoValue if timer output is supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_HAS_TMR_DEV = 5,

	/** Returns a non-zero value to \p infoValue if synchronous input is supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_HAS_DAQI_DEV = 6,

	/** Returns a non-zero value to \p infoValue if synchronous output is supported. Index is ignored. */
	DEV_INFO_HAS_DAQO_DEV = 7,

	/** Returns a bitmask of #DaqEventType values to \p infoValue if events are supported; otherwise, returns zero. Index is ignored. */
	DEV_INFO_DAQ_EVENT_TYPES = 8,

	/** Returns a bitmask of #MemRegion values to \p infoValue, indicating memory regions available. */
	DEV_INFO_MEM_REGIONS = 9
}DevInfoItem;

/** Use with ulDevGetConfig() as a \p configItem argument value to get the current configuration
 * of the specified device.
 */
typedef enum
{
	/** Returns a non-zero value to \p configValue if an expansion board is attached; otherwise, returns zero. */
	DEV_CFG_HAS_EXP = 1,

	/** The connection code stored in EEPROM. In order to modify the connection code, first the memory must be unlocked with #DEV_CFG_MEM_UNLOCK_CODE.
	 * Note: when the connection code is modified, the DAQ device must be reset with #DEV_CFG_RESET for the change to take effect.
	 */
	DEV_CFG_CONNECTION_CODE = 2,

	/** Memory unlock code. The unlock code is 0xAA55 */
	DEV_CFG_MEM_UNLOCK_CODE = 3,

	/** Resets the DAQ device, this causes the DAQ device to disconnect from the host, ulConnectDaqDevice() must be invoked to re-establish the connection*/
	DEV_CFG_RESET = 4

}DevConfigItem;

/** Use with ulDevGetConfigStr() as a \p configItem argument value to get the current configuration
 * of the specified the specified device.
 */
typedef enum
{
	/** Returns the version of the device system defined by the #DevVersionType value of the \p index argument */
	DEV_CFG_VER_STR = 2000,

	/** Returns the IP address of the Ethernet DAQ device */
	DEV_CFG_IP_ADDR_STR = 2001,

	/** Returns the name of the network interface which is used to connect to the Ethernet DAQ device */
	DEV_CFG_NET_IFC_STR = 2002
}DevConfigItemStr;

/** Used with ulDevGetConfigStr() as an \p index argument value with the \p infoItem argument set to ::DEV_CFG_VER_STR for the specified device. */
typedef enum
{
	/** Firmware version installed on the current device is returned to the \p configStr argument. */
	DEV_VER_FW_MAIN = 0,

	/** FPGA version installed on the current device is returned to the \p configStr argument. */
	DEV_VER_FPGA = 1,

	/** Radio firmware version installed on the current device is returned to the \p configStr argument. */
	DEV_VER_RADIO = 2,

	/** Measurement firmware version installed on the current device is returned to the \p configStr argument. */
	DEV_VER_FW_MEASUREMENT = 3,

	/** Measurement firmware version installed on the expansion device attached to the current device is returned to the \p configStr argument. */
	DEV_VER_FW_MEASUREMENT_EXP = 4
}DevVersionType;

/** Use with ulAIGetInfo() to obtain information about the analog input subsystem for the specified device
 * as an \p infoItem argument value. */
typedef enum
{
	/** Returns the A/D resolution in number of bits to the \p infoValue argument. Index is ignored. */
	AI_INFO_RESOLUTION = 1,

	/** Returns the total number of A/D channels to the \p infoValue argument. Index is ignored. */
	AI_INFO_NUM_CHANS = 2,

	/** Returns the number of A/D channels for the specified channel mode to the \p infoValue argument. Set index to one of the #AiInputMode enum values. */
	AI_INFO_NUM_CHANS_BY_MODE = 3,

	/** Returns the number of A/D channels for the specified channel type to the \p infoValue argument. Set index to one of the #AiChanType enum values. */
	AI_INFO_NUM_CHANS_BY_TYPE = 4,

	/** Returns a bitmask of supported #AiChanType values to the \p infoValue argument. Index is ignored. */
	AI_INFO_CHAN_TYPES = 5,

	/** Returns a bitmask of supported #ScanOption values to the \p infoValue argument. Index is ignored. */
	AI_INFO_SCAN_OPTIONS = 6,

	/** Returns a zero or non-zero value to the \p infoValue argument. If non-zero, paced operations are supported. Index is ignored. */
	AI_INFO_HAS_PACER = 7,

	/** Returns the number of differential ranges supported to the \p infoValue argument. Index is ignored. */
	AI_INFO_NUM_DIFF_RANGES = 8,

	/** Returns the number of single-ended ranges supported to the \p infoValue argument. Index is ignored. */
	AI_INFO_NUM_SE_RANGES = 9,

	/** Returns a #Range value to the \p infoValue argument based on the value of the \p index argument specified.
	 * Index should be a number between zero and (the number of differential ranges supported) &ndash; 1.
	 * See #AI_INFO_NUM_DIFF_RANGES. */
	AI_INFO_DIFF_RANGE = 10,

	/** Returns a #Range value to the \p infoValue argument based on the value of the \p index argument specified.
	 * Index should be a number between zero and (the number of single-ended ranges supported) &ndash; 1.
	 * See #AI_INFO_NUM_SE_RANGES. */
	AI_INFO_SE_RANGE = 11,

	/** Returns a bitmask of supported #TriggerType values to the \p infoValue argument. Index is ignored. */
	AI_INFO_TRIG_TYPES = 12,

	/** Returns the maximum length of the queue for the specified channel mode to the \p infoValue argument.
	 * Set index to one of the #AiInputMode enum values. */
	AI_INFO_MAX_QUEUE_LENGTH_BY_MODE = 13,

	/** Returns a bitmask of supported #AiQueueType values to the \p infoValue argument. Index is ignored. */
	AI_INFO_QUEUE_TYPES = 14,

	/** Returns a bitmask of #AiChanQueueLimitation values to the \p infoValue argument that apply to the queue. Index is ignored. */
	AI_INFO_QUEUE_LIMITS = 15,

	/** Returns the FIFO size in bytes to the \p infoValue argument. Index is ignored. */
	AI_INFO_FIFO_SIZE = 16,

	/** Returns a zero or non-zero value to the \p infoValue argument. If non-zero, IEPE mode is supported. Index is ignored. */
	AI_INFO_IEPE_SUPPORTED = 17

}AiInfoItem;

/** Use with ulAIGetInfoDbl() to obtain information about the analog input subsystem for the specified device
 * as an \p infoItem argument value. */
typedef enum
{
	/** Returns the minimum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	AI_INFO_MIN_SCAN_RATE = 1000,

	/** Returns the maximum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	AI_INFO_MAX_SCAN_RATE = 1001,

	/** Returns the maximum throughput in samples per second to the \p infoValue argument. Index is ignored. */
	AI_INFO_MAX_THROUGHPUT = 1002,

	/** Returns the maximum scan rate in samples per second when using the ::SO_BURSTIO ScanOption to the \p infoValue argument. Index is ignored. */
	AI_INFO_MAX_BURST_RATE = 1003,

	/** Returns the maximum throughput in samples per second when using the ::SO_BURSTIO ScanOption to the \p infoValue argument. Index is ignored. */
	AI_INFO_MAX_BURST_THROUGHPUT = 1004
}AiInfoItemDbl;

/** Use with ulAISetConfig() and ulAIGetConfig() to configure the AI subsystem. */
typedef enum
{
	/** The channel type of the specified channel. Set with #AiChanType. */
	AI_CFG_CHAN_TYPE = 1,

	/** The thermocouple type of the specified channel. Set with #TcType. */
	AI_CFG_CHAN_TC_TYPE = 2,

#ifndef doxy_skip
	/** The temperature unit of the specified analog input scan channel. Set with #TempUnit. */
	AI_CFG_SCAN_CHAN_TEMP_UNIT = 3,
#endif /* doxy_skip */
	/** The analog input scan temperature unit. Set with #TempUnit. */
	AI_CFG_SCAN_TEMP_UNIT = 4,
	
#ifndef doxy_skip
	/** The timing mode. Set with #AdcTimingMode. */
	AI_CFG_ADC_TIMING_MODE = 5,

	/** The auto zero mode. Set with #AutoZeroMode. */
	AI_CFG_AUTO_ZERO_MODE = 6,

	/** The date when the device was calibrated last in UNIX Epoch time. Set index to 0 for the factory calibration date,
	* or 1 for the field calibration date.
	* If the value read is not a valid date or the index is invalid, 0 (Unix Epoch) is returned. */
	AI_CFG_CAL_DATE = 7,
#endif /* doxy_skip */

	/** The IEPE current excitation mode for the specified channel. Set with #IepeMode. */
	AI_CFG_CHAN_IEPE_MODE = 8,

	/** The coupling mode for the specified device. Set with #CouplingMode. */
	AI_CFG_CHAN_COUPLING_MODE = 9,

	/** The connection type of the sensor connected to the specified channel. */
	AI_CFG_CHAN_SENSOR_CONNECTION_TYPE = 10,

	/** The open thermocouple detection mode for the specified channel. Set with #OtdMode. */
	AI_CFG_CHAN_OTD_MODE = 11,

	/** The open thermocouple detection mode. Set with #OtdMode. */
	AI_CFG_OTD_MODE = 12,

	/** The calibration table type. Set with #AiCalTableType. */
	AI_CFG_CAL_TABLE_TYPE = 13,

	/** The rejection frequency type. Set with #AiRejectFreqType. */
	AI_CFG_REJECT_FREQ_TYPE = 14,

#ifndef doxy_skip
	/** The date when the expansion board was calibrated last in UNIX Epoch time. Set index to 0 for the factory calibration date,
	* or 1 for the field calibration date.
	* If the value read is not a valid date or the index is invalid, 0 (Unix Epoch) is returned. */
	AI_CFG_EXP_CAL_DATE = 15,
#endif /* doxy_skip */

}AiConfigItem;

/** Use with ulAISetConfigDbl() and ulAIGetConfigDbl() to configure the AI subsystem. */
typedef enum
{
	/** The custom slope of the specified channel. */
	AI_CFG_CHAN_SLOPE = 1000,

	/** The custom offset of the specified channel. */
	AI_CFG_CHAN_OFFSET = 1001,

	/** The sensitivity of the sensor connected to the specified channel. */
	AI_CFG_CHAN_SENSOR_SENSITIVITY = 1002,

	/** The data rate of the specified channel. */
	AI_CFG_CHAN_DATA_RATE = 1003
}AiConfigItemDbl;

/** Use with ulAIGetConfigStr() as a \p configItem argument value to get the current analog input configuration of the current device. */

typedef enum
{
	/** Returns the calibration date. Set index to 0 for the factory calibration date, or 1 for the field calibration date.
	* If the value read is not a valid date or the index is invalid, Unix Epoch is returned. */
	AI_CFG_CAL_DATE_STR = 2000,

	/** Returns the channel coefficients used for the configured sensor. */
	AI_CFG_CHAN_COEFS_STR = 2001,

	/** Returns the calibration date of expansion board. Set index to 0 for the factory calibration date, or 1 for the field calibration date.
	* If the value read is not a valid date or the index is invalid, Unix Epoch is returned. */
	AI_CFG_EXP_CAL_DATE_STR = 2002,

}AiConfigItemStr;

/** Use with ulAOGetInfo() to obtain information about the analog output subsystem for the specified device
 * as an \p infoItem argument value. */
typedef enum
{
	/** Returns the D/A resolution in number of bits to the \p infoValue argument. Index is ignored. */
	AO_INFO_RESOLUTION = 1,

	/** Returns the total number of D/A channels to the \p infoValue argument. Index is ignored. */
	AO_INFO_NUM_CHANS = 2,

	/** Returns a bitmask of supported #ScanOption values to the \p infoValue argument. Index is ignored. */
	AO_INFO_SCAN_OPTIONS = 3,

	/** Returns a zero or non-zero value to the \p infoValue argument. If non-zero, paced operations are supported. Index is ignored. */
	AO_INFO_HAS_PACER = 4,

	/** Returns the number of analog output ranges supported to the \p infoValue argument. Index is ignored. */
	AO_INFO_NUM_RANGES = 5,

	/** Returns a Range value to the \p infoValue argument based on the value of the \p index argument specified.
	 * Index should be a number between zero and (the number of ranges supported) &ndash; 1. See #AO_INFO_NUM_RANGES. */
	AO_INFO_RANGE = 6,

	/** Returns a bitmask of supported #TriggerType values to the \p infoValue argument. Index is ignored. */
	AO_INFO_TRIG_TYPES = 7,

	/** Returns the FIFO size in bytes to the \p infoValue argument. Index is ignored. */
	AO_INFO_FIFO_SIZE = 8
}AoInfoItem;

/** Use with ulAOGetInfoDbl() to obtain information about the analog output subsystem for the specified device
 *  as an \p infoItem argument value. */
typedef enum
{
	/** Returns the minimum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	AO_INFO_MIN_SCAN_RATE = 1000,

	/** Returns the maximum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	AO_INFO_MAX_SCAN_RATE = 1001,

	/** Returns the maximum throughput in samples per second to the \p infoValue argument. Index is ignored. */
	AO_INFO_MAX_THROUGHPUT = 1002
}AoInfoItemDbl;

/** Use with ulAOSetConfig() and ulAOGetConfig() to configure the AO subsystem. */
typedef enum
{
	/** The sync mode. Set with #AOutSyncMode. */
	AO_CFG_SYNC_MODE = 1,

	/** The sense mode for the specified channel. Set with #AOutSenseMode. */
	AO_CFG_CHAN_SENSE_MODE = 2
}AoConfigItem;

/** Use with ulDIOGetInfo() to obtain information about the DIO subsystem for the specified device as an \p infoItem argument value. */
typedef enum
{
	/** Returns the total number of digital ports to the \p infoValue argument. Index is ignored. */
	DIO_INFO_NUM_PORTS = 1,

	/** Returns a #DigitalPortType value to the \p infoValue argument indicating the type of the port
	 * specified by the \p index argument value.
	 * Index should be less than the value obtained using ::DIO_INFO_NUM_PORTS.
	 */
	DIO_INFO_PORT_TYPE = 2,

	/** Returns a #DigitalPortIoType value to the \p infoValue argument indicating the input, output,
	 * and programmability information for the port specified by the \p index argument value.
	 * Index should be less than the value obtained using ::DIO_INFO_NUM_PORTS.
	 */
	DIO_INFO_PORT_IO_TYPE = 3,

	/** Returns a value to the \p infoValue argument indicating the number of bits the specified port has.
	 * The port is specified by the \p index argument value.
	 * Index should be less than the value obtained using ::DIO_INFO_NUM_PORTS.
	 */
	DIO_INFO_NUM_BITS = 4,

	/** Returns a non-zero value to \p infoValue if paced digital operation (scanning) is supported.
	 * Otherwise, returns zero. Index is ignored.
	 */
	DIO_INFO_HAS_PACER = 5,

	/** Returns a bitmask of supported #ScanOption values to the \p infoValue argument for the
	 * specified digital direction.
	 * The direction is specified by setting the \p index argument to one of the #DigitalDirection values.
	 */
	DIO_INFO_SCAN_OPTIONS = 6,

	/** Returns a bitmask of supported #TriggerType values to the \p infoValue argument for the
	 * specified digital direction.
	 * The direction is specified by setting the \p index argument to one of the #DigitalDirection values. */
	DIO_INFO_TRIG_TYPES = 7,

	/** Returns the FIFO size in bytes to the \p infoValue argument for the specified digital direction.
	 * The direction is specified by setting the \p index argument to one of the #DigitalDirection values. */
	DIO_INFO_FIFO_SIZE = 8
}DioInfoItem;

/** Use with ulDIOGetInfoDbl() as an \p infoItem argument to obtain information about the DIO subsystem for the specified device. */
typedef enum
{
	/** Returns the minimum scan rate to the \p infoValue argument for the specified digital direction.
	 * The direction is specified by setting the \p index argument to one of the ::DigitalDirection values. */
	DIO_INFO_MIN_SCAN_RATE = 1000,

	/** Returns the maximum scan rate to the \p infoValue argument for the specified digital direction.
	 * The direction is specified by setting the \p index argument to one of the ::DigitalDirection values. */
	DIO_INFO_MAX_SCAN_RATE = 1001,

	/** Returns the maximum throughput to the \p infoValue argument for the specified digital direction.
	 * The direction is specified by setting the \p index argument to one of the ::DigitalDirection values. */
	DIO_INFO_MAX_THROUGHPUT = 1002
}DioInfoItemDbl;

/** Use with ulDIOGetConfig() and/or ulDIOSetConfig() as a \p configItem argument value to get the current configuration
 * of the specified digital port on the specified device.
 */
typedef enum
{
	/** Returns a bitmask value to \p configValue indicating the current direction of all bits
	 * in the specified port. A 0 indicates all bits are set for input, and non-zero value
	 * indicates the bit in the equivalent bitmask location is set for output.\n For example, a value of 3 indicates
	 * bits 0 and 1 are output, and any other bits in the port are input.
	 */
	DIO_CFG_PORT_DIRECTION_MASK = 1,

	/** Writes a value to the specified port number. This allows writing a value when the port is in
	 * input mode so that when the port is switched to output mode, the state of the bits is known.
	 */
	DIO_CFG_PORT_INITIAL_OUTPUT_VAL = 2,

	/** Returns or writes the low-pass filter setting. A 0 indicates that the filter is disabled for
	 * the corresponding bit.
	 */
	DIO_CFG_PORT_ISO_FILTER_MASK = 3,

	/** Returns the port logic. A 0 indicates non-invert mode, and a non-zero value indicates output inverted. */
	DIO_CFG_PORT_LOGIC = 4
}DioConfigItem;

/** Use with ulCtrGetInfo() to obtain information about the counter subsystem for the specified device
 * as an \p infoItem argument value.
 */
typedef enum
{
	/** Returns the total number of counters to the \p infoValue argument. Index is ignored. */
	CTR_INFO_NUM_CTRS = 1,

	/** Returns a bitmask of supported #CounterMeasurementType values of the counter specified
	 * by the \p index argument to the \p infoValue argument. */
	CTR_INFO_MEASUREMENT_TYPES = 2,

	/** Returns a bitmask of supported #CounterMeasurementMode values compatible with the #CounterMeasurementType
	 * specified by the \p index argument to the \p infoValue argument. */
	CTR_INFO_MEASUREMENT_MODES = 3,

	/** Returns a bitmask of supported #CounterRegisterType values to the \p infoValue argument.
	 * Index is ignored. */
	CTR_INFO_REGISTER_TYPES = 4,

	/** Returns the resolution to the \p infoValue argument. Index is ignored. */
	CTR_INFO_RESOLUTION = 5,

	/** Returns a zero or non-zero value to the \p infoValue argument.
	 * If non-zero, paced operations are supported. Index is ignored. */
	CTR_INFO_HAS_PACER = 6,

	/** Returns a bitmask of supported #ScanOption values to the \p infoValue argument.
	 * Index is ignored. */
	CTR_INFO_SCAN_OPTIONS = 7,

	/** Returns a bitmask of supported #TriggerType values to the \p infoValue argument.
	 * Index is ignored. */
	CTR_INFO_TRIG_TYPES = 8,

	/** Returns the FIFO size in bytes to the \p infoValue argument. Index is ignored. */
	CTR_INFO_FIFO_SIZE = 9
}CtrInfoItem;

/** Use with ulCtrGetInfoDbl() to obtain information about the counter subsystem for the specified device
 * as an \p infoItem argument value.
 */
typedef enum
{
	/** Returns the minimum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	CTR_INFO_MIN_SCAN_RATE = 1000,

	/** Returns the maximum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	CTR_INFO_MAX_SCAN_RATE = 1001,

	/** Returns the maximum throughput in samples per second to the \p infoValue argument. Index is ignored. */
	CTR_INFO_MAX_THROUGHPUT = 1002
}CtrInfoItemDbl;

/** Use with ulCtrSetConfig() and ulCtrGetConfig() to configure the Ctr subsystem. */
typedef enum
{
	/** Returns or writes a bitmask indicating the configuration of one or more counters. */
	CTR_CFG_REG = 1
}CtrConfigItem;

/** Use with ulTmrGetInfo() to obtain information about the timer subsystem for the specified device as an \p infoItem argument value. */
typedef enum
{
	/** Returns the total number of timers to the \p infoValue argument. Index is ignored. */
	TMR_INFO_NUM_TMRS = 1,

	/** Returns a #TimerType value to the \p infoValue argument, indicating the type of the timer
	 * specified by the \p index argument value. Index should be less than the value obtained using ::TMR_INFO_NUM_TMRS. */
	TMR_INFO_TYPE = 2,
}TmrInfoItem;

/** Use with ulTmrGetInfoDbl() to obtain information about the timer subsystem for the specified device as an \p infoItem argument value. */
typedef enum
{
	/** Returns the minimum output frequency to the \p infoValue argument.
	 * Index is ignored. */
	TMR_INFO_MIN_FREQ = 1000,

	/** Returns the maximum output frequency to the \p infoValue argument.
	 * Index is ignored. */
	TMR_INFO_MAX_FREQ = 1001,
}TmrInfoItemDbl;

/** Use with ulDaqIGetInfo() to obtain information about the DAQ input subsystem for the specified device
 * as an \p infoItem argument value.
 */
typedef enum
{
	/** Returns a bitmask of supported #DaqInChanType values to the \p infoValue argument.
	 * Index is ignored. */
	DAQI_INFO_CHAN_TYPES = 1,

	/** Returns a bitmask of supported #ScanOption values to the \p infoValue argument.
	 * Index is ignored. */
	DAQI_INFO_SCAN_OPTIONS = 2,

	/** Returns a bitmask of supported #TriggerType values to the \p infoValue argument.
	 * Index is ignored. */
	DAQI_INFO_TRIG_TYPES = 3,

	/** Returns the FIFO size in bytes to the \p infoValue argument. Index is ignored. */
	DAQI_INFO_FIFO_SIZE = 4
}DaqIInfoItem;

/** Use with ulDaqIGetInfoDbl() to obtain information about the DAQ input subsystem for the specified device
 * as an \p infoItem argument value.
 */
typedef enum
{
	/** Returns the minimum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	DAQI_INFO_MIN_SCAN_RATE = 1000,

	/** Returns the maximum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	DAQI_INFO_MAX_SCAN_RATE = 1001,

	/** Returns the minimum throughput to the \p infoValue argument. Index is ignored. */
	DAQI_INFO_MAX_THROUGHPUT = 1002
}DaqIInfoItemDbl;

/** Use with ulDaqOGetInfo() to obtain information about the DAQ output subsystem for the specified device
  * as an \p infoItem argument value.
 */
typedef enum
{
	/** Returns a bitmask of supported #DaqOutChanType values to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_CHAN_TYPES = 1,

	/** Returns a bitmask of supported #ScanOption values to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_SCAN_OPTIONS = 2,

	/** Returns a bitmask of supported #TriggerType values to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_TRIG_TYPES = 3,

	/** Returns the FIFO size in bytes to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_FIFO_SIZE = 4
}DaqOInfoItem;

/** Use with ulDaqOGetInfoDbl() to obtain information about the DAQ output subsystem for the specified device
 * as an \p infoItem argument value.
 */
typedef enum
{
	/** Returns the minimum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_MIN_SCAN_RATE = 1000,

	/** Returns the maximum scan rate in samples per second to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_MAX_SCAN_RATE = 1001,

	/** Returns the maximum throughput in samples per second to the \p infoValue argument. Index is ignored. */
	DAQO_INFO_MAX_THROUGHPUT = 1002
}DaqOInfoItemDbl;

/** \brief A structure that defines the location and access properties of the physical memory of a device.
 *
 * The struct contains fields for the memory address, size, and access type.
*/
struct MemDescriptor
{
	/** The enumeration indicating the region of the memory. */
	MemRegion region;

	/** A numeric value that specifies the address of the memory; used with ulMemRead() and ulMemWrite(). */
	unsigned int address;

	/** A numeric value that specifies the size of the memory area at the specified address. */
	unsigned int size;

	/** A bitmask indicating the access rights to the memory at the specified address (read, write, or both). */
	MemAccessType accessTypes;

	/** Reserved for future use */
	char reserved[64];
};

/** A structure that defines the location and access properties of the physical memory of a device. */
typedef struct 	MemDescriptor MemDescriptor;

/**
 * \defgroup DeviceDiscovery Device Discovery
 * Manage the MCC devices that are available to the system.
 * @{
 */
/**
 * Get the list of MCC devices available to the system.
 * @param interfaceTypes the interface types to discover
 * @param daqDevDescriptors[] an array of DaqDeviceDescriptor structs, each of which contains fields specifying
 * information about the DAQ device.
 * @param numDescriptors the size of the array. If the size is not correct, the required size is returned.
 * @return The UL error code.
 */

UlError ulGetDaqDeviceInventory(DaqDeviceInterface interfaceTypes, DaqDeviceDescriptor daqDevDescriptors[], unsigned int* numDescriptors );

/**
 * Get the descriptor of the remote network DAQ device.
 * @param host the remote device host name or IP address
 * @param port the remote device port
 * @param ifcName network interface name to be used for communication with the DAQ device (e.g. eth0, wlan0, ...);
 * set to NULL to select the default network interface
 * @param daqDevDescriptor DaqDeviceDescriptor struct containing fields that describe the device
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for discovery operation to end.
 * @return The UL error code.
 */

UlError ulGetNetDaqDeviceDescriptor(const char* host, unsigned short port, const char* ifcName, DaqDeviceDescriptor* daqDevDescriptor, double timeout);

/**
 * Create a device object within the Universal Library for the DAQ device specified by the descriptor.
 * @param daqDevDescriptor DaqDeviceDescriptor struct containing fields that describe the device
 * @return The device handle.
 */
DaqDeviceHandle ulCreateDaqDevice(DaqDeviceDescriptor daqDevDescriptor);

/**
 * Get descriptor information for a device. Before calling this function, you must first run #ulCreateDaqDevice()
 * to retrieve the \p daqDeviceHandle argument.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param daqDeviceDescriptor a DaqDeviceDescriptor struct containing fields that describe the device
 * @return The UL error code.
 */
UlError ulGetDaqDeviceDescriptor(DaqDeviceHandle daqDeviceHandle, DaqDeviceDescriptor* daqDeviceDescriptor);

/**
 * Establish a connection to a physical DAQ device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulConnectDaqDevice(DaqDeviceHandle daqDeviceHandle);

/**
 * Disconnect from a device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulDisconnectDaqDevice(DaqDeviceHandle daqDeviceHandle);

/**
 * Remove a device from the Universal Library, and release all resources associated with that device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulReleaseDaqDevice(DaqDeviceHandle daqDeviceHandle);

/**
 * The connection status of a DAQ device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param connected the connection status; a non-zero value indicates connected
 * @return The UL error code.
 */
UlError ulIsDaqDeviceConnected(DaqDeviceHandle daqDeviceHandle, int* connected);

/**
 * Specifies connection code of a DAQ device. This function must be invoked before ulConnectDaqDevice().
 * @param daqDeviceHandle the handle to the DAQ device
 * @param code the connection code
 * @return The UL error code.
 */
UlError ulDaqDeviceConnectionCode(DaqDeviceHandle daqDeviceHandle, long long code);


/** @} */ 

/**
 * \ingroup Misc
 * Causes the LED on a DAQ device to flash.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param flashCount The number of flashes; set to 0 for a continuous flash until the next call with a non-zero value
 * @return The UL error code.
 */
UlError ulFlashLed(DaqDeviceHandle daqDeviceHandle, int flashCount);

/**
 * \defgroup AnalogInput Analog Input
 * Configure the analog input subsystem and acquire data.
 * @{
 */

/**
 * Returns the value read from an A/D channel.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param channel A/D channel number
 * @param inputMode A/D channel mode
 * @param range A/D range
 * @param flags bit mask that specifies whether to scale and/or calibrate the data
 * @param data A/D data value
 * @return The UL error code.
 */
UlError ulAIn(DaqDeviceHandle daqDeviceHandle, int channel, AiInputMode inputMode, Range range, AInFlag flags, double* data);

/**
 * Scans a range of A/D channels, and stores the samples in an array.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowChan first A/D channel in the scan
 * @param highChan last A/D channel in the scan
 * @param inputMode A/D channel mode
 * @param range A/D range
 * @param samplesPerChan the number of A/D samples to collect from each channel in the scan
 * @param rate A/D sample rate in samples per channel per second. Upon return, this value is set to the actual sample rate.
 * @param options bit mask that specifies A/D scan options
 * @param flags bit mask that specifies whether to scale and/or calibrate the data
 * @param data[] pointer to the buffer to receive the data array
 * @return The UL error code.
 */
UlError ulAInScan(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double* rate, ScanOption options, AInScanFlag flags, double data[]);

/**
 * Returns the status, count, and index of an A/D scan operation.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation (idle or running)
 * @param xferStatus a TransferStatus struct containing fields that return the current sample count, scan count, and buffer index
 * for the specified input scan operation
 * @return The UL error code.
 */
UlError ulAInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the analog input operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulAInScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the wait type
 * @param waitParam Reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code.
 */
UlError ulAInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/**
 * Loads the A/D queue of a specified device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param queue[] an array of AiQueueElement structs, each of which contains fields specifying channel, range, and mode
 * @param numElements the number of elements in the queue
 * @return The UL error code.
 */
UlError ulAInLoadQueue(DaqDeviceHandle daqDeviceHandle, AiQueueElement queue[], unsigned int numElements);

/**
 * Configures the trigger parameters that will be used when #ulAInScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOption.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type type of trigger
 * @param trigChan the trigger channel; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH,
 * ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING.
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE,
 * ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING.
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value. 
 * @param retriggerSampleCount the number of samples per trigger to acquire with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulAInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerSampleCount);

/**
 * Returns a temperature value read from an A/D channel.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param channel A/D channel number
 * @param scale temperature unit
 * @param flags reserved for future use
 * @param data temperature value; if an ::ERR_OPEN_CONNECTION error occurs, the value returned will be -9999.
 * @return The UL error code.
 */
UlError ulTIn(DaqDeviceHandle daqDeviceHandle, int channel, TempScale scale, TInFlag flags, double* data);

/**
 * Scans a range of A/D temperature channels, and stores the samples in an array.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowChan first A/D channel in the scan
 * @param highChan last A/D channel in the scan
 * @param scale temperature unit
 * @param flags reserved for future use
 * @param data[] a pointer to an array that stores the data; if an ::ERR_OPEN_CONNECTION error occurs,
 * the value will be -9999 in the array element associated with the channel causing the error.
 * @return The UL error code.
 */
UlError ulTInArray(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[]);

/** @}*/ 

/** 
 * \defgroup AnalogOutput Analog Output
  * Configure the analog output subsystem and generate data
 * @{
 */
 
/**
 * Writes the value of a D/A output.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param channel D/A channel number
 * @param range D/A range
 * @param flags bit mask that specifies whether to scale and/or calibrate the data
 * @param data the value to write
 * @return The UL error code.
 */
UlError ulAOut(DaqDeviceHandle daqDeviceHandle, int channel, Range range, AOutFlag flags, double data);

/**
 * Writes values to a range of D/A channels.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowChan first D/A channel
 * @param highChan last D/A channel
 * @param range D/A ranges
 * @param flags bit mask that specifies whether to scale and/or calibrate the data
 * @param data[] a pointer to an array that stores the data
 * @return The UL error code.
 */
UlError ulAOutArray(DaqDeviceHandle daqDeviceHandle,  int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]);

/**
 * Writes values to a range of D/A channels.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowChan first D/A channel in the scan
 * @param highChan last D/A channel in the scan
 * @param range D/A range
 * @param samplesPerChan the number of D/A samples to output
 * @param rate the sample rate in scans per second. Upon return, this value is set to the actual sample rate.
 * @param options bit mask that specifies D/A scan options
 * @param flags bit mask that specifies whether to scale and/or calibrate the data
 * @param data[] a pointer to an array that stores the data
 * @return The UL error code.
 */
UlError ulAOutScan(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, Range range, int samplesPerChan, double* rate, ScanOption options, AOutScanFlag flags, double data[]);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the wait type
 * @param waitParam Reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code. 
 */
UlError ulAOutScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/**
 * Returns the status, count, and index of a D/A scan operation.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation
 * @param xferStatus a TransferStatus struct containing fields that return the current sample count, scan count, and buffer index
 * for the specified output scan operation
 * @return The UL error code.
 */
UlError ulAOutScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the analog output operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulAOutScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Configures the trigger parameters that will be used when #ulAOutScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOption.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the trigger type
 * @param trigChan the trigger channel; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH,
 * ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING.
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE,
 * ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING.
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value.
 * @param retriggerSampleCount the number of samples to generate with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulAOutSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerSampleCount);

/** @}*/ 

/** 
 * \defgroup DigitalIO Digital I/O
  * Configure the digital I/O subsystem and acquire or generate data
 * @{
 */

/**
 * Configures a digital port as input or output.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the digital port; the port must be configurable
 * @param direction the port direction (input or output)
 * @return The UL error code.
 */
UlError ulDConfigPort(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, DigitalDirection direction);

/**
 * Configures a digital bit as input or output.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the digital port containing the bit to configure; the port must be configurable.
 * @param bitNum the bit number within the specified port
 * @param direction the bit direction (input or output)
 * @return The UL error code.
 */
UlError ulDConfigBit(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, int bitNum, DigitalDirection direction);

/**
 * Returns the value read from a digital port.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the port type
 * @param data the port value
 * @return The UL error code.
 */
UlError ulDIn(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, unsigned long long* data);

/**
 * Writes the specified value to a digital output port.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the digital port type
 * @param data the port value
 * @return The UL error code.
 */
UlError ulDOut(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, unsigned long long data);

/**
 * Reads the specified digital ports, and Returns the data in an array.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowPort the first port in the scan
 * @param highPort the last port in the scan
 * @param data input data array
 * @return The UL error code.
 */
UlError ulDInArray(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);

/**
 * Sets the values of the specified digital ports.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowPort the first port in the scan
 * @param highPort the last port in the scan
 * @param data output data array
 * @return The UL error code.
 */
UlError ulDOutArray(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[]);

/**
 * Returns the value of a digital bit.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the digital port containing the bit to be read
 * @param bitNum the bit number
 * @param bitValue the number of the bit within the port specified by \p portType
 * @return The UL error code.
 */
UlError ulDBitIn(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, int bitNum, unsigned int* bitValue);

/**
 * Writes a value to a digital bit.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the digital port containing the bit to be written
 * @param bitNum the number of the bit within the port specified by \p portType to be written
 * @param bitValue the bit value to write
 * @return The UL error code.
 */
UlError ulDBitOut(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, int bitNum, unsigned int bitValue);

/**
 * Reads a range of digital ports.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowPort the number of the first port in the scan
 * @param highPort the number of the last port in the scan
 * @param samplesPerPort the number of samples to read from each port
 * @param rate the number of times per second (Hz) to read the port. Upon return, this value is set to the actual scan rate.
 * @param options scan options
 * @param flags reserved for future use
 * @param data[] pointer to an array to receive the digital data
 * @return The UL error code.
 */
UlError ulDInScan(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double* rate, ScanOption options, DInScanFlag flags, unsigned long long data[]);

/**
 * The status of a digital scan operation.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation
 * @param xferStatus a TransferStatus struct containing fields that return the current sample count, scan count, and buffer index
 * for the specified input scan operation
 * @return The UL error code.
 */
UlError ulDInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the digital input operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulDInScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the wait type
 * @param waitParam Reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code.
 */
UlError ulDInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/**
 * Configures the trigger parameters that will be used when #ulDInScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOption.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the trigger type
 * @param trigChan the trigger channel; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH,
 * ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING.
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE,
 * ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING.
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value.
 * @param retriggerSampleCount the number of samples to acquire with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulDInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerSampleCount);

/** Writes data to a range of digital ports.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowPort number of the first port in the scan
 * @param highPort number of the last port in the scan
 * @param samplesPerPort the number of samples per port to write
 * @param rate the number of times per second to write to each port (scan rate). Upon return, this value is set to the actual scan rate.
 * @param options scan options
 * @param flags reserved for future use
 * @param data[] a pointer to an array to receive the digital data
 * @return The UL error code.
 */
UlError ulDOutScan(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double* rate, ScanOption options, DOutScanFlag flags, unsigned long long data[]);

/**
 * Returns the status of the digital output operation
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation
 * @param xferStatus a TransferStatus struct containing fields that return the current sample count, scan count, and buffer index for the specified output scan operation
 * @return The UL error code.
 */
UlError ulDOutScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the digital output operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulDOutScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the wait type
 * @param waitParam Reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code.
 */
UlError ulDOutScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/**
 * Configures the trigger parameters that will be used when ulDOutScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOption.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the trigger type
 * @param trigChan the trigger channel; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH,
 * ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE,
 * ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value.
 * @param retriggerSampleCount the number of samples per trigger to acquire with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulDOutSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerSampleCount);

/**
 * Clears the alarm state for specified bits when alarms are configured to latch. Once the alarm condition is resolved, this function can be used to clear the latched alarm.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param portType the port containing the bit(s) to clear; the specified port must be configurable for alarm output.
 * @param mask bit mask that specifies which bits to clear; set to all 1s to clear all bits in the port.
 * @return The UL error code.
 */
UlError ulDClearAlarm(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, unsigned long long mask);

/** @}*/ 

/** 
 * \defgroup CounterInput Counter Input
 * Configure the counter input subsystem and acquire data
 * @{
 */
 
/**
 * Reads the value of a count register. Use ulCRead() to read any available register type (count, load, max limit, or min limit).
 * @param daqDeviceHandle the handle to the DAQ device
 * @param counterNum the counter number
 * @param data the data value
 * @return The UL error code.
 */
UlError ulCIn(DaqDeviceHandle daqDeviceHandle, int counterNum, unsigned long long* data);

/**
 * Reads the value of the specified counter register.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param counterNum the counter number
 * @param regType the register type
 * @param data the data value
 * @return The UL error code.
 */
UlError ulCRead(DaqDeviceHandle daqDeviceHandle, int counterNum, CounterRegisterType regType, unsigned long long* data);

/**
 * Loads a value into the specified counter register.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param counterNum the counter number
 * @param registerType the register type
 * @param loadValue the load value
 * @return The UL error code.
 */
UlError ulCLoad(DaqDeviceHandle daqDeviceHandle, int counterNum, CounterRegisterType registerType, unsigned long long loadValue);

/**
 * Sets the count of the specified counter to 0.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param counterNum the counter number
 * @return The UL error code.
 */
UlError ulCClear(DaqDeviceHandle daqDeviceHandle, int counterNum);

/**
 * Configures a counter channel, for counters with programmable types.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param counterNum the counter number
 * @param type the type of counter measurement
 * @param mode the counter mode
 * @param edgeDetection sets the active edge of the counter input signal to positive or negative
 * @param tickSize bit mask that specifies the counter resolution for measurement modes such as period, pulse width, and timing
 * @param debounceMode bit mask that specifies the counter debounce mode
 * @param debounceTime bit mask that specifies the counter debounce time
 * @param flags bit mask that specifies counting properties such as resolution and clear on scan start
 * @return The UL error code.
 */
UlError ulCConfigScan(DaqDeviceHandle daqDeviceHandle, int counterNum, CounterMeasurementType type,  CounterMeasurementMode mode,
					  CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
					  CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flags);

/**
 * Reads a range of counter channels.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param lowCounterNum the first channel of the scan
 * @param highCounterNum the last channel of the scan
 * @param samplesPerCounter the number of samples per counter to read
 * @param rate the rate in samples per second per counter. Upon return, this value is set to the actual sample rate.
 * @param options scan options
 * @param flags bit mask that specifies the counter scan option
 * @param data[] pointer to the buffer to receive the data
 * @return The UL error code.
 */
UlError ulCInScan(DaqDeviceHandle daqDeviceHandle, int lowCounterNum, int highCounterNum, int samplesPerCounter, double* rate, ScanOption options, CInScanFlag flags, unsigned long long data[]);

/**
 * Configures the trigger parameters that will be used when ulCInScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOption.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the trigger type
 * @param trigChan the trigger channel; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE, ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH,
 * ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE,
 * ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value.
 * @param retriggerSampleCount the number of samples to acquire with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulCInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerSampleCount);

/**
 * Returns the status of a counter input operation.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation
 * @param xferStatus a TransferStatus struct containing fields that return the current sample count, scan count, and buffer index for the
 * specified input scan operation
 * @return The UL error code.
 */
UlError ulCInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the counter input operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulCInScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the wait type
 * @param waitParam Reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code.
 */
UlError ulCInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/** @}*/ 

/** 
 * \defgroup TimerOutput Timer Output
 * Configure the timer output subsystem
 * @{
 */

/**
 * Starts a timer to generate digital pulses at a specified frequency and duty cycle.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param timerNum the timer number
 * @param frequency frequency of the timer pulse output
 * @param dutyCycle duty cycle of the timer pulse output
 * @param pulseCount the number of pulses to generate; set to 0 for continuous pulse output
 * @param initialDelay the amount of time in seconds to wait before the first pulse is generated at the timer output
 * @param idleState the idle state (high or low)
 * @param options pulse out options
 * @return The UL error code.
 */
UlError ulTmrPulseOutStart(DaqDeviceHandle daqDeviceHandle, int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options);

/**
 * Stops a timer output.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param timerNum the timer number to stop
 * @return The UL error code.
 */
UlError ulTmrPulseOutStop(DaqDeviceHandle daqDeviceHandle, int timerNum);

/**
 * The status of the timer output operation, if supported
 * @param daqDeviceHandle the handle to the DAQ device
 * @param timerNum the timer number
 * @param status the status of the background operation
 * @return The UL error code.
 */
UlError ulTmrPulseOutStatus(DaqDeviceHandle daqDeviceHandle, int timerNum, TmrStatus* status);

/**
 * Configures the trigger parameters that will be used when ulTmrPulseOutStart() is called with the ::PO_RETRIGGER or ::PO_EXTTRIGGER PulseOutOption, when supported.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the digital trigger type
 * @param trigChan ignored
 * @param level ignored
 * @param variance ignored
 * @param retriggerSampleCount ignored
 * @return The UL error code.
 */
UlError ulTmrSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerSampleCount);

/** @}*/ 

/** 
 * \defgroup SyncIo Synchronous device I/O
 * Configure the DAQ I/O subsystem to acquire or generate data
 * @{
 */

/**
 * Allows scanning of multiple input subsystems, such as analog, digital, counter, and stores the samples in an array.
 * This method only works with devices that support synchronous input.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param chanDescriptors[] an array of DaqInChanDescriptor structs with fields for channel number, type, and range
 * that specifies the order and parameters for each channel in the scan
 * @param numChans the number of channels in the scan
 * @param samplesPerChan the number of samples to collect from each channel in the scan
 * @param rate sample rate in samples per channel. Upon return, this value is set to the actual sample rate.
 * @param options scan options
 * @param flags bit mask that specifies whether to scale data, calibrate data, or clear the counter at the start of each scan.
 * @param data[] pointer to the buffer that receives the data
 * @return The UL error code.
 */
UlError ulDaqInScan(DaqDeviceHandle daqDeviceHandle, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double* rate, ScanOption options, DaqInScanFlag flags, double data[]);

/**
 * Returns the status of a synchronous input operation.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation
 * @param xferStatus TransferStatus struct containing fields that return the current sample count, scan count, and buffer index
 * for the specified input scan operation
 * @return The UL error code.
 */
UlError ulDaqInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the synchronous input operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulDaqInScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the wait type
 * @param waitParam reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code.
 */
UlError ulDaqInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/**
 * Configures the trigger parameters that will be used when ulDaqInScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOption.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the trigger type
 * @param trigChanDescriptor the DaqInChanDescriptor struct with fields for channel number, type, and range that defines the trigger input
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE,
 * ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value.
 * @param retriggerSampleCount the number of samples to acquire with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulDaqInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, DaqInChanDescriptor trigChanDescriptor, double level, double variance, unsigned int retriggerSampleCount);

/**
 * Outputs values synchronously to analog output channels and digital output ports.
 * This function only works with devices that support synchronous output.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param chanDescriptors[] an array of DaqOutChanDescriptor structs containing fields for channel number, type, and range
 * that specify the order and parameters for each channel in the scan
 * @param numChans the number of channels in the scan
 * @param samplesPerChan the number of samples per channel
 * @param rate the sample rate in scans per second. Upon return, this value is set to the actual rate.
 * @param options scan options
 * @param flags bit mask that specifies whether to scale and/or calibrate data for any analog channels in the scan
 * @param data[] buffer that holds the data that will be transferred to the device
 * @return The UL error code.
 */
UlError ulDaqOutScan(DaqDeviceHandle daqDeviceHandle, DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double* rate, ScanOption options, DaqOutScanFlag flags, double data[]);

/**
 * Returns the status of a synchronous output operation.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param status the status of the background operation
 * @param xferStatus TransferStatus struct containing fields that return the current sample count, scan count, and buffer index
 * for the specified output scan operation
 * @return The UL error code.
 */
UlError ulDaqOutScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus);

/**
 * Stops the synchronous output operation currently running.
 * @param daqDeviceHandle the handle to the DAQ device
 * @return The UL error code.
 */
UlError ulDaqOutScanStop(DaqDeviceHandle daqDeviceHandle);

/**
 * Returns when the scan operation completes on the specified device, or the time specified by the \p timeout argument elapses.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param waitType the type of wait
 * @param waitParam reserved for future use
 * @param timeout the timeout value in seconds (s); set to -1 to wait indefinitely for the scan operation to end.
 * @return The UL error code.
 */
UlError ulDaqOutScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout);

/**
 * Configures the trigger parameters that will be used when ulDaqOutScan() is called with the ::SO_RETRIGGER or ::SO_EXTTRIGGER ScanOptions.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param type the trigger type
 * @param trigChanDescriptor a DaqInChanDescriptor struct containing fields for channel number, type, and range that defines the trigger input
 * @param level the level at or around which the trigger event should be detected; ignored if \p type is set to ::TRIG_POS_EDGE, ::TRIG_NEG_EDGE,
 * ::TRIG_HIGH, ::TRIG_LOW, ::GATE_HIGH, ::GATE_LOW, ::TRIG_RISING, or ::TRIG_FALLING
 * @param variance the degree to which the input signal can vary relative to the \p level parameter; ignored for all types where \p level is ignored.
 * For pattern triggering, this argument serves as the mask value.
 * @param retriggerSampleCount the number of samples to acquire with each trigger event; ignored unless the ::SO_RETRIGGER ScanOption
 * is set for the scan.
 * @return The UL error code.
 */
UlError ulDaqOutSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, DaqInChanDescriptor trigChanDescriptor, double level, double variance, unsigned int retriggerSampleCount);

/** @}*/ 

/** 
 * \defgroup Misc Miscellaneous
 * Miscellaneous functions
 * @{
 */
 
/**
 * Binds one or more event conditions to a DaqEventCallback function.
 * Upon detection of an event condition, DaqEventCallback is invoked.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param eventTypes a bitmask containing event conditions that can be OR'd together
 * @param eventParameter additional data that specifies an event condition, such as the number of data points at which to invoke ::DE_ON_DATA_AVAILABLE
 * @param eventCallbackFunction the pointer to the user-defined callback function to handle event conditions.
 * @param userData the pointer to the data that will be passed to the callback function
 * @return The UL error code.
 */
UlError ulEnableEvent(DaqDeviceHandle daqDeviceHandle, DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCallbackFunction, void* userData);

/**
 * Disables one or more event conditions, and disconnects their user-defined handlers.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param eventTypes a bitmask containing event conditions that can be OR'd together; OR all event types to disable all events
 * @return The UL error code.
 */
UlError ulDisableEvent(DaqDeviceHandle daqDeviceHandle, DaqEventType eventTypes);

/**
 * Reads a value read from a specified region in memory; use with ulMemGetInfo() to retrieve information about the memory region on a DAQ device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param memRegion memory region
 * @param address memory address
 * @param buffer pointer to the buffer where the value read will be stored
 * @param count the size of the buffer that the user made available to store the value returned
 * @return The UL error code.
 */
UlError ulMemRead(DaqDeviceHandle daqDeviceHandle, MemRegion memRegion, unsigned int address, unsigned char* buffer, unsigned int count);

/**
 * Writes a value to a specified region in memory.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param memRegion memory region
 * @param address memory address
 * @param buffer pointer to the buffer provided by the user containing the value to write
 * @param count number or data points to read; must be equal to or less than the size of the buffer created
 * by the user containing the value to write
 * @return The UL error code.
 */
UlError ulMemWrite(DaqDeviceHandle daqDeviceHandle, MemRegion memRegion, unsigned int address, unsigned char* buffer, unsigned int count);

/**
 * Returns the error message associated with an error code.
 * @param errCode the error code returned from a Universal Library function to translate into an error message
 * @param errMsg the error message associated with the code provided is returned here.
 * The constant ::ERR_MSG_LEN is set to a value such that the buffer is large enough to contain the longest error message.
 * @return The UL error code.
 */
UlError ulGetErrMsg(UlError errCode, char errMsg[ERR_MSG_LEN]);

/** @}*/ 

/** 
 * \defgroup DeviceInfo Device Information
 * Retrieve device information
 * @{
 */

#ifndef doxy_skip
/**
 * Use with #UlInfoItemStr to retrieve the library information as a null-terminated string.
 * @param infoItem the information to read from the device
 * @param index either ignored or an index into the infoStr
 * @param infoStr pointer to the buffer where the information string is copied
 * @param maxConfigLen pointer to the value holding the maximum number of bytes to be read from the device into configStr
 * @return The UL error code.
 * */
UlError ulGetInfoStr(UlInfoItemStr infoItem, unsigned int index, char* infoStr, unsigned int* maxConfigLen);


/**
 * Use with UlConfigItem to change the library configuration options at runtime.
 * @param configItem the type of information to write to the device
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 * */
UlError ulSetConfig(UlConfigItem configItem, unsigned int index, long long configValue);


/**
 * Returns a configuration option set for the library.<br>Use ulSetConfig() to change configuration options.
 * @param configItem the configuration item to retrieve
 * @param index the index into the \p configItem
 * @param configValue the specified configuration value is returned to this variable
 * @return The UL error code. 
 */
UlError ulGetConfig(UlConfigItem configItem, unsigned int index, long long* configValue);
#endif /* doxy_skip */

/**
 * Use with #DevInfoItem to retrieve information about the device subsystem to determine which subsystem types are supported for the device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the subsystem information to retrieve
 * @param index ignored
 * @param infoValue the subsystem information is returned to this variable
 * @return The UL error code.
 */
UlError ulDevGetInfo(DaqDeviceHandle daqDeviceHandle, DevInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * Use with #DevConfigItem to set configuration options at runtime.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to set
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulDevSetConfig(DaqDeviceHandle daqDeviceHandle, DevConfigItem configItem, unsigned int index, long long configValue);

/**
 * Use with #DevConfigItem to retrieve the current configuration options set for a device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve from the device
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulDevGetConfig(DaqDeviceHandle daqDeviceHandle, DevConfigItem configItem, unsigned int index, long long* configValue);

/**
 * Use with #DevConfigItemStr to retrieve the current configuration as a null-terminated string.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve from the device
 * @param index specifies the version type to return as an index into \p configItem (::DevVersionType)
 * @param configStr pointer to the buffer where the configuration string is copied
 * @param maxConfigLen pointer to the value specifying the size of \p configStr made available by the user;
 * returns the number of chars written to \p configStr.
 * @return The UL error code.
 */
UlError ulDevGetConfigStr(DaqDeviceHandle daqDeviceHandle, DevConfigItemStr configItem, unsigned int index, char* configStr, unsigned int* maxConfigLen);

/** @}*/ 

/**
 * \ingroup AnalogInput
 * Use with #AiInfoItem to retrieve information about the AI subsystem.
 * @param daqDeviceHandle the device the handle to the DAQ device
 * @param infoItem the analog input information to retrieve
 * @param index either ignored or an index into the \p infoValue, depending on the value of the \p infoItem argument.
 * \p index can also the specify channel mode or channel type for some AiInfoItems (::AI_INFO_NUM_CHANS_BY_MODE,
 * ::AI_INFO_NUM_CHANS_BY_TYPE, and ::AI_INFO_MAX_QUEUE_LENGTH_BY_MODE)
 * @param infoValue the AI information is returned to this variable
 * @return The UL error code.
 */
UlError ulAIGetInfo(DaqDeviceHandle daqDeviceHandle, AiInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup AnalogInput
 * Use with #AiInfoItemDbl to retrieve information about the AI subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the analog input information to retrieve
 * @param index either ignored or an index into the \p infoValue. This argument can also specify the channel mode
 * or channel type for some #AiInfoItem values (::AI_INFO_NUM_CHANS_BY_MODE, ::AI_INFO_NUM_CHANS_BY_TYPE, and ::AI_INFO_MAX_QUEUE_LENGTH_BY_MODE).
 * @param infoValue the AI information is returned to this variable
 * @return The UL error code.
 */
UlError ulAIGetInfoDbl(DaqDeviceHandle daqDeviceHandle, AiInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup AnalogInput
 * Use with #AiConfigItem to set configuration options at runtime.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to set
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulAISetConfig(DaqDeviceHandle daqDeviceHandle, AiConfigItem configItem, unsigned int index, long long configValue);

/**
 * \ingroup AnalogInput
 * Use with #AiConfigItem to retrieve configuration options set for a device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index either ignored or an index into the \p configValue
 * @param configValue the specified configuration value is returned to this variable
 * @return The UL error code.
 */
UlError ulAIGetConfig(DaqDeviceHandle daqDeviceHandle, AiConfigItem configItem, unsigned int index, long long* configValue);

/**
 * \ingroup AnalogInput
 * Use with #AiConfigItemDbl to set configuration options at runtime.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to set
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulAISetConfigDbl(DaqDeviceHandle daqDeviceHandle, AiConfigItemDbl configItem, unsigned int index, double configValue);

/**
 * \ingroup AnalogInput
 * Use with #AiConfigItem to retrieve configuration options set for a device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index either ignored or an index into the \p configValue
 * @param configValue the specified configuration value is returned to this variable
 * @return The UL error code.
 */
UlError ulAIGetConfigDbl(DaqDeviceHandle daqDeviceHandle, AiConfigItemDbl configItem, unsigned int index, double* configValue);

/**
 * \ingroup AnalogInput
 * Use with #AiConfigItemStr to retrieve configuration options as a null-terminated string.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index either ignored or an index into the configStr
 * @param configStr pointer to the buffer where the configuration string is copied
 * @param maxConfigLen pointer to the value holding the maximum number of bytes to be read from the device into configStr
 * @return The UL error code.
 */
UlError ulAIGetConfigStr(DaqDeviceHandle daqDeviceHandle, AiConfigItemStr configItem, unsigned int index, char* configStr, unsigned int* maxConfigLen);

/** 
 * \ingroup AnalogOutput
 * Use with #AoInfoItem to retrieve information about the AO subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the analog output information to retrieve
 * @param index either ignored or an index into the \p infoValue, depending on the value of the \p infoItem argument
 * @param infoValue the AO information is returned to this variable
 * @return The UL error code.
 */
UlError ulAOGetInfo(DaqDeviceHandle daqDeviceHandle, AoInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup AnalogOutput
 * Use with #AoInfoItemDbl to retrieve information about the AO subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the analog output information to retrieve
 * @param index either ignored or an index into the \p infoValue
 * @param infoValue the AO information is returned to this variable
 * @return The UL error code.
 */
UlError ulAOGetInfoDbl(DaqDeviceHandle daqDeviceHandle, AoInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup AnalogOutput
 * Use with #AoConfigItem to set configuration options at runtime.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to set
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulAOSetConfig(DaqDeviceHandle daqDeviceHandle, AoConfigItem configItem, unsigned int index, long long configValue);

/**
 * \ingroup AnalogOutput
 * Use with #AoConfigItem to retrieve configuration options set for a device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index either ignored or an index into the \p configValue
 * @param configValue the specified configuration value is returned to this variable
 * @return The UL error code.
 */
UlError ulAOGetConfig(DaqDeviceHandle daqDeviceHandle, AoConfigItem configItem, unsigned int index, long long* configValue);

/**
 * \ingroup DigitalIO
 * Use with #DioInfoItem to retrieve information about the DIO subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the digital I/O information to retrieve
 * @param index an index into the \p infoValue, depending on the value of the \p infoItem argument; usually specifies port direction
 * (::DD_INPUT or ::DD_OUTPUT); ignored when \p infoItem is set to ::DIO_INFO_NUM_PORTS and ::DIO_INFO_HAS_PACER
 * @param infoValue the DIO information is returned to this variable
 * @return The UL error code.
 */
UlError ulDIOGetInfo(DaqDeviceHandle daqDeviceHandle, DioInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup DigitalIO
 * Use with #DioInfoItemDbl to retrieve information about the DIO subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the digital I/O information to retrieve
 * @param index the port direction (::DD_INPUT or ::DD_OUTPUT)
 * @param infoValue the DIO information is returned to this variable
 * @return The UL error code.
 */
UlError ulDIOGetInfoDbl(DaqDeviceHandle daqDeviceHandle, DioInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup DigitalIO
 * Use with #DioConfigItem to retrieve information about the DIO subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index the port index
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulDIOSetConfig(DaqDeviceHandle daqDeviceHandle, DioConfigItem configItem, unsigned int index, long long configValue);

/**
 * \ingroup DigitalIO
 * Use with #DioConfigItem to retrieve the current configuration about the DIO subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index the port index
 * @param configValue the specified configuration value is returned to this variable
 * @return The UL error code.
 */
UlError ulDIOGetConfig(DaqDeviceHandle daqDeviceHandle, DioConfigItem configItem, unsigned int index, long long* configValue);

/**
 * \ingroup CounterInput
 * Use with #CtrInfoItem to retrieve information about the counter subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the counter information to retrieve
 * @param index either ignored or an index into the \p infoValue, depending on the value of the \p infoItem argument
 * @param infoValue the counter information is returned to this variable
 * @return The UL error code.
 */
UlError ulCtrGetInfo(DaqDeviceHandle daqDeviceHandle, CtrInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup CounterInput
 * Use with #CtrInfoItemDbl to retrieve information about the counter subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the counter information to retrieve
 * @param index either ignored or an index into the \p infoValue, depending on the value of the \p infoItem argument
 * @param infoValue the counter information is returned to this variable
 * @return The UL error code.
 */
UlError ulCtrGetInfoDbl(DaqDeviceHandle daqDeviceHandle, CtrInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup CounterInput
 * Use with #CtrConfigItem to retrieve configuration options set for a device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to retrieve
 * @param index either ignored or an index into the \p configValue
 * @param configValue the specified configuration value is returned to this variable
 * @return The UL error code.
 */
UlError ulCtrGetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long* configValue);

/**
 * \ingroup CounterInput
 * Use with #CtrConfigItem to set configuration options at runtime.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param configItem the configuration item to set
 * @param index either ignored or an index into the \p configValue
 * @param configValue the value to set the specified configuration item to
 * @return The UL error code.
 */
UlError ulCtrSetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long configValue);

/**
 * \ingroup TimerOutput
 * Use with #TmrInfoItem to retrieve information about the timer subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the timer information to retrieve
 * @param index either ignored or an index into the \p infoValue, depending on the value of the \p infoItem argument
 * @param infoValue the timer information is returned to this variable
 * @return The UL error code.
 */
UlError ulTmrGetInfo(DaqDeviceHandle daqDeviceHandle, TmrInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup TimerOutput
 * Use with #TmrInfoItemDbl to retrieve information about the timer subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the timer information to retrieve
 * @param index either ignored or an index into the \p infoValue, depending on the value of the \p infoItem argument
 * @param infoValue the timer information is returned to this variable
 * @return The UL error code.
 */
UlError ulTmrGetInfoDbl(DaqDeviceHandle daqDeviceHandle, TmrInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup SyncIo
 * Use with #DaqIInfoItem to retrieve information about the synchronous input subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the synchronous input information to retrieve
 * @param index ignored for all info types available to this function
 * @param infoValue the synchronous input information is returned to this variable
 * @return The UL error code.
 */
UlError ulDaqIGetInfo(DaqDeviceHandle daqDeviceHandle, DaqIInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup SyncIo
 * Use with #DaqIInfoItemDbl to retrieve information about the synchronous input subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the synchronous input information to retrieve
 * @param index ignored for all info types available to this function
 * @param infoValue the synchronous input information is returned to this variable
 * @return The UL error code.
 */
UlError ulDaqIGetInfoDbl(DaqDeviceHandle daqDeviceHandle, DaqIInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup SyncIo
 * Use with #DaqOInfoItem to retrieve information about the synchronous output subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the synchronous output information to retrieve
 * @param index ignored for all info types available to this function
 * @param infoValue the synchronous output information is returned to this variable
 * @return The UL error code.
 */
UlError ulDaqOGetInfo(DaqDeviceHandle daqDeviceHandle, DaqOInfoItem infoItem, unsigned int index, long long* infoValue);

/**
 * \ingroup SyncIo
 * Use with #DaqOInfoItemDbl to retrieve information about the synchronous output subsystem.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param infoItem the synchronous output information to retrieve
 * @param index ignored for all info types available to this function
 * @param infoValue the synchronous output information is returned to this variable
 * @return The UL error code.
 */
UlError ulDaqOGetInfoDbl(DaqDeviceHandle daqDeviceHandle, DaqOInfoItemDbl infoItem, unsigned int index, double* infoValue);

/**
 * \ingroup DeviceInfo
 * Use with MemDescriptor to retrieve information about the memory region on a DAQ device.
 * @param daqDeviceHandle the handle to the DAQ device
 * @param memRegion the memory region
 * @param memDescriptor a MemDescriptor struct containing fields where information is returned, such as memory type and access
 * @return The UL error code.
 */

UlError ulMemGetInfo(DaqDeviceHandle daqDeviceHandle, MemRegion memRegion, MemDescriptor* memDescriptor);

#ifndef doxy_skip
/**
 * Create a device object within the Universal Library for the DAQ device specified by the descriptor. This function intended to be
 * used by languages that don't have capability to pass structures by value. i.e LabVIEW, VB, ...
 * @param daqDevDescriptor DaqDeviceDescriptor struct containing fields that describe the device
 * @return The device handle.
 */
DaqDeviceHandle ulCreateDaqDevicePtr(DaqDeviceDescriptor* daqDevDescriptor);

#endif /* doxy_skip */

#ifdef __cplusplus
}
#endif

#endif /* UL_DAQ_H_ */
