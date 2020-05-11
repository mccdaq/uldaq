/*
    UL call demonstrated:        	  ulDaqInScan()

    Purpose:                          Performs a continuous scan of the available
     	 	 	 	 	 	 	 	  analog, digital, and/or counter input subsystems

    Demonstration:                    Displays the analog, digital, and counter
                                      input data on the specified channels

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an DAQ input subsystem
    4. Get the channel types supported by the DAQ input subsystem
    5. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    6. Configure the available analog, digital, and counter channels
    7. Call ulDaqInScan() to start the scan
    8. Call ulDaqInScanStatus to check the status of the background operation
    9. Display the data for each channel
    10. Call ulDaqInScanStop() to stop the background operation
    11. Call ulDisconnectDaqDevice and ulReleaseDaqDevice() before exiting the process
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"


// prototypes
UlError ConfigureAnalogInputChannels(int numberOfChannels, Range range, DaqInChanDescriptor* descriptors, int* scanDesriptorIndex);
UlError ConfigureDigitalInputChannel(DaqDeviceHandle daqDeviceHandle, DaqInChanDescriptor* descriptors, int* scanDesriptorIndex);
UlError ConfigureCounterInputChannels(int numberOfChannels, DaqInChanDescriptor* descriptors, int* scanDesriptorIndex);

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64
#define MAX_SCAN_OPTIONS_LENGTH 256
#define MAX_SCAN_CHAN_COUNT 64

int main(void)
{
	int descriptorIndex = 0;
	AiInputMode inputMode = AI_SINGLE_ENDED;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	int samplesPerChannel = 10000;
	double rate = 1000;
	Range range = BIP10VOLTS;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	DaqInScanFlag flags = DAQINSCAN_FF_DEFAULT;

	int numberOfAiChannels = 0;
	int numberOfScanChannels = 0;
	int hasDAQI = 0;
	int index = 0;
	int chanTypesMask = 0;

	char inputModeStr[MAX_STR_LENGTH];
	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];
	char daqiChannelTypeStr[MAX_SCAN_OPTIONS_LENGTH];
	char rangeStr[MAX_SCAN_OPTIONS_LENGTH];

	DaqInChanDescriptor chanDescriptors[MAX_SCAN_CHAN_COUNT];

	int scanDescriptorIndex = 0;

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

	// verify the specified device supports analog input
	err = getDevInfoHasDaqi(daqDeviceHandle, &hasDAQI);
	if (!hasDAQI)
	{
		printf("\nThe specified DAQ device does not support DAQ input\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the channel types supported by the DAQ input subsystem
	err = getDaqiChannelTypes(daqDeviceHandle, &chanTypesMask);

	if (chanTypesMask == 0)
	{
		printf("\nDaqInScan is not supported by the specified DAQ device\n");
		goto end;
	}

	// configure the analog channels
	if (chanTypesMask & DAQI_ANALOG_SE)
	{
		// get the first supported analog input mode
		err = getAiInfoFirstSupportedInputMode(daqDeviceHandle, &numberOfAiChannels, &inputMode, inputModeStr);

		// get the first supported input range
		getAiInfoFirstSupportedRange(daqDeviceHandle, inputMode, &range, rangeStr);

		err = ConfigureAnalogInputChannels(2, range, chanDescriptors, &scanDescriptorIndex);

		/* uncomment this section to enable IEPE mode for the specified analog channel and set coupling mode to AC

		// enable IEPE mode for the specified channel
		err = ulAISetConfig(daqDeviceHandle, AI_CFG_CHAN_IEPE_MODE, chanDescriptors[0].channel, IEPE_ENABLED);

		// set coupling mode to AC for the specified channel
		err = ulAISetConfig(daqDeviceHandle, AI_CFG_CHAN_COUPLING_MODE, chanDescriptors[0].channel, CM_AC);
	
		// set sensor sensitivity for the specified channel
		err = ulAISetConfigDbl(daqDeviceHandle, AI_CFG_CHAN_SENSOR_SENSITIVITY,  chan, 1.0);*/
	}

	// configure the digital channels
	if ((chanTypesMask & DAQI_DIGITAL) && err == ERR_NO_ERROR)
	{
		err = ConfigureDigitalInputChannel(daqDeviceHandle, chanDescriptors, &scanDescriptorIndex);
	}

	// configure the counter channels
	if ((chanTypesMask & DAQI_CTR32) && err == ERR_NO_ERROR)
	{
		err = ConfigureCounterInputChannels(1, chanDescriptors, &scanDescriptorIndex);
	}
	

	numberOfScanChannels = scanDescriptorIndex;

	// allocate a buffer to receive the data
	buffer = (double*) malloc(numberOfScanChannels * samplesPerChannel * sizeof(double));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulDaqInScan()\n");
	printf("    Number of scan channels: %d\n", numberOfScanChannels);
	for (i=0; i<numberOfScanChannels; i++)
	{
		ConvertDaqIChanTypeToString(chanDescriptors[i].type, daqiChannelTypeStr);
		if (chanDescriptors[i].type == DAQI_ANALOG_SE || chanDescriptors[i].type == DAQI_ANALOG_DIFF)
		{
			ConvertRangeToString(chanDescriptors[i].range, rangeStr);
			printf("        ScanChannel %d: type = %s, channel = %d, range = %s\n", i, daqiChannelTypeStr, chanDescriptors[i].channel, rangeStr);
		}
		else
		{
			printf("        ScanChannel %d: type = %s, channel = %d\n", i, daqiChannelTypeStr, chanDescriptors[i].channel);
		}
	}
	printf("    Samples per channel: %d\n", samplesPerChannel);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	// start the acquisition
	err = ulDaqInScan(daqDeviceHandle, chanDescriptors, numberOfScanChannels, samplesPerChannel, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;
		int i = 0;

		// get the initial status of the acquisition
		ulDaqInScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			// get the current status of the acquisition
			err = ulDaqInScanStatus(daqDeviceHandle, &status, &transferStatus);

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
				for (i = 0; i < numberOfScanChannels; i++)
				{
					if (chanDescriptors[i].type == DAQI_ANALOG_SE || chanDescriptors[i].type == DAQI_ANALOG_DIFF)
					{
						printf("chan (%s%d) = %+-10.6f\n",
								"Ai", chanDescriptors[i].channel,
								buffer[index + i]);
					}
					else
					{
						printf("chan (%s%d) = %-10lld \n",
								(chanDescriptors[i].type == DAQI_DIGITAL) ? "Di" : "Ci", chanDescriptors[i].channel,
								(long long)buffer[index + i]);
					}
				}

				usleep(100000);
			}
		}

		// stop the acquisition if it is still running
		if (status == SS_RUNNING && err == ERR_NO_ERROR)
		{
			err = ulDaqInScanStop(daqDeviceHandle);
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

UlError ConfigureAnalogInputChannels(int numberOfChannels, Range range, DaqInChanDescriptor* descriptors, int* scanDesriptorIndex)
{
	UlError err = ERR_NO_ERROR;
	int i;
	int index = *scanDesriptorIndex;

	// fill a descriptor for each channel
	for ( i = 0; i < numberOfChannels; i++)
	{
		descriptors[index].channel = i;
		descriptors[index].type = DAQI_ANALOG_SE;
		descriptors[index].range = range;
		index++;
	}

	*scanDesriptorIndex = index;

	return err;
}

UlError ConfigureDigitalInputChannel(DaqDeviceHandle daqDeviceHandle, DaqInChanDescriptor* descriptors, int* scanDesriptorIndex)
{
	UlError err = ERR_NO_ERROR;
	DigitalPortType portType;
	char portTypeStr[MAX_STR_LENGTH];
	int index = *scanDesriptorIndex;

	err = getDioInfoFirstSupportedPortType(daqDeviceHandle, &portType, portTypeStr);

	// configure the port for input
	err = ulDConfigPort(daqDeviceHandle, portType, DD_INPUT);

	descriptors[index].channel = portType;
	descriptors[index].type = DAQI_DIGITAL;
	index++;

	*scanDesriptorIndex = index;

	return err;
}

UlError ConfigureCounterInputChannels(int numberOfChannels, DaqInChanDescriptor* descriptors, int* scanDesriptorIndex)
{
	UlError err = ERR_NO_ERROR;
	int i;
	int index = *scanDesriptorIndex;

	// fill a descriptor for each channel
	for (i = 0; i < numberOfChannels; i++)
	{
		descriptors[index].channel = i;
		descriptors[index].type = DAQI_CTR32;
		index++;
	}

	*scanDesriptorIndex = index;

	return err;
}
