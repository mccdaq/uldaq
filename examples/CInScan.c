/*
    UL call demonstrated:       	  ulCInScan()

    Purpose:                          Performs a continuous scan of the
                                      first supported event counter channel

    Demonstration:                    Displays the event counter data for
    								  the first supported event counter

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an counter input subsystem
    4. Verify the counter input subsystem has a hardware pacer
    5. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    6. Call ulCInScan() to start the scan of A/D input channels
    7. Call ulCInScanStatus() to check the status of the background operation
    8. Display the data for each channel
    9. Call ulCInScanStop() to stop the background operation
    10. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_SCAN_OPTIONS_LENGTH 256

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	// set some variables that are used to acquire data
	int lowCtr = 0;
	int highCtr = 1;
	int chanCount = 0;
	const int samplesPerCounter = 10000;
	double rate = 1000;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	CInScanFlag flags = CINSCAN_FF_DEFAULT;

	int hasCI = 0;
	int hasPacer = 0;
	int index = 0;
	int numberOfCounters = 0;

	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];

	unsigned long long* buffer = NULL;
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
	err = getDevInfoHasCtr(daqDeviceHandle, &hasCI);
	if (!hasCI)
	{
		printf("\nThe specified DAQ device does not support counter input\n");
		goto end;
	}

	// verify the specified DAQ device supports hardware pacing for analog input
	err = getCtrInfoHasPacer(daqDeviceHandle, &hasPacer);
	if (!hasPacer)
	{
		printf("\nThe specified DAQ device does not support hardware paced counter input\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the counter numbers for the supported counters
	getCtrInfoNumberOfChannels(daqDeviceHandle, &numberOfCounters);

	if (highCtr >= numberOfCounters)
		highCtr = numberOfCounters - 1;

	chanCount = highCtr - lowCtr + 1;

	// allocate a buffer to receive the data
	buffer = (unsigned long long*) malloc(chanCount * samplesPerCounter * sizeof(unsigned long long));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulCInscan()\n");
	printf("    Counter: %d - %d\n", lowCtr, highCtr);
	printf("    Samples per channel: %d\n", samplesPerCounter);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	// start the acquisition
	err = ulCInScan(daqDeviceHandle, lowCtr, highCtr, samplesPerCounter, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;

		// get the initial status of the acquisition
		ulCInScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			// get the current status of the acquisition
			err = ulCInScanStatus(daqDeviceHandle, &status, &transferStatus);

			if(err == ERR_NO_ERROR)
			{
				// reset the cursor to the top of the display and
				// show the termination message
				resetCursor();
				printf("Hit 'Enter' to terminate the process\n\n");
				printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);
				printf("actual scan rate = %f\n\n", rate);

				index = transferStatus.currentIndex;
				printf("currentScanCount =  %-10llu \n", transferStatus.currentScanCount);
				printf("currentTotalCount = %-10llu \n", transferStatus.currentTotalCount);
				printf("currentIndex =      %-10d \n\n", index);

				if(index >= 0)
				{
					// display the data
					for (i = 0; i < chanCount; i++)
					{
						printf("Counter %d = %-20lld\n",
								i + lowCtr,
								buffer[index + i]);
					}

					usleep(100000);
				}
			}
		}

		// stop the acquisition if it is still running
		if (status == SS_RUNNING && err == ERR_NO_ERROR)
		{
			err = ulCInScanStop(daqDeviceHandle);
		}
	}

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

	// release the handle to the DAQ device
	ulReleaseDaqDevice(daqDeviceHandle);

	// release the scan buffer
	if(buffer)
		free(buffer);

	if(err != ERR_NO_ERROR)
	{
		char errMsg[ERR_MSG_LEN];
		ulGetErrMsg(err, errMsg);
		printf("Error Code: %d \n", err);
		printf("Error Message: %s \n", errMsg);
	}

	return 0;
}
