/*
    UL call demonstrated:        	  ulTIn()

    Purpose:                          Reads the user-specified temperature input channels

    Demonstration:                    Displays the temperature data for each of
                                      the user-specified channels

    Steps:
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has an analog input subsystem
    4. Verify the DAQ device supports temperature input
    5. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    6. Call ulTIn() for each channel specified to read a value from the A/D input channel
    7. Display the data for each channel
    8. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64

void configureChannels(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan);

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	// set some variables used to acquire data
	int lowChan = 0;
	int highChan = 3;
	int chan = 0;
	TempScale scale = TS_CELSIUS;
	TInFlag flags = TIN_FF_DEFAULT;

	int hasAI = 0;
	int hasTempChan = 0;
	int numberOfChannels = 0;

	char chanTypeStr[MAX_STR_LENGTH];
	char sensorStr[MAX_STR_LENGTH];

	int i = 0;
	double data = 0;
	UlError err = ERR_NO_ERROR;

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

	// verify the specified DAQ device has temperature channels
	err = getAiInfoHasTempChan(daqDeviceHandle, &hasTempChan, &numberOfChannels);
	if (!hasTempChan)
	{
		printf("\nThe specified DAQ device does not have a temperature channel\n");
		goto end;
	}

	if (highChan >= numberOfChannels)
		highChan = numberOfChannels - 1;

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulTIn()\n");
	printf("    Channels: %d - %d\n", lowChan, highChan);

	// configure the analog input channels if required
	configureChannels(daqDeviceHandle, lowChan, highChan);

	for (chan = lowChan; chan <= highChan; chan++)
	{
		getAiConfigTempChanConfig(daqDeviceHandle, chan, chanTypeStr, sensorStr);
		printf("    Channel %d: type: %s (%s)\n", chan, chanTypeStr, sensorStr);
	}

	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	while(err == ERR_NO_ERROR && !enter_press())
	{
		// reset the cursor to the top of the display and
		// show the termination message
		resetCursor();
		printf("Hit 'Enter' to terminate the process\n\n");
		printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);

		// display data for the first 4 analog input channels
		for (chan = lowChan; chan <= highChan; chan++)
		{
			err = ulTIn(daqDeviceHandle, chan, scale, flags, &data);

			if(err == ERR_NO_ERROR)
				printf("Channel(%d) Data: %10.6f %20s\n", chan, data, "");
			else if(err == ERR_OPEN_CONNECTION)
			{
				printf("Channel(%d) Data: %10.6f (open connection)\n", chan, data);
				err = ERR_NO_ERROR;
			}
		}

		usleep(500000);
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

void configureChannels(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan)
{
	UlError err = ERR_NO_ERROR;
	int chan;

	const int USB_2416_ID = 0xd0;
	const int USB_2416_4AO_ID = 0xd1;
	const int USB_2408_ID = 0xfd;
	const int USB_2408_2AO_ID = 0xfe;

	DaqDeviceDescriptor devDescriptor;
	err = ulGetDaqDeviceDescriptor(daqDeviceHandle, &devDescriptor);

	// set the specified analog channels to TC type if the specified DAQ device is a USB-2408 or USB-2416 device
	
	if(devDescriptor.productId == USB_2416_ID || devDescriptor.productId == USB_2416_4AO_ID || 
	   devDescriptor.productId == USB_2408_ID || devDescriptor.productId == USB_2408_2AO_ID)
	{	
		for(chan = lowChan; chan <= highChan; chan++)
		{
			err = ulAISetConfig(daqDeviceHandle, AI_CFG_CHAN_TYPE, chan, AI_TC);
		}
	}
}

