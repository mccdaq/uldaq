/*
    UL call demonstrated:       	  ulAOutScan()

    Purpose:                          Continuously output a waveform
                                      on a D/A output channel

    Demonstration:                    Outputs user generated data
                                      on analog output channel 0

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an analog output subsystem
    3. Verify the analog output subsystem has a hardware pacer
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Get the first supported analog range
    6. Call ulAOutScan() to start the scan of D/A output channels
    7. Call ulAOutScanStatus() to check the status of the background operation
    8. Call ulAOutScanStop() to stop the background operation
    9. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uldaq.h"
#include "utility.h"

// prototypes
void CreateOutputData(int numberOfSamplesPerChannel, int chanCount, Range range, double* buffer);

// constants
#define MAX_DEV_COUNT  100

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	int lowChan = 0;
	int highChan = 0;
	Range range;
	const int samplesPerChannel = 1000;
	double rate = 1000;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	AOutScanFlag flags = AOUTSCAN_FF_DEFAULT;

	int hasAO = 0;
	int hasPacer = 0;
	int index = 0;

	char rangeStr[MAX_STR_LENGTH];

	// allocate a buffer for the data to be output
	const int chanCount = highChan - lowChan + 1;
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

	// get a handle to  the DAQ device associated with the first descriptor
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
		printf("\nThis device does not support analog output\n");
		goto end;
	}

	// verify the specified DAQ device supports hardware pacing for analog output
	err = getAoInfoHasPacer(daqDeviceHandle, &hasPacer);
	if (!hasPacer)
	{
		printf("\nThe specified DAQ device does not support hardware paced analog output\n");
		goto end;
	}

	// allocate a buffer for the output data
	buffer = (double*) malloc(chanCount * samplesPerChannel * sizeof(double));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	// get the first supported output range
	err = getAoInfoFirstSupportedRange(daqDeviceHandle, &range, rangeStr);

	// fill the buffer with data
	CreateOutputData(samplesPerChannel, chanCount, range, buffer);

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// create a connection to the device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulAOutscan()\n");
	printf("    Channels: %d - %d\n", lowChan, highChan);
	printf("    Range: %s\n", rangeStr);
	printf("    Samples per channel: %d\n", samplesPerChannel);
	printf("    Rate: %f\n", rate);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	// clear the display
	ret = system("clear");

	// start the output
	err = ulAOutScan(daqDeviceHandle, lowChan, highChan, range, samplesPerChannel, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;

		// get the initial status of the acquisition
		ulAOutScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			// get the current status of the acquisition
			err = ulAOutScanStatus(daqDeviceHandle, &status, &transferStatus);

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

		// stop the output if it is still running
		if (status == SS_RUNNING && err == ERR_NO_ERROR)
		{
			err = ulAOutScanStop(daqDeviceHandle);
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

void CreateOutputData(int numberOfSamplesPerChannel, int chanCount, Range range, double* buffer)
{
	const double twoPI = 2 * M_PI;
	int i, j;

	double min = 0.0;
	double max = 0.0;
	ConvertRangeToMinMax(range, &min, &max);

	double amplitude =  max - min;
	double f = twoPI / numberOfSamplesPerChannel;
	double phase = 0.0;
	double offset = (min == 0) ? amplitude/2 : 0;

	for (i=0; i<numberOfSamplesPerChannel; i++)
	{
		for (j=0; j<chanCount; j++)
		{
			*buffer++ = (double) ((sin(phase) * amplitude/2) + offset);
		}

		phase += f;
		if( phase > twoPI )
			phase = phase - twoPI;
	}
}

