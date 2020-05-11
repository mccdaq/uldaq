/*
    UL call demonstrated:             ulGetNetDaqDeviceDescriptor()

    Purpose:                          Discovers remote Ethernet DAQ devices.

                                      Note: If the host system and Ethernet DAQ device
									  are in the same subnet then the ulGetDaqDeviceInventory()
                                      function can be used to detect the Ethernet devices.

    Demonstration:                    Discovers a remote Ethernet DAQ device at the specified address
                                      and flashes the LED of the detected Ethernet device


    Steps:
    1. Enter the address of an Ethernet DAQ device
    2. Call ulGetNetDaqDeviceDescriptor() to obtain the descriptor of the Ethernet DAQ device
    3. Call ulCreateDaqDevice() to to get a handle for the DAQ device
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Call ulFlashLed() to flash the LED of the Ethernet DAQ device
    6. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

int main(void)
{
	DaqDeviceDescriptor devDescriptor;
	DaqDeviceHandle daqDeviceHandle = 0;

	UlError err = ERR_NO_ERROR;

	unsigned short port = 54211;
	char host[128] = {0};
	const char* ifcName = NULL;
	double timeout = 5.0; //seconds

	int __attribute__((unused)) ret;
	char c;

	printf ("Enter the host name or IP address of DAQ device: ");
	ret = scanf ("%127s%c", host,&c);

	printf ("\nEnter the port number of DAQ device: ");
	ret = scanf ("%hu", &port);

	printf("\nDiscovering DAQ device at the specified address: %s - please wait ...\n\n", host);

	// Get the descriptor of the specified Ethernet DAQ device
	err = ulGetNetDaqDeviceDescriptor(host, port, ifcName, &devDescriptor, timeout);

	if (err != ERR_NO_ERROR)
		goto end;

	printf("DAQ device discovered\n");
	printf("  %s: (%s)\n", devDescriptor.productName, devDescriptor.uniqueId);

	// get a handle to the DAQ device associated with the descriptor
	daqDeviceHandle = ulCreateDaqDevice(devDescriptor);

	if (daqDeviceHandle == 0)
	{
		printf ("\nUnable to create a handle to the specified DAQ device\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptor.devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	printf("\n%s ready\n", devDescriptor.devString);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	// clear the display
	ret = system("clear");

	while(err == ERR_NO_ERROR && !enter_press())
	{
		// reset the cursor to the top of the display and
		// show the termination message
		resetCursor();
		printf("Flashing DAQ device's LED ...\n\n");

		printf("Hit 'Enter' to terminate the process\n");

		err = ulFlashLed(daqDeviceHandle, 1);
		usleep(250000);
	}

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

	// release the handle to the DAQ device
	if(daqDeviceHandle)
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

