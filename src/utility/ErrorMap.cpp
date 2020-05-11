/*
 * ErrorMap.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "ErrorMap.h"
#include "../uldaq.h"

namespace ul
{

ErrorMap::ErrorMap()
{
	mErrMap.insert(std::pair<int, std::string>(ERR_NO_ERROR, "No error has occurred"));  // 0
	mErrMap.insert(std::pair<int, std::string>(ERR_UNHANDLED_EXCEPTION, "Unhandled internal exception"));  // 1
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DEV_HANDLE, "Invalid device handle"));  // 2
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DEV_TYPE, "This function cannot be used with this device")); // 3
	mErrMap.insert(std::pair<int, std::string>(ERR_USB_DEV_NO_PERMISSION, "Insufficient permission to access this device")); // 4
	mErrMap.insert(std::pair<int, std::string>(ERR_USB_INTERFACE_CLAIMED, "USB interface is already claimed")); // 5
	mErrMap.insert(std::pair<int, std::string>(ERR_DEV_NOT_FOUND, "Device not found")); // 6
	mErrMap.insert(std::pair<int, std::string>(ERR_DEV_NOT_CONNECTED, "Device not connected or connection lost"));  // 7
	mErrMap.insert(std::pair<int, std::string>(ERR_DEAD_DEV, "Device no longer responding"));  // 8
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_BUFFER_SIZE, "Buffer too small for operation"));  // 9
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_BUFFER, "Invalid buffer"));  // 10
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_MEM_TYPE, "Invalid memory type"));  // 11
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_MEM_REGION, "Invalid memory region"));  // 12
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_RANGE, "Invalid range"));  // 13
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_AI_CHAN, "Invalid analog input channel specified"));  // 14
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_INPUT_MODE, "Invalid input mode specified"));  // 15
	mErrMap.insert(std::pair<int, std::string>(ERR_ALREADY_ACTIVE, "A background process is already in progress")); // 16
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_TRIG_TYPE, "Invalid trigger type specified")); // 17
	mErrMap.insert(std::pair<int, std::string>(ERR_OVERRUN, "FIFO overrun, data was not transferred from device fast enough")); // 18
	mErrMap.insert(std::pair<int, std::string>(ERR_UNDERRUN, "FIFO underrun, data was not transferred to device fast enough")); // 19
	mErrMap.insert(std::pair<int, std::string>(ERR_TIMEDOUT, "Operation timed out")); // 20
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_OPTION, "Invalid option specified"));  // 21
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_RATE, "Invalid sampling rate specified"));  // 22
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_BURSTIO_COUNT, "Sample count cannot be greater than FIFO size for BURSTIO scans")); // 23
	mErrMap.insert(std::pair<int, std::string>(ERR_CONFIG_NOT_SUPPORTED, "Configuration not supported")); // 24
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CONFIG_VAL, "Invalid configuration value")); // 25
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_AI_CHAN_TYPE, "Invalid analog input channel type specified"));  // 26
	mErrMap.insert(std::pair<int, std::string>(ERR_ADC_OVERRUN, "ADC overrun occurred")); // 27
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_TC_TYPE, "Invalid thermocouple type specified"));  // 28
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_UNIT, "Invalid unit specified"));  // 29
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_QUEUE_SIZE, "Invalid queue size"));  // 30
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CONFIG_ITEM, "Invalid config item specified"));  // 31
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_INFO_ITEM, "Invalid info item specified"));  // 32
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_FLAG, "Invalid flag specified"));  // 33
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_SAMPLE_COUNT, "Invalid sample count specified"));  // 33
	mErrMap.insert(std::pair<int, std::string>(ERR_INTERNAL, "Internal error"));  // 35
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_COUPLING_MODE, "Invalid coupling mode"));  // 36
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_SENSOR_SENSITIVITY, "Invalid sensor sensitivity"));  // 37
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_IEPE_MODE, "Invalid IEPE mode"));  // 38
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_AI_CHAN_QUEUE, "Invalid channel queue specified"));  // 39
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_AI_GAIN_QUEUE, "Invalid gain queue specified"));  // 40
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_AI_MODE_QUEUE, "Invalid mode queue specified"));  // 41
	mErrMap.insert(std::pair<int, std::string>(ERR_FPGA_FILE_NOT_FOUND, "FPGA file not found"));  // 42
	mErrMap.insert(std::pair<int, std::string>(ERR_UNABLE_TO_READ_FPGA_FILE, "Unable to read FPGA file"));  // 43
	mErrMap.insert(std::pair<int, std::string>(ERR_NO_FPGA, "FPGA not loaded"));  // 44
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_ARG, "Invalid argument"));  // 45
	mErrMap.insert(std::pair<int, std::string>(ERR_MIN_SLOPE_VAL_REACHED, "Minimum slope value reached"));  // 46
	mErrMap.insert(std::pair<int, std::string>(ERR_MAX_SLOPE_VAL_REACHED, "Maximum slope value reached"));  // 47
	mErrMap.insert(std::pair<int, std::string>(ERR_MIN_OFFSET_VAL_REACHED, "Minimum offset value reached"));  // 48
	mErrMap.insert(std::pair<int, std::string>(ERR_MAX_OFFSET_VAL_REACHED, "Maximum offset value reached"));  // 49
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_PORT_TYPE, "Invalid port type specified"));  // 50
	mErrMap.insert(std::pair<int, std::string>(ERR_WRONG_DIG_CONFIG, "Digital I/O is configured incorrectly"));  // 51
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_BIT_NUM, "Invalid bit number"));  // 52
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_PORT_VAL, "Invalid port value specified"));  // 53
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_RETRIG_COUNT, "Invalid re-trigger count"));  // 54
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_AO_CHAN, "Invalid analog output channel specified"));  // 55
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DA_VAL, "Invalid D/A output value specified"));  // 56
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_TMR, "Invalid timer specified"));  // 57
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_FREQUENCY, "Invalid frequency specified"));  // 58
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DUTY_CYCLE, "Invalid duty cycle specified"));  // 59
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_INITIAL_DELAY, "Invalid initial delay specified"));  // 60
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CTR, "Invalid counter specified"));  // 61
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CTR_VAL, "Invalid counter value specified"));  // 62
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DAQI_CHAN_TYPE, "Invalid DAQ input channel type specified"));  // 63
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_NUM_CHANS, "Invalid number of channels specified"));  // 64
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CTR_REG, "Invalid counter register specified"));  // 65
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CTR_MEASURE_TYPE, "Invalid counter measurement type specified"));  // 66
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CTR_MEASURE_MODE, "Invalid counter measurement mode specified"));  // 67
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DEBOUNCE_TIME, "Invalid debounce time specified"));  // 68
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DEBOUNCE_MODE, "Invalid debounce mode specified"));  // 69
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_EDGE_DETECTION, "Invalid edge detection mode specified"));  // 70
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_TICK_SIZE, "Invalid tick size specified"));  // 71
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DAQO_CHAN_TYPE, "Invalid DAQ output channel type specified"));  // 72
	mErrMap.insert(std::pair<int, std::string>(ERR_NO_CONNECTION_ESTABLISHED, "No connection established"));  // 73
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_EVENT_TYPE, "Invalid event type specified"));  // 74
	mErrMap.insert(std::pair<int, std::string>(ERR_EVENT_ALREADY_ENABLED, "An event handler has already been enabled for this event type"));  // 75
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_EVENT_PARAMETER, "Invalid event parameter specified"));  // 76
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CALLBACK_FUCNTION, "Invalid callback function specified"));  // 77
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_MEM_ADDRESS, "Invalid memory address"));  // 78
	mErrMap.insert(std::pair<int, std::string>(ERR_MEM_ACCESS_DENIED, "Memory access denied"));  // 79
	mErrMap.insert(std::pair<int, std::string>(ERR_DEV_UNAVAILABLE, "Device is not available at time of request"));  // 80
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_RETRIG_TRIG_TYPE, "Re-trigger option is not supported for the specified trigger type"));  // 81
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DEV_VER, "This function cannot be used with this version of this device"));  // 82
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_DIG_OPERATION, "This digital operation is not supported on the specified port"));  // 83
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_PORT_INDEX, "Invalid digital port index specified"));  // 84
	mErrMap.insert(std::pair<int, std::string>(ERR_OPEN_CONNECTION, "Temperature input has open connection"));  // 85
	mErrMap.insert(std::pair<int, std::string>(ERR_DEV_NOT_READY, "Device is not ready to send data"));  // 86
	mErrMap.insert(std::pair<int, std::string>(ERR_PACER_OVERRUN, "Pacer overrun, external clock rate too fast"));  // 87
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_TRIG_CHANNEL, "Invalid trigger channel specified"));  // 88
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_TRIG_LEVEL, "Invalid trigger level specified"));  // 89
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CHAN_ORDER, "Invalid channel order"));  // 90
	mErrMap.insert(std::pair<int, std::string>(ERR_TEMP_OUT_OF_RANGE, "Temperature input is out of range"));  // 91
	mErrMap.insert(std::pair<int, std::string>(ERR_TRIG_THRESHOLD_OUT_OF_RANGE, "Trigger threshold is out of range"));  // 92
	mErrMap.insert(std::pair<int, std::string>(ERR_INCOMPATIBLE_FIRMWARE, "Incompatible firmware version, firmware update required"));  // 93

	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_NET_IFC, "Specified network interface is not available or disconnected"));   // 94
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_NET_HOST, "Invalid host specified"));  // 95
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_NET_PORT, "Invalid port specified"));  // 96
	mErrMap.insert(std::pair<int, std::string>(ERR_NET_IFC_UNAVAILABLE, "Network interface used to obtain the device descriptor not available or disconnected"));  // 97
	mErrMap.insert(std::pair<int, std::string>(ERR_NET_CONNECTION_FAILED, "Network connection failed"));  // 98

	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_CONNECTION_CODE, "Invalid connection code"));  // 99
	mErrMap.insert(std::pair<int, std::string>(ERR_CONNECTION_CODE_IGNORED, "Connection code ignored"));  // 100
	mErrMap.insert(std::pair<int, std::string>(ERR_NET_DEV_IN_USE, "Network device already in use"));  // 101
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_NET_FRAME, "Invalid network frame"));  // 102
	mErrMap.insert(std::pair<int, std::string>(ERR_NET_TIMEOUT, "Network device did not respond within expected time"));  // 103
	mErrMap.insert(std::pair<int, std::string>(ERR_DATA_SOCKET_CONNECTION_FAILED, "Data socket connection failed")); //104
	mErrMap.insert(std::pair<int, std::string>(ERR_PORT_USED_FOR_ALARM, "One or more bits on the specified port are used for alarm")); //105
	mErrMap.insert(std::pair<int, std::string>(ERR_BIT_USED_FOR_ALARM, "The specified bit is used for alarm")); //106
	mErrMap.insert(std::pair<int, std::string>(ERR_CMR_EXCEEDED, "Common-mode voltage range exceeded")); //107
	mErrMap.insert(std::pair<int, std::string>(ERR_NET_BUFFER_OVERRUN, "Network buffer overrun, data was not transferred from buffer fast enough")); //108
	mErrMap.insert(std::pair<int, std::string>(ERR_BAD_NET_BUFFER, "Invalid network buffer")); //109


}

std::string ErrorMap::getErrorMsg(int errNum)
{
	std::string msg;
	std::map<int, std::basic_string<char> >::iterator itr = mErrMap.find(errNum);

	if(itr != mErrMap.end())
		msg = mErrMap[errNum];
	else
		msg = "Error has no text";

	return msg;
}



} /* namespace ul */
