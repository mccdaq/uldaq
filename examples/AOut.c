/*
    UL call demonstrated:        	  ulAOut()

    Purpose:                          Writes an A/D output channel

    Demonstration:                    Writes the analog output data to a
                                      user-specified channel

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an analog output subsystem
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Enter a value to output for the A/D channel
    6. Call ulAOut() to write a value to an A/D output channel
    7. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
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

	int channel = 0;
	Range range;
	AOutFlag flags = AOUT_FF_DEFAULT;

	int hasAO = 0;
	double min = 0.0;
	double max = 0.0;

	char rangeStr[MAX_STR_LENGTH];

	char * p;
	char dataStr[MAX_STR_LENGTH];
	double data = 0.0;
	UlError err = ERR_NO_ERROR;

	int i = 0;
	int __attribute__ ((unused))ret;
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

	daqDeviceHandle = ulCreateDaqDevice(devDescriptors[descriptorIndex]);

	if (daqDeviceHandle == 0)
	{
		printf ("\nUnable to create a handle to the specified DAQ device\n");
		goto end;
	}

	// verify the specified DAQ device supports analog output
	err = getDevInfoHasAo(daqDeviceHandle, &hasAO);
	if (!hasAO)
	{
		printf("\nThe DAQ device does not support analog output\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	getAoInfoFirstSupportedRange(daqDeviceHandle, &range, rangeStr);

	ConvertRangeToMinMax(range, &min, &max);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulAOut()\n");
	printf("    Channel: 0\n");
	printf("    Range: %s\n\n", rangeStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	// clear the display
	ret = system("clear");

	while(err == ERR_NO_ERROR && !enter_press())
	{
		// reset the cursor to the top of the display and
		// show the termination message
		cursorUp();
		clearEOL();
		resetCursor();
		printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);
		printf ("Enter a value between %f and %f volts (or non-numeric character to exit):  ", min, max);
		ret = scanf ("%63s", dataStr);
		strtod(dataStr, &p);
		if (*p != '\0')
			break;
		data = atof(dataStr);

		err = ulAOut(daqDeviceHandle, channel, range, flags, data);

		usleep(100000);
	}

end:

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

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

