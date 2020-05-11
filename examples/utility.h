/*
	This file contains some of the functions used get the device and
	subsystem capabilities.  To assist in displaying values from
	enumerations, this file contains a group of functions to convert
	the enumerated values to strings.
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdio.h>
#include <unistd.h>
#include <memory.h>

#include "uldaq.h"



/****************************************************************************
 * Enum Conversion Functions
 ****************************************************************************/
void ConvertInputModeToString(AiInputMode inputMode, char* modeStr)
{
	switch(inputMode)
	{
	case(AI_DIFFERENTIAL):
		strcpy(modeStr, "AI_DIFFERENTIAL");
		break;
	case(AI_SINGLE_ENDED):
		strcpy(modeStr, "AI_SINGLE_ENDED");
		break;
	case(AI_PSEUDO_DIFFERENTIAL):
		strcpy(modeStr, "AI_PSEUDO_DIFFERENTIAL");
		break;
	}
}


void ConvertRangeToString(Range range, char* rangeStr)
{
	switch(range)
	{
	case(BIP60VOLTS):
		strcpy(rangeStr, "BIP60VOLTS");
		break;
	case(BIP30VOLTS):
		strcpy(rangeStr, "BIP30VOLTS");
		break;
	case(BIP15VOLTS):
		strcpy(rangeStr, "BIP15VOLTS");
		break;
	case(BIP20VOLTS):
		strcpy(rangeStr, "BIP20VOLTS");
		break;
	case(BIP10VOLTS):
		strcpy(rangeStr, "BIP10VOLTS");
		break;
	case(BIP5VOLTS):
		strcpy(rangeStr, "BIP5VOLTS");
		break;
	case(BIP4VOLTS):
		strcpy(rangeStr, "BIP4VOLTS");
		break;
	case(BIP2PT5VOLTS):
		strcpy(rangeStr, "BIP2PT5VOLTS");
		break;
	case(BIP2VOLTS):
		strcpy(rangeStr, "BIP2VOLTS");
		break;
	case(BIP1PT25VOLTS):
		strcpy(rangeStr, "BIP1PT25VOLTS");
		break;
	case(BIP1VOLTS):
		strcpy(rangeStr, "BIP1VOLTS");
		break;
	case(BIPPT625VOLTS):
		strcpy(rangeStr, "BIPPT625VOLTS");
		break;
	case(BIPPT5VOLTS):
		strcpy(rangeStr, "BIPPT5VOLTS");
		break;
	case(BIPPT25VOLTS):
		strcpy(rangeStr, "BIPPT25VOLTS");
		break;
	case(BIPPT125VOLTS):
		strcpy(rangeStr, "BIPPT125VOLTS");
		break;
	case(BIPPT2VOLTS):
		strcpy(rangeStr, "BIPPT2VOLTS");
		break;
	case(BIPPT1VOLTS):
		strcpy(rangeStr, "BIPPT1VOLTS");
		break;
	case(BIPPT078VOLTS):
		strcpy(rangeStr, "BIPPT078VOLTS");
		break;
	case(BIPPT05VOLTS):
		strcpy(rangeStr, "BIPPT05VOLTS");
		break;
	case(BIPPT01VOLTS):
		strcpy(rangeStr, "BIPPT01VOLTS");
		break;
	case(BIPPT005VOLTS):
		strcpy(rangeStr, "BIPPT005VOLTS");
		break;
	case(BIP3VOLTS):
		strcpy(rangeStr, "BIP3VOLTS");
		break;
	case(BIPPT312VOLTS):
		strcpy(rangeStr, "BIPPT312VOLTS");
		break;
	case(BIPPT156VOLTS):
		strcpy(rangeStr, "BIPPT156VOLTS");
		break;
	case(UNI60VOLTS):
		strcpy(rangeStr, "UNI60VOLTS");
		break;
	case(UNI30VOLTS):
		strcpy(rangeStr, "UNI30VOLTS");
		break;
	case(UNI15VOLTS):
		strcpy(rangeStr, "UNI15VOLTS");
		break;
	case(UNI20VOLTS):
		strcpy(rangeStr, "UNI20VOLTS");
		break;
	case(UNI10VOLTS):
		strcpy(rangeStr, "UNI10VOLTS");
		break;
	case(UNI5VOLTS):
		strcpy(rangeStr, "UNI5VOLTS");
		break;
	case(UNI4VOLTS):
		strcpy(rangeStr, "UNI4VOLTS");
		break;
	case(UNI2PT5VOLTS):
		strcpy(rangeStr, "UNI2PT5VOLTS");
		break;
	case(UNI2VOLTS):
		strcpy(rangeStr, "UNI2VOLTS");
		break;
	case(UNI1PT25VOLTS):
		strcpy(rangeStr, "UNI1PT25VOLTS");
		break;
	case(UNI1VOLTS):
		strcpy(rangeStr, "UNI1VOLTS");
		break;
	case(UNIPT625VOLTS):
		strcpy(rangeStr, "UNIPT625VOLTS");
		break;
	case(UNIPT5VOLTS):
		strcpy(rangeStr, "UNIPT5VOLTS");
		break;
	case(UNIPT25VOLTS):
		strcpy(rangeStr, "UNIPT25VOLTS");
		break;
	case(UNIPT125VOLTS):
		strcpy(rangeStr, "UNIPT125VOLTS");
		break;
	case(UNIPT2VOLTS):
		strcpy(rangeStr, "UNIPT2VOLTS");
		break;
	case(UNIPT1VOLTS):
		strcpy(rangeStr, "UNIPT1VOLTS");
		break;
	case(UNIPT078VOLTS):
		strcpy(rangeStr, "UNIPT078VOLTS");
		break;
	case(UNIPT05VOLTS):
		strcpy(rangeStr, "UNIPT05VOLTS");
		break;
	case(UNIPT01VOLTS):
		strcpy(rangeStr, "UNIPT01VOLTS");
		break;
	case(UNIPT005VOLTS):
		strcpy(rangeStr, "UNIPT005VOLTS");
		break;
	case(MA0TO20):
		strcpy(rangeStr, "MA0TO20");
		break;
	}
}


