/*
    UL call demonstrated:        	  ulDaqOutScan()

    Purpose:                          Continuously output user generated data
                                      on available D/A, and DIO output channels

    Demonstration:                    Outputs user generated data on available
                                      output channels

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has a DAQ output subsystem
    4. Get the channel types supported by the DAQ output subsystem
    5. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    6. Configure the analog and digital channels
    7. Call ulDaqOutScan() to start the scan
    8. Call ulDaqOutScanStatus to check the status of the background operation
    9. Display the data for each channel
    10. Call ulDaqOutScanStop() to stop the background operation
    11. Call ulDisconnectDaqDevice and ulReleaseDaqDevice() before exiting the process
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uldaq.h"
#include "utility.h"


// prototypes
UlError ConfigureAnalogOutputChannels(int numberOfChannels, Range range, DaqOutChanDescriptor* descriptors, int* scanDesriptorIndex);
UlError ConfigureDigitalOutputChannels(DaqDeviceHandle daqDeviceHandle, DaqOutChanDescriptor* descriptors, int* scanDesriptorIndex);
void CreateOutputData(int numberOfChannels, DaqOutChanDescriptor* descriptors, int numberOfSamplesPerChannel, Range range, long long bitsPerPort, double* buffer);

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

	int samplesPerChannel = 1000;
	double rate = 1000;
	Range range = BIP10VOLTS;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	DaqOutScanFlag flags = DAQOUTSCAN_FF_DEFAULT;

	int numberOfScanChannels = 2;
	int hasDAQO = 0;
	int index = 0;
	int chanTypesMask = 0;
	int bitsPerPort = 0;

	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];
	char daqoChannelTypeStr[MAX_SCAN_OPTIONS_LENGTH];
	char rangeStr[MAX_SCAN_OPTIONS_LENGTH];

	DaqOutChanDescriptor chanDescriptors[numberOfScanChannels];

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

	// verify the specified device supports analog output
	err = getDevInfoHasDaqo(daqDeviceHandle, &hasDAQO);
	if (!hasDAQO)
	{
		printf("\nThe specified DAQ device does not support DAQ output\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the channel types supported by the DAQ output subsystem
	err = getDaqoChannelTypes(daqDeviceHandle, &chanTypesMask);

	if (chanTypesMask == 0)
	{
		printf("\nDaqOutScan is not supported by the specified DAQ device\n");
		goto end;
	}

	// configure the analog channels
	if (chanTypesMask & DAQO_ANALOG)
	{
		// get the first supported output range
		getAoInfoFirstSupportedRange(daqDeviceHandle, &range, rangeStr);

		err = ConfigureAnalogOutputChannels(1, range, chanDescriptors, &scanDescriptorIndex);
	}
	else
	{
		// no analog channels so decrement the count
		numberOfScanChannels--;
	}

	// configure the digital channels
	if ((chanTypesMask & DAQO_DIGITAL) && err == ERR_NO_ERROR)
	{
		err = ConfigureDigitalOutputChannels(daqDeviceHandle, chanDescriptors, &scanDescriptorIndex);

		// get the number of bits in the port
		err = getDioInfoNumberOfBitsForFirstPort(daqDeviceHandle, &bitsPerPort);
	}
	else
	{
		// no digital channels so decrement the count
		numberOfScanChannels--;
	}

	// allocate a buffer for the output data
	buffer = (double*) malloc(numberOfScanChannels * samplesPerChannel * sizeof(double));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	// fill the buffer with data
	CreateOutputData(numberOfScanChannels, chanDescriptors, samplesPerChannel, range, bitsPerPort, buffer);

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulDaqOutScan()\n");
	printf("    Number of scan channels: %d\n", numberOfScanChannels);
	for (i = 0; i < numberOfScanChannels; i++)
	{
		ConvertDaqOChanTypeToString(chanDescriptors[i].type, daqoChannelTypeStr);
		if (chanDescriptors[i].type == DAQO_ANALOG)
		{
			ConvertRangeToString(chanDescriptors[i].range, rangeStr);
			printf("        ScanChannel %d: type = %s, channel = %d, range = %s\n", i, daqoChannelTypeStr, chanDescriptors[i].channel, rangeStr);
		}
		else
		{
			printf("        ScanChannel %d: type = %s, channel = %d\n", i, daqoChannelTypeStr, chanDescriptors[i].channel);
		}
	}
	printf("    Samples per channel: %d\n", samplesPerChannel);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	// start the output
	err = ulDaqOutScan(daqDeviceHandle, chanDescriptors, numberOfScanChannels, samplesPerChannel, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;

		// get the initial status of the background operation
		ulDaqOutScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			// get the current status of the background operation
			err = ulDaqOutScanStatus(daqDeviceHandle, &status, &transferStatus);

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

				usleep(100000);
			}
		}

		ulDaqOutScanStop(daqDeviceHandle);
	}

	// before leaving, configure the digital port for input
	for (i = 0; i < numberOfScanChannels; i++)
	{
		if (chanDescriptors[i].type == DAQO_DIGITAL)
		{
			err = ulDConfigPort(daqDeviceHandle, (DigitalPortType) chanDescriptors[i].channel, DD_INPUT);
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

UlError ConfigureAnalogOutputChannels(int numberOfChannels, Range range, DaqOutChanDescriptor* descriptors, int* scanDesriptorIndex)
{
	UlError err = ERR_NO_ERROR;
	int numbeOfAnalogChans = 0;
	int i;
	int index = *scanDesriptorIndex;

		// fill a descriptor for each channel
	for (i = 0; i< numberOfChannels; i++)
	{
		descriptors[index].channel = i;
		descriptors[index].type = DAQO_ANALOG;
		descriptors[index].range = range;
		index++;

		numbeOfAnalogChans++;

		if (numbeOfAnalogChans == numberOfChannels)
			break;
	}

	*scanDesriptorIndex = index;

	return err;
}

UlError ConfigureDigitalOutputChannels(DaqDeviceHandle daqDeviceHandle, DaqOutChanDescriptor* descriptors, int* scanDesriptorIndex)
{
	UlError err = ERR_NO_ERROR;
	DigitalPortType portType;
	char portTypeStr[MAX_STR_LENGTH];
	int index = *scanDesriptorIndex;

	err = getDioInfoFirstSupportedPortType(daqDeviceHandle, &portType, portTypeStr);

	// configure the port for output
	err = ulDConfigPort(daqDeviceHandle, portType, DD_OUTPUT);

	descriptors[index].channel = portType;
	descriptors[index].type = DAQO_DIGITAL;
	index++;

	*scanDesriptorIndex = index;

	return err;
}

void CreateOutputData(int numberOfChannels, DaqOutChanDescriptor* descriptors, int numberOfSamplesPerChannel, Range range, long long bitsPerPort, double* buffer)
{
	const double twoPI = 2 * M_PI;
	int sample, channel;

	double min = 0.0;
	double max = 0.0;
	ConvertRangeToMinMax(range, &min, &max);

	double f = twoPI / numberOfSamplesPerChannel;
	double analogAmplitude =  max - min ;
	double digitalAmplitude = pow((double)2.0, bitsPerPort) - 1;;
	double phase = 0.0;

	int index = 0;

	for (sample = 0; sample < numberOfSamplesPerChannel; sample++)
	{
		for (channel = 0; channel  < numberOfChannels; channel++)
		{
			if (descriptors[channel].type == DAQO_ANALOG)
			{
				double value = (double) (sin(phase) * analogAmplitude/2);

				if (channel== 0)
				{
					buffer[index++] = value;
				}
				else
				{
					buffer[index++] = value/2;
				}
			}
			else
			{
				buffer[index++] = (sin(phase) * digitalAmplitude/2 + (digitalAmplitude / 2));
			}
		}

		phase += f;
		if( phase > twoPI )
			phase = phase - twoPI;
	}
}

