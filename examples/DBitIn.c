/*
    UL call demonstrated:        	  ulDBitIn()

    Purpose:                          Reads the values of the bits for the
    								  first digital port

    Demonstration:                    Displays the value of each bit in the
    								  first digital port

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an digital input subsystem
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Get the first supported digital port
    6. Get the number of bits for the digital port
    7. Call ulDConfigBit (if supported - otherwise - ulDConfigPort) to configure each bit for input
    8. Call ulDBitIn() to read a value for each bit in the digital port
    9. Display the data for each bit
    10. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64

int main(void)

{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	int hasDIO = 0;
	int bitsPerPort = 0;
	DigitalPortType portType;
	DigitalPortIoType portIoType;

	char portTypeStr[MAX_STR_LENGTH];
	char portIoTypeStr[MAX_STR_LENGTH];
	int bitNumber;

	unsigned int data = 0;
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

	// verify the specified DAQ device supports digital input
	err = getDevInfoHasDio(daqDeviceHandle, &hasDIO);
	if (!hasDIO)
	{
		printf("\nThe specified DAQ device does not support digital I/O\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	// get the first port type (AUXPORT0, FIRSTPORTA, ...)
	err = getDioInfoFirstSupportedPortType(daqDeviceHandle, &portType, portTypeStr);

	// get the I/O type for the fisrt port
	err = getDioInfoFirstSupportedPortIoType(daqDeviceHandle, &portIoType, portIoTypeStr);

	// get the number of bits for the first port (port index = 0)
	err = getDioInfoNumberOfBitsForFirstPort(daqDeviceHandle, &bitsPerPort);

	// if the port is bit configurable, then configure the individual bits
	// for input; otherwise, configure the entire port for input
	if (portIoType == DPIOT_BITIO)
	{
		// configure all of the bits for input for the port
		for (bitNumber= 0; bitNumber < bitsPerPort; bitNumber++)
		{
			err = ulDConfigBit(daqDeviceHandle, portType, bitNumber, DD_INPUT);
			if (err != ERR_NO_ERROR)
				break;
		}
	}
	else if (portIoType == DPIOT_IO)
	{
		// configure the entire port for input
		err = ulDConfigPort(daqDeviceHandle, portType, DD_INPUT);
	}

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulDBitIn()\n");
	printf("    Port: %s\n", portTypeStr);
	printf("    Port I/O type: %s\n", portIoTypeStr);
	printf("    Bits: %d\n", bitsPerPort);
	printf("Hit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	while(err == ERR_NO_ERROR && !enter_press())
	{
		resetCursor();
		printf("Hit 'Enter' to terminate the process\n\n");
		printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);

		// read each of the bits from the first port
		for (bitNumber = 0; bitNumber < bitsPerPort; bitNumber++)
		{
			err = ulDBitIn(daqDeviceHandle, portType, bitNumber, &data);

			if(err == ERR_NO_ERROR)
			{
				printf("Bit Number: %d:  Data: %d\n", bitNumber, data);
			}
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