void ConvertRangeToMinMax(Range range, double* min, double* max)
{
	switch(range)
	{
	case(BIP60VOLTS):
		*min = -60.0;
		*max = 60.0;
		break;
	case(BIP30VOLTS):
		*min = -30.0;
		*max = 30.0;
		break;
	case(BIP15VOLTS):
		*min = -15.0;
		*max = 15.0;
		break;
	case(BIP20VOLTS):
		*min = -20.0;
		*max = 20.0;
		break;
	case(BIP10VOLTS):
		*min = -10.0;
		*max = 10.0;
		break;
	case(BIP5VOLTS):
		*min = -5.0;
		*max = 5.0;
		break;
	case(BIP4VOLTS):
		*min = -4.0;
		*max = 4.0;
		break;
	case(BIP2PT5VOLTS):
		*min = -2.5;
		*max = 2.5;
		break;
	case(BIP2VOLTS):
		*min = -2.0;
		*max = 2.0;
		break;
	case(BIP1PT25VOLTS):
		*min = -1.25;
		*max = 1.25;
		break;
	case(BIP1VOLTS):
		*min = -1.0;
		*max = 1.0;
		break;
	case(BIPPT625VOLTS):
		*min = -0.625;
		*max = 0.625;
		break;
	case(BIPPT5VOLTS):
		*min = -0.5;
		*max = 0.5;
		break;
	case(BIPPT25VOLTS):
		*min = -0.25;
		*max = 0.25;
		break;
	case(BIPPT125VOLTS):
		*min = -0.125;
		*max = 0.125;
		break;
	case(BIPPT2VOLTS):
		*min = -0.2;
		*max = 0.2;
		break;
	case(BIPPT1VOLTS):
		*min = -0.1;
		*max = 0.1;
		break;
	case(BIPPT078VOLTS):
		*min = -0.078;
		*max = 0.078;
		break;
	case(BIPPT05VOLTS):
		*min = -0.05;
		*max = 0.05;
		break;
	case(BIPPT01VOLTS):
		*min = -0.01;
		*max = 0.01;
		break;
	case(BIPPT005VOLTS):
		*min = -0.005;
		*max = 0.005;
		break;
	case(BIP3VOLTS):
		*min = -3.0;
		*max = 3.0;
		break;
	case(BIPPT312VOLTS):
		*min = -0.312;
		*max = 0.312;
		break;
	case(BIPPT156VOLTS):
		*min = -0.156;
		*max = 0.156;
		break;

	case(UNI60VOLTS):
		*min = 0.0;
		*max = 60.0;
		break;
	case(UNI30VOLTS):
		*min = 0.0;
		*max = 30.0;
		break;
	case(UNI15VOLTS):
		*min = 0.0;
		*max = 15.0;
		break;
	case(UNI20VOLTS):
		*min = 0.0;
		*max = 20.0;
		break;
	case(UNI10VOLTS):
		*min = 0.0;
		*max = 10.0;
		break;
	case(UNI5VOLTS):
		*min = 0.0;
		*max = 5.0;
		break;
	case(UNI4VOLTS):
		*min = 0.0;
		*max = 4.0;
		break;
	case(UNI2PT5VOLTS):
		*min = 0.0;
		*max = 2.5;
		break;
	case(UNI2VOLTS):
		*min = 0.0;
		*max = 2.0;
		break;
	case(UNI1PT25VOLTS):
		*min = 0.0;
		*max = 1.25;
		break;
	case(UNI1VOLTS):
		*min = 0.0;
		*max = 1.0;
		break;
	case(UNIPT625VOLTS):
		*min = 0.0;
		*max = 0.625;
		break;
	case(UNIPT5VOLTS):
		*min = 0.0;
		*max = 0.5;
		break;
	case(UNIPT25VOLTS):
		*min = 0.0;
		*max = 0.25;
		break;
	case(UNIPT125VOLTS):
		*min = 0.0;
		*max = 0.125;
		break;
	case(UNIPT2VOLTS):
		*min = 0.0;
		*max = 0.2;
		break;
	case(UNIPT1VOLTS):
		*min = 0.0;
		*max = 0.1;
		break;
	case(UNIPT078VOLTS):
		*min = 0.0;
		*max = 0.078;
		break;
	case(UNIPT05VOLTS):
		*min = 0.0;
		*max = 0.05;
		break;
	case(UNIPT01VOLTS):
		*min = 0.0;
		*max = 0.1;
		break;
	case(UNIPT005VOLTS):
		*min = 0.0;
		*max = 0.005;
		break;
	case(MA0TO20):
		*min = 0.0;
		*max = 20.0;
		break;
	}
}


