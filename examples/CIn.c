/*
    UL call demonstrated:        	  ulCIn()

    Purpose:                          Reads a counter input channel

    Demonstration:                    Displays the event counter input data
                                      on a user-specified channel

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has a counter input subsystem
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Call ulCClear() to clear the counter (set it to 0)
    6. Call ulCIn() to read a value from a counter channel
    7. Display the data for the specified counter
    8. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	int ctrNumber = 0;
	int hasCTR = 0;

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

	// verify the specified DAQ device supports counter input
	err = getDevInfoHasCtr(daqDeviceHandle, &hasCTR);
	if (!hasCTR)
	{
		printf("\nThe specified DAQ device does not support counter input\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;


	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulCIn()\n");
	printf("    Counter: %d\n", ctrNumber);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	// clear the counter
	err = ulCClear(daqDeviceHandle, ctrNumber);

	if (err != ERR_NO_ERROR)
		goto end;

	while(err == ERR_NO_ERROR && !enter_press())
	{
		// reset the cursor to the top of the display and
		// show the termination message
		resetCursor();
		printf("Hit 'Enter' to terminate the process\n\n");
		printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);

		err = ulCIn(daqDeviceHandle, ctrNumber, &data);

		// display data for the counter channel
		if(err == ERR_NO_ERROR)
		{
			printf("Counter(%d) Data: %lld\n", ctrNumber, data);
		}

		usleep(100000);
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

