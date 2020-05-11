/*
    UL call demonstrated:       	  ulDInScan()

    Purpose:                          Performs a continuous scan of the
    								  first digital port

    Demonstration:                    Displays the digital input data for
    								  the first digital port

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an digital input subsystem
    4. Verify the digital input subsystem has a hardware pacer
    5. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    6. Call ulDConfigPort to configure the port for input
    7. Call ulDInScan() to start the scan of the DIO subsystem
    8. Call ulDInScanStatus() to check the status of the background operation
    9. Display the data for each channel
    10. Call ulDInScanStop() to stop the background operation
    11. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
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

	DigitalPortType lowPort;
	DigitalPortType highPort;
	int samplesPerPort = 1000;
	double rate = 100;
	ScanOption scanOptions = (ScanOption) (SO_DEFAULTIO | SO_CONTINUOUS);
	DInScanFlag flags = DINSCAN_FF_DEFAULT;

	int hasDIO = 0;
	int hasPacer = 0;
	int index = 0;

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
	{
		goto end;
	}

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

	// verify the specified DAQ device supports digital input
	err = getDevInfoHasDio(daqDeviceHandle, &hasDIO);
	if (!hasDIO)
	{
		printf("\nThe specified DAQ device does not support digital I/O\n");
		goto end;
	}

	// verify the device supports hardware pacing
	err = getDioInfoHasPacer(daqDeviceHandle, DD_INPUT, &hasPacer);
	if (!hasPacer)
	{
		printf("\nThe specified DAQ device does not support hardware paced digital input\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	err = getDioInfoFirstSupportedPortType(daqDeviceHandle, &portType, portTypeStr);

	lowPort = portType;
	highPort = portType;

	err = ulDConfigPort(daqDeviceHandle, portType, DD_INPUT);

	// allocate a buffer to receive the data
	buffer = (unsigned long long* ) malloc(chanCount * samplesPerPort * sizeof(unsigned long long));

	if(buffer == NULL)
	{
		printf("\nOut of memory, unable to create scan buffer\n");
		goto end;
	}

	ConvertScanOptionsToString(scanOptions, scanOptionsStr);

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulDInScan()\n");
	printf("    Port: %s\n", portTypeStr);
	printf("    Samples per port: %d\n", samplesPerPort);
	printf("    Rate: %f\n", rate);
	printf("    Scan options: %s\n", scanOptionsStr);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	err = ulDInScan(daqDeviceHandle, lowPort, highPort, samplesPerPort, &rate, scanOptions, flags, buffer);

	if(err == ERR_NO_ERROR)
	{
		ScanStatus status;
		TransferStatus transferStatus;
		int i = 0;

		ulDInScanStatus(daqDeviceHandle, &status, &transferStatus);

		while(status == SS_RUNNING && err == ERR_NO_ERROR && !enter_press())
		{
			err = ulDInScanStatus(daqDeviceHandle, &status, &transferStatus);

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

				if(index >= 0)
				{
					// display the data
					for (i = 0; i < chanCount; i++)
					{
						clearEOL();
						printf("chan %d = 0x%llx\n",
								i + lowPort,
								buffer[index + i]);
					}

					usleep(100000);
				}
			}
		}

		ulDInScanStop(daqDeviceHandle);
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