void ConvertTriggerTypeToString(TriggerType triggerType, char* triggerTypeStr)
{
	switch(triggerType)
	{
	case(TRIG_NONE):
		strcpy(triggerTypeStr, "TRIG_NONE");
		break;
	case(TRIG_POS_EDGE):
		strcpy(triggerTypeStr, "TRIG_POS_EDGE");
		break;
	case(TRIG_NEG_EDGE):
		strcpy(triggerTypeStr, "TRIG_NEG_EDGE");
		break;
	case(TRIG_HIGH):
		strcpy(triggerTypeStr, "TRIG_HIGH");
		break;
	case(TRIG_LOW):
		strcpy(triggerTypeStr, "TRIG_LOW");
		break;
	case(GATE_HIGH):
		strcpy(triggerTypeStr, "GATE_HIGH");
		break;
	case(GATE_LOW):
		strcpy(triggerTypeStr, "GATE_LOW");
		break;
	case(TRIG_RISING):
		strcpy(triggerTypeStr, "TRIG_RISING");
		break;
	case(TRIG_FALLING):
		strcpy(triggerTypeStr, "TRIG_FALLING");
		break;
	case(TRIG_ABOVE):
		strcpy(triggerTypeStr, "TRIG_ABOVE");
		break;
	case(TRIG_BELOW):
		strcpy(triggerTypeStr, "TRIG_BELOW");
		break;
	case(GATE_ABOVE):
		strcpy(triggerTypeStr, "GATE_ABOVE");
		break;
	case(GATE_BELOW):
		strcpy(triggerTypeStr, "GATE_BELOW");
		break;
	case(GATE_IN_WINDOW):
		strcpy(triggerTypeStr, "GATE_IN_WINDOW");
		break;
	case(GATE_OUT_WINDOW):
		strcpy(triggerTypeStr, "GATE_OUT_WINDOW");
		break;
	case(TRIG_PATTERN_EQ):
		strcpy(triggerTypeStr, "TRIG_PATTERN_EQ");
		break;
	case(TRIG_PATTERN_NE):
		strcpy(triggerTypeStr, "TRIG_PATTERN_NE");
		break;
	case(TRIG_PATTERN_ABOVE):
		strcpy(triggerTypeStr, "TRIG_PATTERN_ABOVE");
		break;
	case(TRIG_PATTERN_BELOW):
		strcpy(triggerTypeStr, "TRIG_PATTERN_BELOW");
		break;
	}
}


void ConvertPortTypeToString(DigitalPortType portType, char* portTypeStr)
{
	switch(portType)
	{
	case(AUXPORT0):
		strcpy(portTypeStr, "AUXPORT0");
		break;
	case(AUXPORT1):
		strcpy(portTypeStr, "AUXPORT1");
		break;
	case(AUXPORT2):
		strcpy(portTypeStr, "AUXPORT2");
		break;
	case(FIRSTPORTA):
		strcpy(portTypeStr, "FIRSTPORTA");
		break;
	case(FIRSTPORTB):
		strcpy(portTypeStr, "FIRSTPORTB");
		break;
	case(FIRSTPORTCL):
		strcpy(portTypeStr, "FIRSTPORTCL");
		break;
	case(FIRSTPORTCH):
		strcpy(portTypeStr, "FIRSTPORTCH");
		break;
	case(SECONDPORTA):
		strcpy(portTypeStr, "SECONDPORTA");
		break;
	case(SECONDPORTB):
		strcpy(portTypeStr, "SECONDPORTBA");
		break;
	case(SECONDPORTCL):
		strcpy(portTypeStr, "SECONDPORTCL");
		break;
	case(SECONDPORTCH):
		strcpy(portTypeStr, "SECONDPORTCH");
		break;
	case(THIRDPORTA):
		strcpy(portTypeStr, "THIRDPORTA");
		break;
	case(THIRDPORTB):
		strcpy(portTypeStr, "THIRDPORTB");
		break;
	case(THIRDPORTCL):
		strcpy(portTypeStr, "THIRDPORTCL");
		break;
	case(THIRDPORTCH):
		strcpy(portTypeStr, "THIRDPORTCH");
		break;
	case(FOURTHPORTA):
		strcpy(portTypeStr, "FOURTHPORTA");
		break;
	case(FOURTHPORTB):
		strcpy(portTypeStr, "FOURTHPORTB");
		break;
	case(FOURTHPORTCL):
		strcpy(portTypeStr, "FOURTHPORTCL");
		break;
	case(FOURTHPORTCH):
		strcpy(portTypeStr, "FOURTHPORTCH");
		break;
	case(FIFTHPORTA):
		strcpy(portTypeStr, "FIFTHPORTA");
		break;
	case(FIFTHPORTB):
		strcpy(portTypeStr, "FIFTHPORTB");
		break;
	case(FIFTHPORTCL):
		strcpy(portTypeStr, "FIFTHPORTCL");
		break;
	case(FIFTHPORTCH):
		strcpy(portTypeStr, "FIFTHPORTCH");
		break;
	case(SIXTHPORTA):
		strcpy(portTypeStr, "SIXTHPORTA");
		break;
	case(SIXTHPORTB):
		strcpy(portTypeStr, "SIXTHPORTB");
		break;
	case(SIXTHPORTCL):
		strcpy(portTypeStr, "SIXTHPORTCL");
		break;
	case(SIXTHPORTCH):
		strcpy(portTypeStr, "SIXTHPORTCH");
		break;
	case(SEVENTHPORTA):
		strcpy(portTypeStr, "SEVENTHPORTA");
		break;
	case(SEVENTHPORTB):
		strcpy(portTypeStr, "SEVENTHPORTB");
		break;
	case(SEVENTHPORTCL):
		strcpy(portTypeStr, "SEVENTHPORTCL");
		break;
	case(SEVENTHPORTCH):
		strcpy(portTypeStr, "SEVENTHPORTCH");
		break;
	case(EIGHTHPORTA):
		strcpy(portTypeStr, "EIGHTHPORTA");
		break;
	case(EIGHTHPORTB):
		strcpy(portTypeStr, "EIGHTHPORTB");
		break;
	case(EIGHTHPORTCL):
		strcpy(portTypeStr, "EIGHTHPORTCL");
		break;
	case(EIGHTHPORTCH):
		strcpy(portTypeStr, "EIGHTHPORTCH");
		break;
	}
}


