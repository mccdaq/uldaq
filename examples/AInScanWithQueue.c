/*
    UL call demonstrated:        	  ulAInLoadQueue()

    Purpose:                          Set up the queue with available ranges
    								  and input modes

    Demonstration:                    Initialize and load the queue

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an analog input subsystem
    4. Verify the analog input subsystem has a hardware pacer
    5. Get the supported ranges
    6. Get the supported queue types
    7. Setup the queue structure array
    8. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    9. Call ulAInLoadQueue() to load the queue
    10. Call ulAInScan() to start the scan of A/D input channels
    11. Call ulAiScanStatus to check the status of the background operation
    12. Display the data for each channel
    13. Call ulAinScanStop() to stop the background operation
    14. Call ulDisconnectDaqDevice and ulReleaseDaqDevice() before exiting the process
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_RANGE_COUNT  16
#define MAX_STR_LENGTH 64
#define MAX_SCAN_OPTIONS_LENGTH 256
#define MAX_QUEUE_SIZE 32	// arbitrary limit for the size for the queue

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	// when using the queue, the lowChan, highChan, inputMode, and range
	// parameters are ignored since they are specified in queueArray
	int lowChan = 0;
	int highChan = 3;
	AiInputMode inputMode;
	Range range;

	// set some variables that are used to acquire data
	int samplesPerChannel = 10000;
	double rate = 100;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	AInScanFlag flags = AINSCAN_FF_DEFAULT;

	Range ranges[MAX_RANGE_COUNT];

	int hasAI = 0;
	int hasPacer = 0;
	int numRanges = MAX_RANGE_COUNT;
	int numberOfChannels = 0;
	int queueTypes;
	int index = 0;

	char rangeStr[MAX_STR_LENGTH];
	char inputModeStr[MAX_STR_LENGTH];
	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];

	int chanCount = 0;
	double *buffer = NULL;

	AiQueueElement queueArray[MAX_QUEUE_SIZE];

	UlError err = ERR_NO_ERROR;

	int __attribute__ ((unused))ret;
	char c;
	int i = 0;

	// Get descriptors for all of the available DAQ devices
	err = ulGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);

	if(err != ERR_NO_ERROR)
		goto end;

	// verify at least one DAQ device is detected
	if (numDevs == 0)
	{
		printf("No DAQ devices are connected\n");
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
		goto end;;
	}

	// verify the specified DAQ device supports analog input
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

	// get the queue types
	err = getAiInfoQueueTypes(daqDeviceHandle, &queueTypes);

	// get the first supported analog input mode
	err = getAiInfoFirstSupportedInputMode(daqDeviceHandle, &numberOfChannels, &inputMode, inputModeStr);

	if (highChan >= numberOfChannels)
		highChan = numberOfChannels - 1;

	chanCount = highChan - lowChan + 1;

	// does the device support a queue
	if (queueTypes == 0)
	{
		printf("\nThe specified DAQ device does not support a queue\n");
		goto end;
	}

	// get the analog input ranges
	err = getAiInfoRanges(daqDeviceHandle, inputMode, &numRanges, ranges);

	// assign each channel in the queue an input mode (SE/DIFF) and a range ... if
	// multiple ranges are supported, we will cycle through them and repeat ranges if the
	// number of channels exceeds the number of ranges
	//
	// this block of code could be used to set other queue elements such as the input mode
	// and channel list
	for (i=0; i<chanCount; i++)
	{
		queueArray[i].channel = i;
		queueArray[i].inputMode = inputMode;
		queueArray[i].range = ranges[i % numRanges];
	}

	// allocate a buffer to receive the data
	buffer = (double*) malloc(chanCount * samplesPerChannel * sizeof(double));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the device
	err = ulConnectDaqDevice(daqDeviceHandle);
	if (err != ERR_NO_ERROR)
		goto end;

	err = ulAInLoadQueue(daqDeviceHandle, queueArray, chanCount);
	if (err != ERR_NO_ERROR)
		goto end;

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulAInLoadQueue()\n");
	printf("    Channels: %d - %d\n", lowChan, highChan);
	for (i = 0; i < chanCount; i++)
	{
		ConvertInputModeToString(queueArray[i].inputMode, inputModeStr);
		ConvertRangeToString(queueArray[i].range, rangeStr);
		printf("        Channel %d:  Input mode: %s, Range = %s\n", queueArray[i].channel, inputModeStr, rangeStr);
	}
	printf("    Samples per channel: %d\n", samplesPerChannel);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	// clear the display
	ret = system("clear");

	// the range argument of the ulAInScan function is ignored becasue the ragnes are set by the ulAInLoadQueue() function,
	// however the range variable is initilized to an arbitrary range here to prevent compiler warnings
	range = BIP10VOLTS;

	// start the acquisition
	//
	// when using the queue, the lowChan, highChan, inputMode, and range
	// parameters are ignored since they are specified in queueArray
	err = ulAInScan(daqDeviceHandle, lowChan, highChan, inputMode, range, samplesPerChannel, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;

		// get the initial status of the acquisition
		ulAInScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			// get the current status of the acquisition
			err = ulAInScanStatus(daqDeviceHandle, &status, &transferStatus);

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
						printf("chan %d = %+-10.6f\n",
								i + lowChan,
								buffer[index + i]);
					}

					usleep(100000);
				}
			}
		}

		// stop the acquisition if it is still running
		if (status == SS_RUNNING && err == ERR_NO_ERROR)
		{
			err = ulAInScanStop(daqDeviceHandle);
		}
	}

	// disconnect from the device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

	// release the handle to the device
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
