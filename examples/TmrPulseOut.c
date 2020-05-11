/*
    UL call demonstrated:       	  ulTmrPulseOutStart()

    Purpose:                          Generate an output pulse using the
    								  specified timer

    Demonstration:                    Outputs user defined pulse on the
    								  specified timer

    Steps:digital
    1. Call ulGetDaqDeviceInventory() to get the list of available DAQ devices
    2. Call ulCreateDaqDevice() to to get a handle for the first DAQ device
    3. Verify the DAQ device has a timer output subsystem
    4. Call ulConnectDaqDevice() to establish a UL connection to the DAQ device
    5. Call ulTmrPulseOutStart() to start output pulse for the specified timer
    6. Call ulTmrPulseOutStatus(), if it is supported, to monitor the status of the
       pulse output
    7. Call ulTmrPulseOutStop() to stop the output pulse
    8. Call ulDisconnectDaqDevice() and ulReleaseDaqDevice() before exiting the process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "uldaq.h"
#include "utility.h"

#define MAX_DEV_COUNT  100

int main(void)
{
	int descriptorIndex = 0;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	int timerNumber = 0;
	double frequency = 1000.0;
	double dutyCycle = .5;
	unsigned long long pulseCount = 0;
	double initialDelay = 0.0;
	TmrIdleState idleState = TMRIS_LOW;
	PulseOutOption options = PO_DEFAULT;

	int hasTMR = 0;

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

	// get a handle to the device associated with the first descriptor
	 daqDeviceHandle = ulCreateDaqDevice(devDescriptors[descriptorIndex]);

	if (daqDeviceHandle == 0)
	{
		printf ("\nUnable to create a handle to the specified DAQ device\n");
		goto end;
	}

	// verify the specified DAQ device supports timer output
	err = getDevInfoHasTmr(daqDeviceHandle, &hasTMR);
	if (!hasTMR)
	{
		printf("\nThe specified DAQ device does not support timer output\n");
		goto end;
	}

	printf("\nConnecting to device %s - please wait ...\n", devDescriptors[descriptorIndex].devString);

	// establish a connection to the specified DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	if (err != ERR_NO_ERROR)
		goto end;

	printf("\n%s ready\n", devDescriptors[descriptorIndex].devString);
	printf("    Function demonstrated: ulTmrPulseOutStart()\n");
	printf("    Channel: %d\n", timerNumber);
	printf("    Frequency: %f\n", frequency);
	printf("    Duty cycle: %f\n", dutyCycle);
	printf("\nHit ENTER to continue\n");

	ret = scanf("%c", &c);

	ret = system("clear");

	err =  ulTmrPulseOutStart(daqDeviceHandle, timerNumber, &frequency, &dutyCycle, pulseCount, &initialDelay, idleState, options);

	if(err == ERR_NO_ERROR)
	{
		TmrStatus status = TMRS_RUNNING;

		err =  ulTmrPulseOutStatus(daqDeviceHandle, timerNumber, &status);

		if (status == TMRS_RUNNING)
		{
			// if the status is RUNNING, then this timer does support the ulTmrPulseOutStatus()
			// function so we will call the function to determine if the pulse output is stopped
			// due to an error
			while(status == TMRS_RUNNING && err == ERR_NO_ERROR && !enter_press())
			{
				// reset the cursor to the top of the display and
				// show the termination message
				resetCursor();

				printf("Status = TMRS_RUNNING\n");
				printf("Outputting %f Hz pulse with duty cycle %.3f for timer %d ...\n\n", frequency, dutyCycle, timerNumber);

				printf("Hit 'Enter' to terminate the process and stop the timer output\n");

				usleep(100000);

				err =  ulTmrPulseOutStatus(daqDeviceHandle, timerNumber, &status);
			}

			// stop the output if it is still running
			if (status == TMRS_RUNNING && err == ERR_NO_ERROR)
			{
				err = ulTmrPulseOutStop(daqDeviceHandle, timerNumber);
			}
		}
		else
		{
			// if the status is IDLE, then this timer does not support the ulTmrPulseOutStatus()
			// function so we will wait for user input to stop the pulse output

			// reset the cursor to the top of the display and
			// show the termination message
			resetCursor();

			printf("Active DAQ device: %s (%s)\n\n", devDescriptors[descriptorIndex].productName, devDescriptors[descriptorIndex].uniqueId);

			printf("Outputting %f Hz pulse with duty cycle %.3f for timer %d ...\n\n", frequency, dutyCycle, timerNumber);

			printf("Hit 'Enter' to terminate the process and stop the timer output\n");

			while (!enter_press())
			{
				usleep(100000);
			}


			// stop the output if it is still running
			err = ulTmrPulseOutStop(daqDeviceHandle, timerNumber);
		}

		for (i = 0; i < 5; i++)
		{
			cursorUp();
			clearEOL();
		}
		resetCursor();

		printf("Status = TMRS_IDLE\n");

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