void ConvertPortIoTypeToString(DigitalPortIoType portIoType, char* portIoTypeStr)
{
	switch(portIoType)
	{
	case(DPIOT_IN):
		strcpy(portIoTypeStr, "DPIOT_IN");
		break;
	case(DPIOT_OUT):
		strcpy(portIoTypeStr, "DPIOT_OUT");
		break;
	case(DPIOT_IO):
		strcpy(portIoTypeStr, "DPIOT_IO");
		break;
	case(DPIOT_BITIO):
		strcpy(portIoTypeStr, "DPIOT_BITIO");
		break;
	case(DPIOT_NONCONFIG):
		strcpy(portIoTypeStr, "DPIOT_NONCONFIG");
		break;
	}
}


void ConvertEventTypesToString(DaqEventType eventType, char* eventTypeStr)
{
	switch(eventType)
	{
	case(DE_NONE):
		strcpy(eventTypeStr, "DE_NONE");
		break;
	case(DE_ON_DATA_AVAILABLE):
		strcpy(eventTypeStr, "DE_ON_DATA_AVAILABLE");
		break;
	case(DE_ON_INPUT_SCAN_ERROR):
		strcpy(eventTypeStr, "DE_ON_INPUT_SCAN_ERROR");
		break;
	case(DE_ON_END_OF_INPUT_SCAN):
		strcpy(eventTypeStr, "DE_ON_END_OF_INPUT_SCAN");
		break;
	case(DE_ON_OUTPUT_SCAN_ERROR):
		strcpy(eventTypeStr, "DE_ON_OUTPUT_SCAN_ERROR");
		break;
	case(DE_ON_END_OF_OUTPUT_SCAN):
		strcpy(eventTypeStr, "DE_ON_END_OF_OUTPUT_SCAN");
		break;
	}
}

void ConvertScanOptionsToString(ScanOption scanOptions, char* scanOptionsStr)
{
	strcpy(scanOptionsStr, "");

	if (scanOptions == 0)
		strcat(scanOptionsStr, "Default");
	else {
		if (scanOptions & SO_SINGLEIO)
			strcat(scanOptionsStr, "SO_SINGLEIO, ");
		if (scanOptions & SO_BLOCKIO)
			strcat(scanOptionsStr, "SO_BLOCKIO, ");
		if (scanOptions & SO_BURSTIO)
			strcat(scanOptionsStr, "SO_BURSTIO, ");
		if (scanOptions & SO_CONTINUOUS)
			strcat(scanOptionsStr, "SO_CONTINUOUS, ");
		if (scanOptions & SO_EXTCLOCK)
			strcat(scanOptionsStr, "SO_EXTCLOCK, ");
		if (scanOptions & SO_EXTTRIGGER)
			strcat(scanOptionsStr, "SO_EXTTRIGGER, ");
		if (scanOptions & SO_RETRIGGER)
			strcat(scanOptionsStr, "SO_RETRIGGER, ");
		if (scanOptions & SO_BURSTMODE)
			strcat(scanOptionsStr, "SO_BURSTMODE, ");
		if (scanOptions & SO_PACEROUT)
			strcat(scanOptionsStr, "SO_PACEROUT, ");
		*strrchr(scanOptionsStr, ',')= '\0';
		}
}

