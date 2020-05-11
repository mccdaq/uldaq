/*
    Wrapper call demonstrated:        ulAInSetTrigger()

    Purpose:                          Setup an external trigger

    Demonstration:                    Uses the first available trigger type to
                                      set up an external trigger that is used
                                      to start a scan

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an analog input subsystem
    3. Verify the analog input subsystem has a hardware pacer
    4. Get the supported trigger types
    5. Get the supported queue types
    6. Fill the channel array
    7. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    8. Call ulAInSetTrigger to set the external trigger
    9. Call ulAInScan() to start the scan of A/D input channels
    10. Call ulAiScanStatus to check the status of the background operation
    11. Display the data for each channel
    12. Call ulAinScanStop() to stop the background operation
    13. Call ulDisconnectDaqDevice and ulReleaseDaqDevice() before exiting the process
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64
#define MAX_SCAN_OPTIONS_LENGTH 256

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	// set some variables that are used to acquire data
	int lowChan = 0;
	int highChan = 3;
	AiInputMode inputMode;
	Range range;
	int samplesPerChannel = 10000;
	double rate = 100;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS | SO_EXTTRIGGER);
	AInScanFlag flags = AINSCAN_FF_DEFAULT;

	int hasAI = 0;
	int hasPacer = 0;
	int numberOfChannels = 0;
	TriggerType triggerType;
	int index = 0;

	char inputModeStr[MAX_STR_LENGTH];
	char rangeStr[MAX_STR_LENGTH];
	char triggerTypeStr[MAX_STR_LENGTH];
	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];

	// allocate a buffer to receive the data
	int chanCount = 0;
	double *buffer = NULL;
	UlError err = ERR_NO_ERROR;

	int __attribute__ ((unused))ret;
	char c;
	int i = 0;

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

	// verify the specified device supports analog input
	err = getDevInfoHasAi(daqDeviceHandle, &hasAI);
	if (!hasAI)
	{
		printf("\nThe specified DAQ device does not support analog input\n");
		goto end;
	}

	// verify the specified device supports hardware pacing for analog input
	err = getAiInfoHasPacer(daqDeviceHandle, &hasPacer);
	if (!hasPacer)
	{
		printf("\nThe specified DAQ device does not support hardware paced analog input\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
	{
		goto end;
	}

	// get the first supported analog input mode
	err = getAiInfoFirstSupportedInputMode(daqDeviceHandle, &numberOfChannels, &inputMode, inputModeStr);

	if (highChan >= numberOfChannels)
		highChan = numberOfChannels - 1;

	chanCount = highChan - lowChan + 1;

	// allocate a buffer to receive the data
	buffer = (double*) malloc(chanCount * samplesPerChannel * sizeof(double));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	// get the first supported analog input range
	err = getAiInfoFirstSupportedRange(daqDeviceHandle, inputMode, &range, rangeStr);

	// get the first supported trigger type (this returns a digital trigger type)
	getAiInfoFirstTriggerType(daqDeviceHandle, &triggerType, triggerTypeStr);

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulAInSetTrigger()\n");
	printf("    Channels: %d - %d\n", lowChan, highChan);
	printf("    Input mode: %s\n", inputModeStr);
	printf("    Range: %s\n", rangeStr);
	printf("    Samples per channel: %d\n", samplesPerChannel);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("    Trigger type: %s\n", triggerTypeStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	// clear the display
	ret = system("clear");

	//	set the trigger
	//
	// this example uses the default values for setting the trigger so there is no need to call this function ...
	// if you want to change the trigger type (or any other trigger parameter), uncomment this function call and
	// change the trigger type (or any other parameter)
	//err = ulAInSetTrigger( daqDeviceHandle, triggerType, 0, 0.0, 0.0, 0);

	// start the acquisition
	err = ulAInScan(daqDeviceHandle, lowChan, highChan, inputMode, range, samplesPerChannel, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;

		// get the initial status of the acquisition
		ulAInScanStatus(daqDeviceHandle, &status, &transferStatus);

		printf ("Hit 'Enter' to quit waiting for trigger\n\n");
		printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);
		printf ("Waiting for trigger ...\n");

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			// get the current status of the acquisition
			err = ulAInScanStatus(daqDeviceHandle, &status, &transferStatus);

			index = transferStatus.currentIndex;
			if(err == ERR_NO_ERROR && index >= 0)
			{
				resetCursor();
				printf("%-40s\n\n","Hit 'Enter' to terminate the process");
				printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);
				printf("actual scan rate = %f\n\n", rate);

				printf("currentScanCount =  %-10llu \n", transferStatus.currentScanCount);
				printf("currentTotalCount = %-10llu \n", transferStatus.currentTotalCount);
				printf("currentIndex =      %-10d \n\n", index);

				// display the data
				for (i = 0; i < chanCount; i++)
				{
					printf("chan %d = %+-10.6f\n",
							i + lowChan,
							buffer[index + i]);
				}

				usleep(100000);
			}
		}

		if (index < 0)
			printf("Trigger cancelled by user\n");

		// stop the acquisition if it is still running
		if (status == SS_RUNNING && err == ERR_NO_ERROR)
		{
			err = ulAInScanStop(daqDeviceHandle);
		}
	}

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

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

