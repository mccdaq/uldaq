/*
    UL call demonstrated:       	  ulDOutScan()

    Purpose:                          Continuously output user generated
                                      data on the first digital port

    Demonstration:                    Outputs user specified data
                                      on the first digital port

    Steps:digital
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an digital output subsystem
    4. Verify the digital output subsystem has a hardware pacer
    5. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    6. Call ulDConfigPort to configure the port for output
    7. Generate the data to be output
    8. Call ulDOutScan() to start the scan for the DIO subsystem
    9. Call ulDOutScanStatus() to check the status of the background operation
    10. Call ulDOutScanStop() to stop the background operation
    11. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uldaq.h"
#include "utility.h"

// prototypes
void CreateOutputData(int samplesPerPort, long long bitsPerPort, unsigned long long* buffer);

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

	DigitalPortType lowPort;
	DigitalPortType highPort;
	const int samplesPerPort = 1000;
	double rate = 1000;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	DOutScanFlag flags = DOUTSCAN_FF_DEFAULT;

	int hasDIO = 0;
	int hasPacer = 0;
	int index = 0;
	int bitsPerPort = 0;

	char portTypeStr[MAX_STR_LENGTH];
	char scanOptionsStr[MAX_SCAN_OPTIONS_LENGTH];

	DigitalPortType portType;

	int chanCount = 1;
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

	// verify the specified DAQ device supports the DIO subsystem
	err = getDevInfoHasDio(daqDeviceHandle, &hasDIO);
	if (!hasDIO)
	{
		printf("\nThis device does not support digital I/O\n");
		goto end;
	}

	// verify the specified DAQ device supports hardware output pacing
	err = getDioInfoHasPacer(daqDeviceHandle, DD_OUTPUT, &hasPacer);
	if (!hasPacer)
	{
		printf("\nThe specified DAQ device does not support hardware paced digital output\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the first supported port type
	err = getDioInfoFirstSupportedPortType(daqDeviceHandle, &portType, portTypeStr);
	lowPort = portType;
	highPort = portType;

	// get the number of bits in the port
	err = getDioInfoNumberOfBitsForFirstPort(daqDeviceHandle, &bitsPerPort);

	err = ulDConfigPort(daqDeviceHandle, portType, DD_OUTPUT);

	// allocate a buffer for the output data
	buffer = (unsigned long long*) malloc(chanCount * samplesPerPort * sizeof(unsigned long long));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	CreateOutputData(samplesPerPort, bitsPerPort, buffer);

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulDOutscan()\n");
	printf("    Port: %s\n", portTypeStr);
	printf("    Bits per port: %d\n", bitsPerPort);
	printf("    Samples per port: %d\n", samplesPerPort);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	// start the output
	err = ulDOutScan(daqDeviceHandle, lowPort, highPort, samplesPerPort, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;

		ulDOutScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			err = ulDOutScanStatus(daqDeviceHandle, &status, &transferStatus);

			if(err == ERR_NO_ERROR)
			{
				resetCursor();
				printf("Hit 'Enter' to terminate the process\n\n");
				printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);
				printf("actual scan rate = %f\n\n", rate);

				index = transferStatus.currentIndex;
				printf("currentScanCount =  %-10llu \n", transferStatus.currentScanCount);
				printf("currentTotalCount = %-10llu \n", transferStatus.currentTotalCount);
				printf("currentIndex =      %-10d \n\n", index);

				usleep(10000);
			}
		}

		ulDOutScanStop(daqDeviceHandle);
	}

	// before leaving, configure the entire first port for input
	err = ulDConfigPort(daqDeviceHandle, portType, DD_INPUT);

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

void CreateOutputData(int samplesPerPort, long long bitsPerPort, unsigned long long* buffer)
{
	const double twoPI = 2 * M_PI;
	int sample;

	double amplitude = pow((double)2.0, bitsPerPort) - 1;
	double f = twoPI / samplesPerPort;
	double phase = 0.0;

	for (sample=0; sample<samplesPerPort; sample++)
	{
		*buffer++ = (unsigned long long) (sin(phase) * amplitude/2 + (amplitude / 2));
		phase += f;
		if( phase > twoPI )
			phase = phase - twoPI;
	}
}