void ConvertDaqIChanTypeToString(DaqInChanType daqiChanType, char* daqiChanTypeStr)
{
	switch(daqiChanType)
	{
	case(DAQI_ANALOG_DIFF):
		strcpy(daqiChanTypeStr, "DAQI_ANALOG_DIFF");
		break;
	case(DAQI_ANALOG_SE):
		strcpy(daqiChanTypeStr, "DAQI_ANALOG_SE");
		break;
	case(DAQI_DIGITAL):
		strcpy(daqiChanTypeStr, "DAQI_DIGITAL");
		break;
	case(DAQI_CTR16):
		strcpy(daqiChanTypeStr, "DAQI_CTR16");
		break;
	case(DAQI_CTR32):
		strcpy(daqiChanTypeStr, "DAQI_CTR32");
		break;
	case(DAQI_CTR48):
		strcpy(daqiChanTypeStr, "DAQI_CTR48");
		break;
	case(DAQI_DAC):
		strcpy(daqiChanTypeStr, "DAQI_DAC");
		break;
	}
}


void ConvertDaqOChanTypeToString(DaqOutChanType daqoChanType, char* daqoChanTypeStr)
{
	switch(daqoChanType)
	{
	case(DAQI_ANALOG_DIFF):
		strcpy(daqoChanTypeStr, "DAQO_ANALOG");
		break;
	case(DAQI_ANALOG_SE):
		strcpy(daqoChanTypeStr, "DAQO_DIGITAL");
		break;
	}
}

void ConvertTCTypeToString(TcType type, char* typeStr)
{
	switch(type)
	{
	case(TC_J):
		strcpy(typeStr, "J");
		break;
	case(TC_K):
		strcpy(typeStr, "K");
		break;
	case(TC_T):
		strcpy(typeStr, "T");
		break;
	case(TC_E):
		strcpy(typeStr, "E");
		break;
	case(TC_R):
		strcpy(typeStr, "R");
		break;
	case(TC_S):
		strcpy(typeStr, "S");
		break;
	case(TC_B):
		strcpy(typeStr, "B");
		break;
	case(TC_N):
		strcpy(typeStr, "N");
		break;
	}
}

void ConvertSensorConnectionTypeToString(SensorConnectionType type, char* typeStr)
{
	switch(type)
	{
	case(SCT_2_WIRE_1):
		strcpy(typeStr, "2-wire (1 sensor)");
		break;
	case(SCT_2_WIRE_2):
		strcpy(typeStr, "2-wire (2 sensors)");
		break;
	case(SCT_3_WIRE):
		strcpy(typeStr, "3-wire");
		break;
	case(SCT_4_WIRE):
		strcpy(typeStr, "4-wire");
		break;
	}
}

/****************************************************************************
 * Device Info Functions
 ****************************************************************************/
UlError getDevInfoHasAi(DaqDeviceHandle daqDeviceHandle, int* hasAi)
{
	long long aiSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_AI_DEV, 0, &aiSupported);

	*hasAi = (int)aiSupported;

	return err;
}

UlError getDevInfoHasAo(DaqDeviceHandle daqDeviceHandle, int* hasAo)
{
	long long aoSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_AO_DEV, 0, &aoSupported);

	*hasAo = (int)aoSupported;

	return err;
}

UlError getDevInfoHasDio(DaqDeviceHandle daqDeviceHandle, int* hasDio)
{
	long long dioSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_DIO_DEV, 0, &dioSupported);

	*hasDio = (int)dioSupported;

	return err;
}

UlError getDevInfoHasCtr(DaqDeviceHandle daqDeviceHandle, int* hasCtr)
{
	long long ctrSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_CTR_DEV, 0, &ctrSupported);

	*hasCtr = (int)ctrSupported;

	return err;
}

UlError getDevInfoHasTmr(DaqDeviceHandle daqDeviceHandle, int* hasTmr)
{
	long long tmrSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_TMR_DEV, 0, &tmrSupported);

	*hasTmr = (int)tmrSupported;

	return err;
}

UlError getDevInfoHasDaqi(DaqDeviceHandle daqDeviceHandle, int* hasDaqi)
{
	long long daqiSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_DAQI_DEV, 0, &daqiSupported);

	*hasDaqi = (int)daqiSupported;

	return err;
}

UlError getDevInfoHasDaqo(DaqDeviceHandle daqDeviceHandle, int* hasDaqo)
{
	long long daqoSupported;
	UlError err = ERR_NO_ERROR;

	err = ulDevGetInfo(daqDeviceHandle, DEV_INFO_HAS_DAQO_DEV, 0, &daqoSupported);

	*hasDaqo = (int)daqoSupported;

	return err;
}



/****************************************************************************
 * Analog Input Info Functions
 ****************************************************************************/
UlError getAiInfoHasPacer(DaqDeviceHandle daqDeviceHandle, int* hasPacer)
{
	long long pacerSupported;
	UlError err = ERR_NO_ERROR;

	err = ulAIGetInfo(daqDeviceHandle, AI_INFO_HAS_PACER, 0, &pacerSupported);

	*hasPacer = (int)pacerSupported;

	return err;
}


