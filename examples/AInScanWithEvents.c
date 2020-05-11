/*
    UL call demonstrated:        	  ulEnableEvent() and ulAInScanWait()

    Purpose:                          Use events to determine when data is available

    Demonstration:                    Use the a callback function to display the
    								  the data for a user-specified range of
    								  A/D channels

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an analog input subsystem
    3. Verify the analog input subsystem has a hardware pacer
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Initialize the ScanEventParameters structure used to pass parameters to the callback function
    6. Call ulEnableEvent() to enable the DE_ON_DATA_AVAILABLE event
    7. Call ulAInScan() to start the finite scan of A/D input channels
    8. Call ulAInScanWait() to wait for the finite scan to complete
    9. The callback is called each time 100 samples are available to allow the data to be displayed
    10. Call ulAinScanStop() to stop the background operation
    11. Call ulDisconnectDaqDevice and ulReleaseDaqDevice() before exiting the process
*/
#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64
#define MAX_SCAN_OPTIONS_LENGTH 256

void eventCallbackFunction(DaqDeviceHandle daqDeviceHandle, DaqEventType eventType, unsigned long long eventData, void* userData);

struct ScanEventParameters
{
	double* buffer;	// data buffer
	int bufferSize;	// data buffer size
	int lowChan;	// first channel in acquisition
	int highChan;	// last channel in acquisition
	double rate;
};
typedef struct ScanEventParameters ScanEventParameters;

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;
	ScanEventParameters scanEventParameters;

	// set some variables that are used to acquire data
	int lowChan = 0;
	int highChan = 3;
	AiInputMode inputMode;
	Range range;
	int samplesPerChannel = 10000;
	double rate = 100;
	AInScanFlag flags = AINSCAN_FF_DEFAULT;
	DaqEventType eventTypes = (DaqEventType) (DE_ON_DATA_AVAILABLE | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_INPUT_SCAN);

	// set the scan options for a FINITE scan ... to set the scan options for
	// a continuous scan, uncomment the line that or's the SO_CONTINUOUS option
	// into to the scanOptions variable
	//
	// if this is changed to a CONTINUOUS scan, then changes will need to be made
	// to the event handler (eventCallbackFunction) to account for the buffer wrap
	// around condition
	ScanOption scanOptions = SO_DEFAULTIO;
	//scanOptions = (ScanOption) (scanOptions | SO_CONTINUOUS);

	int hasAI = 0;
	int hasPacer = 0;
	int numberOfChannels = 0;
	int availableSampleCount = 0;

	char inputModeStr[MAX_STR_LENGTH];
	char rangeStr[MAX_STR_LENGTH];
	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];

	// allocate a buffer to receive the data
	int chanCount = 0;
	int bufferSize;
	double* buffer = NULL;
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

	// verify the specified DAQ device supports analog input
	err = getDevInfoHasAi(daqDeviceHandle, &hasAI);
	if (!hasAI)
	{
		printf("\nThe specified DAQ device does not support analog input\n");
		goto end;
	}

	// verify the device supports hardware pacing for analog input
	err = getAiInfoHasPacer(daqDeviceHandle, &hasPacer);

	if (!hasPacer)
	{
		printf("The specified DAQ device does not support hardware paced analog input\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the first supported analog input mode
	err = getAiInfoFirstSupportedInputMode(daqDeviceHandle, &numberOfChannels, &inputMode, inputModeStr);

	if (highChan >= numberOfChannels)
		highChan = numberOfChannels - 1;

	chanCount = highChan - lowChan + 1;

	// allocate a buffer to receive the data
	bufferSize = chanCount * samplesPerChannel;
	buffer = (double*) malloc(bufferSize * sizeof(double));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	// store the scan event parameters for use in the callback function
	scanEventParameters.buffer =  buffer;
	scanEventParameters.bufferSize =  bufferSize;
	scanEventParameters.lowChan = lowChan;
	scanEventParameters.highChan = highChan;

	// enable the event to be notified every time 100 samples are available
	availableSampleCount = 100;
	err = ulEnableEvent(daqDeviceHandle, eventTypes, availableSampleCount, eventCallbackFunction, &scanEventParameters);

	// get the first supported analog input range
	err = getAiInfoFirstSupportedRange(daqDeviceHandle, inputMode, &range, rangeStr);

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulEnableEvent()\n");
	printf("    Channels: %d - %d\n", lowChan, highChan);
	printf("    Input mode: %s\n", inputModeStr);
	printf("    Range: %s\n", rangeStr);
	printf("    Samples per channel: %d\n", samplesPerChannel);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	// clear the display
	ret = system("clear");

	// start the finite acquisition
	err = ulAInScan(daqDeviceHandle, lowChan, highChan, inputMode, range, samplesPerChannel, &rate, scanOptions, flags, buffer);

	scanEventParameters.rate = rate;

	if(err == ERR_NO_ERROR)
	{
		// wait here until scan is done ... events will be handled in the event handler
		// (eventCallbackFunction) until the background scan completes
		int timeToWait = -1;
		err = ulAInScanWait(daqDeviceHandle, WAIT_UNTIL_DONE, 0, timeToWait);

		if (err != ERR_NO_ERROR)
		{
			printf ("ulAInScanWait error = %d\n", err);
		}

		err = ulAInScanStop(daqDeviceHandle);
	}

	// disable events
	ulDisableEvent(daqDeviceHandle, eventTypes);

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

	// release the handle to the DAQ device
	if(daqDeviceHandle)
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

void eventCallbackFunction(DaqDeviceHandle daqDeviceHandle, DaqEventType eventType, unsigned long long eventData, void* userData)
{
	char eventTypeStr[MAX_STR_LENGTH];
	UlError err = ERR_NO_ERROR;
	DaqDeviceDescriptor activeDevDescriptor;
	int i;

	ScanEventParameters* scanEventParameters = (ScanEventParameters*) userData;
	int chanCount = scanEventParameters->highChan  - scanEventParameters->lowChan + 1;
	int newlineCount = chanCount + 7;

	// reset the cursor to the top of the display and
	// show the termination message
	resetCursor();

	ulGetDaqDeviceDescriptor(daqDeviceHandle, &activeDevDescriptor);

	printf("Hit 'Enter' to terminate the process\n\n");
	printf("Active DAQ device: %s (%s)\n\n", activeDevDescriptor.productName, activeDevDescriptor.uniqueId);

	ConvertEventTypesToString(eventType, eventTypeStr);
	printf("eventType: %s \n", eventTypeStr);

	if (eventType == DE_ON_DATA_AVAILABLE)
	{
		long long scanCount = eventData;
		long long totalSamples = scanCount * chanCount;
		long long index = 0;

		printf("eventData: %lld \n\n", eventData);
		printf("actual scan rate = %f\n\n", scanEventParameters->rate);

		// if this example is changed to a CONTINUOUS scan, then you will need
		// to maintain the index of where the data is being written to the buffer
		// to handle the buffer wrap around condition
		index = (totalSamples - chanCount) % scanEventParameters->bufferSize;

		printf("currentIndex = %lld \n\n", index);

		for (i = 0; i < chanCount; i++)
		{
			printf("chan %d = %+-10.6f\n",
					i + scanEventParameters->lowChan,
					scanEventParameters->buffer[index + i]);
		}
	}

	if(eventType == DE_ON_INPUT_SCAN_ERROR)
	{
		for (i = 0; i < newlineCount; i++)
			putchar('\n');

		err = (UlError) eventData;
		char errMsg[ERR_MSG_LEN];
		ulGetErrMsg(err, errMsg);
		printf("Error Code: %d \n", err);
		printf("Error Message: %s \n", errMsg);
	}


	if (eventType == DE_ON_END_OF_INPUT_SCAN)
	{
		for (i = 0; i < newlineCount; i++)
			putchar('\n');

		printf("\nThe scan using device %s (%s) is complete \n", activeDevDescriptor.productName, activeDevDescriptor.uniqueId);
	}

	// stop the acquisition if the Enter key is pressed
	if (enter_press())
	{
		err = ulAInScanStop(daqDeviceHandle);
	}
}



