/*
    UL call demonstrated:        	  ulDOut()

    Purpose:                          Writes the first digital port

    Demonstration:                    Writes the digital data to the
    								  first digital port

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an digital output subsystem
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Get the first supported digital port
    7. Call ulDConfigPort to configure the port for output
    8. Call ulDOut() to write a value to the digital port
    9. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	int hasDIO = 0;
	int bitsPerPort = 0;
	DigitalPortType portType;
	DigitalPortIoType portIoType;

	char portTypeStr[MAX_STR_LENGTH];
	char portIoTypeStr[MAX_STR_LENGTH];

	unsigned long long maxPortValue = 0;

	char * p;
	char dataStr[MAX_STR_LENGTH];
	unsigned long long data = 0;
	UlError err = ERR_NO_ERROR;

	int i = 0;
	int __attribute__((unused)) ret;
	char c;

	// Get descriptors for all of the available DAQ devices
	err = ulGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);

	if (err != ERR_NO_ERROR)
		goto end;

	// verify at least one DAQ device is detected
	if (numDevs == 0)
	{
		printf("No DAQ device is detected\n");
		goto end;
	}

	printf("Found %d DAQ device(s)\n", numDevs);
	for (i = 0; i < (int) numDevs; i++)
		printf("  [%d] %s: (%s)\n", i, devDescriptors[i].productName, devDescriptors[i].uniqueId);

	if(numDevs > 1)
		descriptorIndex = selectDAQDevice(numDevs);

	// get a handle to the DAQ device associated with the first descriptor
	daqDeviceHandle = ulCreateDaqDevice(devDescriptors[descriptorIndex]);

	if (daqDeviceHandle == 0)
	{
		printf ("\nUnable to create a handle to the specified DAQ device\n");
		goto end;
	}

	// verify the specified DAQ device supports digital output
	err = getDevInfoHasDio(daqDeviceHandle, &hasDIO);
	if (!hasDIO)
	{
		printf("\nThe DAQ device does not support digital I/O\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the first port type (AUXPORT0, FIRSTPORTA, ...)
	err = getDioInfoFirstSupportedPortType(daqDeviceHandle, &portType, portTypeStr);

	// get the I/O type for the fisrt port
	err = getDioInfoFirstSupportedPortIoType(daqDeviceHandle, &portIoType, portIoTypeStr);

	// get the number of bits for the first port
	err = getDioInfoNumberOfBitsForFirstPort(daqDeviceHandle, &bitsPerPort);

	if(portIoType == DPIOT_IO || portIoType == DPIOT_BITIO)
	{
		// configure the first port for output
		err = ulDConfigPort(daqDeviceHandle, portType, DD_OUTPUT);
	}

	// calculate the max value for the port
	maxPortValue = (unsigned long long) pow(2.0, (double)bitsPerPort) - 1;

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulDOut()\n");
	printf("    Port: %s\n", portTypeStr);
	printf("    Port I/O type: %s\n", portIoTypeStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	while(err == ERR_NO_ERROR)
	{
		// reset the cursor to the top of the display and
		// show the termination message
		resetCursor();
		printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);

		clearEOL();
		printf ("Enter a value between 0 and %llu (or non-numeric character to exit):  ", maxPortValue);
		ret = scanf ("%63s", dataStr);
		strtod(dataStr, &p);
		if (*p != '\0')
			break;
		data = atoll(dataStr);

		if (data > maxPortValue)
		{
			clearEOL();
			printf ("Invalid value entered\n");
		}
		else
		{
			clearEOL();
			err = ulDOut(daqDeviceHandle, portType, data);
		}

		usleep(100000);
	}

	if(portIoType == DPIOT_IO || portIoType == DPIOT_BITIO)
	{
		// before leaving, configure the entire port for input
		err = ulDConfigPort(daqDeviceHandle, portType, DD_INPUT);
	}

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

	// release the handle to the DAQ device
	ulReleaseDaqDevice(daqDeviceHandle);

	if(err != ERR_NO_ERROR)
	{
		char errMsg[ERR_MSG_LEN];
		ulGetErrMsg(err, errMsg);
		printf("Error Code: %d \n", err);
		printf("Error Message: %s \n", errMsg);
	}

	return 0;
}