UlError getAiInfoFirstTriggerType(DaqDeviceHandle daqDeviceHandle, TriggerType* triggerType, char* triggerTypeStr)
{
	UlError err = ERR_NO_ERROR;

	long long triggerTypes = 0;
	err = ulAIGetInfo(daqDeviceHandle, AI_INFO_TRIG_TYPES, 0, &triggerTypes);

	if (err == ERR_NO_ERROR && triggerTypes != 0)
	{
		// use the first available trigger type
		long long triggerMask = 1;
		while ((triggerTypes & triggerMask) == 0)
			triggerMask = triggerMask << 1;

		*triggerType = (TriggerType)triggerMask;

		ConvertTriggerTypeToString(*triggerType, triggerTypeStr);
	}

	return err;
}

UlError getAiInfoRanges(DaqDeviceHandle daqDeviceHandle, AiInputMode inputMode, int *numberOfRanges, Range* ranges)
{
	UlError err = ERR_NO_ERROR;
	int i = 0;
	long long numRanges = 0;
	long long rng;

	if (inputMode == AI_SINGLE_ENDED)
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_NUM_SE_RANGES, 0, &numRanges);
	}
	else
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_NUM_DIFF_RANGES, 0, &numRanges);
	}

	if(numRanges <= *numberOfRanges)
	{
		for (i=0; i<numRanges; i++)
		{
			if (inputMode == AI_SINGLE_ENDED)
			{
				err = ulAIGetInfo(daqDeviceHandle, AI_INFO_SE_RANGE, i, &rng);
			}
			else
			{
				err = ulAIGetInfo(daqDeviceHandle, AI_INFO_DIFF_RANGE, i, &rng);
			}

			ranges[i] = (Range)rng;
		}
	}

	*numberOfRanges = (int)numRanges;

	return err;
}

UlError getAiInfoFirstSupportedInputMode(DaqDeviceHandle daqDeviceHandle, int* numberOfChannels, AiInputMode *inputMode, char* inputModeStr)
{
	UlError err = ERR_NO_ERROR;

	long long numChans;
	err = ulAIGetInfo(daqDeviceHandle, AI_INFO_NUM_CHANS_BY_MODE, AI_SINGLE_ENDED, &numChans);

	if (numChans > 0)
	{
		*inputMode = AI_SINGLE_ENDED;
	}
	else
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_NUM_CHANS_BY_MODE, AI_DIFFERENTIAL, &numChans);

		if (numChans > 0)
			*inputMode = AI_DIFFERENTIAL;
	}

	ConvertInputModeToString(*inputMode, inputModeStr);

	*numberOfChannels = (int)numChans;

	return err;
}

UlError getAiInfoFirstSupportedRange(DaqDeviceHandle daqDeviceHandle, AiInputMode inputMode, Range* range, char* rangeStr)
{
	UlError err = ERR_NO_ERROR;
	long long rng;

	if (inputMode == AI_SINGLE_ENDED)
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_SE_RANGE, 0, &rng);
	}
	else
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_DIFF_RANGE, 0, &rng);
	}

	*range = (Range)rng;

	ConvertRangeToString(*range, rangeStr);

	return err;
}

UlError getAiInfoQueueTypes(DaqDeviceHandle daqDeviceHandle, int* queueTypes)
{
	UlError err = ERR_NO_ERROR;
	long long qTypes;

	err = ulAIGetInfo(daqDeviceHandle, AI_INFO_QUEUE_TYPES, 0, &qTypes);

	*queueTypes = (int)qTypes;

	return err;
}

UlError getAiInfoHasTempChan(DaqDeviceHandle daqDeviceHandle, int* hasTempChan, int* numberOfChannels)
{
	long long chanTypeMask = 0;
	long long tempChanCount = 0;
	UlError err = ERR_NO_ERROR;

	*numberOfChannels = 0;

	err = ulAIGetInfo(daqDeviceHandle, AI_INFO_CHAN_TYPES, 0, &chanTypeMask);

	if(chanTypeMask & (AI_TC  | AI_RTD | AI_THERMISTOR | AI_SEMICONDUCTOR))
		*hasTempChan = 1;
	else
		*hasTempChan = 0;

	if(*hasTempChan)
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_NUM_CHANS_BY_TYPE, AI_TC, &tempChanCount);
		*numberOfChannels = tempChanCount;
	}	

	return err;
}

UlError getAiInfoIepeSupported(DaqDeviceHandle daqDeviceHandle, int* iepeSupported)
{
	long long supportsIepe;
	UlError err = ERR_NO_ERROR;

	err = ulAIGetInfo(daqDeviceHandle, AI_INFO_IEPE_SUPPORTED, 0, &supportsIepe);

	*iepeSupported = (int)supportsIepe;

	return err;
}

UlError getAiConfigTempChanConfig(DaqDeviceHandle daqDeviceHandle, int chan, char* chanTypeStr, char* sensorStr)
{
	UlError err = ERR_NO_ERROR;
	long long chanTypeMask = 0;

	long long chanType, cfg;
	char typeStr[64] = "";
	char cfgStr[64] = "N/A";

	err = ulAIGetConfig(daqDeviceHandle, AI_CFG_CHAN_TYPE, chan, &chanType);

	if(err == ERR_NO_ERROR)
	{
		if(chanType == AI_TC)
		{
			strcpy(typeStr, "Thermocouple");

			err = ulAIGetConfig(daqDeviceHandle, AI_CFG_CHAN_TC_TYPE, chan, &cfg);

			ConvertTCTypeToString((TcType)cfg, cfgStr);
		}
		else if(chanType == AI_RTD || chanType == AI_THERMISTOR)
		{
			if(chanType == AI_RTD)
				strcpy(typeStr, "RTD");
			else
				strcpy(typeStr, "Thermistor");

			err = ulAIGetConfig(daqDeviceHandle, AI_CFG_CHAN_SENSOR_CONNECTION_TYPE, chan, &cfg);

			ConvertSensorConnectionTypeToString((SensorConnectionType) cfg, cfgStr);
		}
		else if(chanType == AI_SEMICONDUCTOR)
		{
			strcpy(typeStr, "Semicoductor");
		}
		else if(chanType == AI_VOLTAGE)
		{
			strcpy(typeStr, "Voltage");
		}
		else if(chanType == AI_DISABLED)
		{
			strcpy(typeStr, "Disabled");
		}
	}
	else if(err == ERR_CONFIG_NOT_SUPPORTED) // channel is not configurable
	{
		err = ulAIGetInfo(daqDeviceHandle, AI_INFO_CHAN_TYPES, 0, &chanTypeMask);

		if(chanTypeMask & AI_TC)
		{
			strcpy(typeStr, "Thermocouple");

			err = ulAIGetConfig(daqDeviceHandle, AI_CFG_CHAN_TC_TYPE, chan, &cfg);

			ConvertTCTypeToString((TcType)cfg, cfgStr);
		}
	}

	strcpy(chanTypeStr, typeStr);
	strcpy(sensorStr, cfgStr);

	return err;
}


/****************************************************************************
 * Analog Output Info Functions
 ****************************************************************************/
UlError getAoInfoHasPacer(DaqDeviceHandle daqDeviceHandle, int* hasPacer)
{
	UlError err = ERR_NO_ERROR;
	long long pacerSupport;

	err = ulAOGetInfo(daqDeviceHandle, AO_INFO_HAS_PACER, 0, &pacerSupport);

	*hasPacer = (int)pacerSupport;

	return err;
}

UlError getAoInfoFirstSupportedRange(DaqDeviceHandle daqDeviceHandle, Range* range, char* rangeStr)
{
	UlError err = ERR_NO_ERROR;
	long long rng;

	err = ulAOGetInfo(daqDeviceHandle, AO_INFO_RANGE, 0, &rng);

	*range = (Range)rng;

	ConvertRangeToString(*range, rangeStr);

	return err;
}


/****************************************************************************
 * Digital I/O Info Functions
 ****************************************************************************/
UlError getDioInfoHasPacer(DaqDeviceHandle daqDeviceHandle, DigitalDirection direction, int* hasPacer)
{
	UlError err = ERR_NO_ERROR;
	long long pacerSupport;

	err = ulDIOGetInfo(daqDeviceHandle, DIO_INFO_HAS_PACER, direction, &pacerSupport);

	*hasPacer = (int)pacerSupport;

	return err;
}

UlError getDioInfoFirstSupportedPortType(DaqDeviceHandle daqDeviceHandle, DigitalPortType* portType, char* portTypeStr)
{
	UlError err = ERR_NO_ERROR;
	long long pType = 0;

	err = ulDIOGetInfo(daqDeviceHandle, DIO_INFO_PORT_TYPE, 0, &pType);

	*portType = (DigitalPortType)pType;

	ConvertPortTypeToString(*portType, portTypeStr);

	return err;
}

UlError getDioInfoNumberOfBitsForFirstPort(DaqDeviceHandle daqDeviceHandle, int* bitsPerPort)
{
	UlError err = ERR_NO_ERROR;
	long long numBits;

	err = ulDIOGetInfo(daqDeviceHandle, DIO_INFO_NUM_BITS, 0, &numBits);

	*bitsPerPort = (int)numBits;

	return err;
}

UlError getDioInfoFirstSupportedPortIoType(DaqDeviceHandle daqDeviceHandle, DigitalPortIoType* portIoType, char* portIoTypeStr)
{
	UlError err = ERR_NO_ERROR;
	long long ioType;

	err = ulDIOGetInfo(daqDeviceHandle, DIO_INFO_PORT_IO_TYPE, 0, &ioType);

	*portIoType = (DigitalPortIoType)ioType;

	ConvertPortIoTypeToString(*portIoType, portIoTypeStr);

	return err;
}


/****************************************************************************
 * Counter Info Functions
 ****************************************************************************/
UlError getCtrInfoNumberOfChannels(DaqDeviceHandle daqDeviceHandle, int* numberOfCtrChannels)
{
	UlError err = ERR_NO_ERROR;
	long long numChans;

	err = ulCtrGetInfo(daqDeviceHandle, CTR_INFO_NUM_CTRS, 0, &numChans);

	*numberOfCtrChannels = (int)numChans;

	return err;
}

UlError getCtrInfoHasPacer(DaqDeviceHandle daqDeviceHandle, int* hasPacer)
{
	UlError err = ERR_NO_ERROR;
	long long pacerSupport;

	err = ulCtrGetInfo(daqDeviceHandle, CTR_INFO_HAS_PACER, 0, &pacerSupport);

	*hasPacer = (int)pacerSupport;

	return err;
}

UlError getCtrInfoSupportedEventCounters(DaqDeviceHandle daqDeviceHandle, int* eventCounters, int* numberOfEventCounters)
{
	UlError err = ERR_NO_ERROR;
	int i = 0;
	int numberOfCounters = 0;
	long long measurementTypes;

	int numEvntCtrs = 0;

	// get the number of counter channels
	err = getCtrInfoNumberOfChannels(daqDeviceHandle, &numberOfCounters);

	// fill a descriptor for each channel
	for (i = 0; i < numberOfCounters; i++)
	{
		err = ulCtrGetInfo(daqDeviceHandle, CTR_INFO_MEASUREMENT_TYPES, i, &measurementTypes);

		if (measurementTypes & CMT_COUNT)
		{
			if(numEvntCtrs < *numberOfEventCounters)
			{
				*eventCounters++ = i;
			}

			numEvntCtrs++;
		}
	}

	*numberOfEventCounters = numEvntCtrs;

	return err;
}

UlError getCtrInfoSupportedEncoderCounters(DaqDeviceHandle daqDeviceHandle, int* encoders, int* numberOfEncoders)
{
	UlError err = ERR_NO_ERROR;
	int i = 0;
	int numberOfCounters = 0;
	long long measurementTypes;

	int numEncCtrs = 0;

	// get the number of counter channels
	err = getCtrInfoNumberOfChannels(daqDeviceHandle, &numberOfCounters);

	// fill a descriptor for each channel
	for (i = 0; i < numberOfCounters; i++)
	{
		err = ulCtrGetInfo(daqDeviceHandle, CTR_INFO_MEASUREMENT_TYPES, i, &measurementTypes);

		if (measurementTypes & CMT_ENCODER)
		{
			if(numEncCtrs < *numberOfEncoders)
			{
				*encoders++ = i;
			}

			numEncCtrs++;
		}
	}

	*numberOfEncoders = numEncCtrs;

	return err;
}

UlError getCtrInfoMeasurementTypes(DaqDeviceHandle daqDeviceHandle, long long counterNumber, int* measurementTypes)
{
	UlError err = ERR_NO_ERROR;
	long long measTypes;

	err = ulCtrGetInfo(daqDeviceHandle, CTR_INFO_MEASUREMENT_TYPES, counterNumber, &measTypes);

	*measurementTypes = (int)measTypes;

	return err;
}


/****************************************************************************
 * DAQI Info Functions
 ****************************************************************************/
UlError getDaqiChannelTypes(DaqDeviceHandle daqDeviceHandle, int* chanTypesMask)
{
	UlError err = ERR_NO_ERROR;
	long long ctMask;

	err = ulDaqIGetInfo(daqDeviceHandle, DAQI_INFO_CHAN_TYPES, 0, &ctMask);

	*chanTypesMask = (int)ctMask;

	return err;
}


UlError getDaqiInfoFirstTriggerType(DaqDeviceHandle daqDeviceHandle, TriggerType* triggerType, char* triggerTypeStr)
{
	UlError err = ERR_NO_ERROR;

	long long triggerTypes = 0;
	err = ulDaqIGetInfo(daqDeviceHandle, DAQI_INFO_TRIG_TYPES, 0, &triggerTypes);

	if (err == ERR_NO_ERROR && triggerTypes != 0)
	{
		// use the first available trigger type
		long long triggerMask = 1;
		while ((triggerTypes & triggerMask) == 0)
			triggerMask = triggerMask << 1;

		*triggerType = (TriggerType)triggerMask;

		ConvertTriggerTypeToString(*triggerType, triggerTypeStr);
	}

	return err;
}


/****************************************************************************
 * DAQO Info Functions
 ****************************************************************************/
UlError getDaqoChannelTypes(DaqDeviceHandle daqDeviceHandle, int* chanTypesMask)
{
	UlError err = ERR_NO_ERROR;
	long long ctMask;

	err = ulDaqOGetInfo(daqDeviceHandle, DAQO_INFO_CHAN_TYPES, 0, &ctMask);

	*chanTypesMask = (int)ctMask;

	return err;
}

void resetCursor() {printf("\033[1;1H");}
void clearEOL() {printf("\033[2K");}
void cursorUp() {printf("\033[A");}

void flush_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

int enter_press()
{
	int stdin_value = 0;
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    stdin_value = FD_ISSET(STDIN_FILENO, &fds);
    if (stdin_value != 0)
    	flush_stdin();
    return stdin_value;
}

int selectDAQDevice(int numberOfDAQDevices)
{
	int daqDeviceIndex = 0;
	int ret;
	while(1)
	{
		printf("\nPlease select a DAQ device, enter a number between 0 and %d: ", numberOfDAQDevices - 1);
		ret = scanf("%d",&daqDeviceIndex);
		if(!ret || daqDeviceIndex < 0 || daqDeviceIndex >= numberOfDAQDevices)
		{
			printf("Invalid device number\n");
		}
		else
		{
			printf("\n");
			break;
		}
		flush_stdin();
	}

	return 	daqDeviceIndex;
}
#endif /* UTILITY_H_ */
