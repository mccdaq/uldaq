/*
 * ulc.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include <libusb-1.0/libusb.h>
#include "./ul_internal.h"
#include "./DaqDeviceManager.h"
#include "./DaqDevice.h"
#include "./AiDevice.h"
#include "./AoDevice.h"
#include "./DioDevice.h"
#include "./CtrDevice.h"
#include "./TmrDevice.h"
#include "./DaqIDevice.h"
#include "./DaqODevice.h"
#include "./DaqEventHandler.h"
#include "./utility/ErrorMap.h"
#include "./usb/UsbDaqDevice.h"
#include "./hid/HidDaqDevice.h"
#include "uldaq.h"
#include "UlDaqDeviceManager.h"
#include "UlException.h"

using namespace ul;

UlError ulInit()
{
	UlError err = UlDaqDeviceManager::init();

	return err;
}

UlError ulGetDaqDeviceInventory(DaqDeviceInterface interfaceTypes, DaqDeviceDescriptor daqDevDescriptors[], unsigned int* numDescriptors )
{
	UL_LOG("ulGetDaqDeviceInventory() <----");

	UlError err = ERR_NO_ERROR;

	std::vector<DaqDeviceDescriptor> daqDeviceList = UlDaqDeviceManager::getDaqDeviceInventory(interfaceTypes);

	if(*numDescriptors >= daqDeviceList.size())
	{
		for(unsigned int i = 0; i < daqDeviceList.size(); i++)
		{
			daqDevDescriptors[i] = daqDeviceList[i];
		}
	}
	else
		err = ERR_BAD_BUFFER_SIZE;

	*numDescriptors = daqDeviceList.size();

	UL_LOG("ulGetDaqDeviceInventory() ---->");

	return err;
}

UlError ulGetNetDaqDeviceDescriptor(const char* host, unsigned short port, const char* ifcName, DaqDeviceDescriptor* daqDevDescriptor, double timeout)
{
	UL_LOG("ulGetNetDaqDeviceDescriptor() <----");

	UlError error = ERR_NO_ERROR;

	try
	{
		*daqDevDescriptor = UlDaqDeviceManager::getNetDaqDeviceDescriptor(host, port, ifcName, timeout);
	}
	catch(UlException& e)
	{
		error = e.getError();
	}
	catch(...)
	{
		error = ERR_UNHANDLED_EXCEPTION;
	}

	UL_LOG("ulGetNetDaqDeviceDescriptor() ---->");

	return error;
}

// coverity[pass_by_value]
DaqDeviceHandle ulCreateDaqDevice(DaqDeviceDescriptor daqDevDescriptor)
{
	UL_LOG("ulCreateDaqDevice() <----");

	int __attribute__((unused)) error = ERR_NO_ERROR;

	DaqDeviceHandle virtualHandle = 0;

	try
	{
		DaqDevice& daqDev = (DaqDevice&) UlDaqDeviceManager::createDaqDevice(daqDevDescriptor);

		virtualHandle =  daqDev.getDeviceNumber();
	}
	catch(UlException& e)
	{
		error = e.getError();
	}
	catch(...)
	{
		error = ERR_UNHANDLED_EXCEPTION;
	}

	UL_LOG("ulCreateDaqDevice() ---->");

	return virtualHandle;
}

DaqDeviceHandle ulCreateDaqDevicePtr(DaqDeviceDescriptor* daqDevDescriptor)
{
	int __attribute__((unused)) error = ERR_NO_ERROR;

	DaqDeviceHandle virtualHandle = 0;

	if(daqDevDescriptor)
	{
		try
		{
			DaqDevice& daqDev = (DaqDevice&) UlDaqDeviceManager::createDaqDevice(*daqDevDescriptor);

			virtualHandle =  daqDev.getDeviceNumber();
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}

	return virtualHandle;
}

UlError ulGetDaqDeviceDescriptor(DaqDeviceHandle daqDeviceHandle, DaqDeviceDescriptor* daqDeviceDescriptor)
{
	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(daqDeviceDescriptor)
			*daqDeviceDescriptor = pDaqDevice->getDescriptor();
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulConnectDaqDevice(DaqDeviceHandle daqDeviceHandle)
{
	UL_LOG("ulConnectDaqDevice() <----");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			pDaqDevice->connect();
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	UL_LOG("ulConnectDaqDevice() ---->");

	return error;
}


UlError ulDisconnectDaqDevice(DaqDeviceHandle daqDeviceHandle)
{
	UL_LOG("ulDisconnectDaqDevice() <----");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			pDaqDevice->disconnect();
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	UL_LOG("ulDisconnectDaqDevice() ---->");

	return error;
}

UlError ulIsDaqDeviceConnected(DaqDeviceHandle daqDeviceHandle, int* connected)
{
	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(connected != NULL)
		{
			if(pDaqDevice->isConnected())
				*connected = 1;
			else
				*connected = 0;
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqDeviceConnectionCode(DaqDeviceHandle daqDeviceHandle, long long code)
{
	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			pDaqDevice->connectionCode(code);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulReleaseDaqDevice(DaqDeviceHandle daqDeviceHandle)
{
	UL_LOG("ulReleaseDaqDevice() <----");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			UlDaqDeviceManager::releaseDaqDevice(*pDaqDevice);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	UL_LOG("ulReleaseDaqDevice() ---->");

	return error;
}

UlError ulFlashLed(DaqDeviceHandle daqDeviceHandle, int flashCount)
{
	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			pDaqDevice->flashLed(flashCount);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAIn(DaqDeviceHandle daqDeviceHandle, int channel, AiInputMode mode, Range range, AInFlag flags, double* data)
{
	FnLog log("ulAIn()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				if(data)
					*data = aiDev->aIn(channel, mode, range, flags);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAInScan(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, AiInputMode inputMode, Range range, int samplesPerChan, double* rate, ScanOption options, AInScanFlag flags, double data[])
{
	FnLog log("ulAInScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				if(rate)
					*rate = aiDev->aInScan(lowChan, highChan, inputMode, range, samplesPerChan, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	//FnLog log("UlGetAInScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
				error = aiDev->getStatus(status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulAInScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
				error = aiDev->wait(waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAInScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulAInScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
				aiDev->stopBackground();
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAInLoadQueue(DaqDeviceHandle daqDeviceHandle, AiQueueElement queue[], unsigned int numElements)
{
	FnLog log("ulALoadQueue()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
				aiDev->aInLoadQueue(queue, numElements);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulAInSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
				aiDev->setTrigger(type, trigChan, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulTIn(DaqDeviceHandle daqDeviceHandle, int channel, TempScale scale, TInFlag flags, double* data)
{
	FnLog log("ulTIn()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				if(data)
					aiDev->tIn(channel, scale, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulTInArray(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, TempScale scale, TInArrayFlag flags, double data[])
{
	FnLog log("ulTInArray()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				aiDev->tInArray(lowChan, highChan, scale, flags, data);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOut(DaqDeviceHandle daqDeviceHandle, int channel, Range range, AOutFlag flags, double data)
{
	FnLog log("ulAOut()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
				aoDev->aOut(channel, range, flags, data);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOutArray(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[])
{
	FnLog log("ulAOutArray()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
			{
				aoDev->aOutArray(lowChan, highChan, range, flags, data);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOutScan(DaqDeviceHandle daqDeviceHandle, int lowChan, int highChan, Range range, int samplesPerChan, double* rate, ScanOption options, AOutScanFlag flags, double data[])
{
	FnLog log("ulAOutScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
			{
				if(rate)
					*rate = aoDev->aOutScan(lowChan, highChan, range, samplesPerChan, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOutScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	FnLog log("ulAOutScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
				error = aoDev->getStatus(status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOutScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulAOutScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
				error = aoDev->wait(waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOutScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulAInScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
				aoDev->stopBackground();
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOutSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulAInSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
				aoDev->setTrigger(type, trigChan, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDConfigPort(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, DigitalDirection direction)
{
	FnLog log("ulDConfigPort()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->dConfigPort(portType, direction);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDConfigBit(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, int bitNum, DigitalDirection direction)
{
	FnLog log("ulDConfigBit()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->dConfigBit(portType, bitNum, direction);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDIn(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, unsigned long long* data)
{
	FnLog log("ulDIn()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				if(data)
					*data = dioDev->dIn(portType);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDOut(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, unsigned long long data)
{
	FnLog log("ulDOut()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->dOut(portType, data);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDInArray(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	FnLog log("ulDInArray()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				if(data)
					dioDev->dInArray(lowPort, highPort, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDOutArray(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, unsigned long long data[])
{
	FnLog log("ulDOutArray()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->dOutArray(lowPort, highPort, data);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDBitIn(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, int bitNum, unsigned int* bitValue)
{
	FnLog log("ulDIn()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				if(bitValue)
					*bitValue = dioDev->dBitIn(portType, bitNum) ? 1 : 0;
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDBitOut(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, int bitNum, unsigned int bitValue)
{
	FnLog log("ulDBitOut()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				bool val = bitValue ? true : false;
				dioDev->dBitOut(portType, bitNum, val);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulDInScan(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double* rate, ScanOption options, DInScanFlag flags, unsigned long long data[])
{
	FnLog log("ulDInScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				if(rate)
					*rate = dioDev->dInScan(lowPort, highPort, samplesPerPort, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}
UlError ulDOutScan(DaqDeviceHandle daqDeviceHandle, DigitalPortType lowPort, DigitalPortType highPort, int samplesPerPort, double* rate, ScanOption options, DOutScanFlag flags, unsigned long long data[])
{
	FnLog log("ulDOutScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				if(rate)
					*rate = dioDev->dOutScan(lowPort, highPort, samplesPerPort, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	FnLog log("ulDInScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				error = dioDev->getStatus(SD_INPUT, status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDOutScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	FnLog log("ulDOutScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				error = dioDev->getStatus(SD_OUTPUT, status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDInScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulDInScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice != NULL)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->stopBackground(SD_INPUT);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDOutScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulDOutScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->stopBackground(SD_OUTPUT);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulDInScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				error = dioDev->wait(SD_INPUT, waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDOutScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulDOutScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				error = dioDev->wait(SD_OUTPUT, waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulDInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulDInSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->setTrigger(SD_INPUT, type, trigChan, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDOutSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulDOutSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->setTrigger(SD_OUTPUT, type, trigChan, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDClearAlarm(DaqDeviceHandle daqDeviceHandle, DigitalPortType portType, unsigned long long mask)
{
	FnLog log("ulDOut()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
				dioDev->dClearAlarm(portType, mask);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCIn(DaqDeviceHandle daqDeviceHandle, int counterNum, unsigned long long* data)
{
	FnLog log("ulCIn()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				if(data)
					*data = ctrDev->cIn(counterNum);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCRead(DaqDeviceHandle daqDeviceHandle, int counterNum, CounterRegisterType regType, unsigned long long* data)
{
	FnLog log("ulCRead()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				if(data)
					*data = ctrDev->cRead(counterNum, regType);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCLoad(DaqDeviceHandle daqDeviceHandle, int counterNum, CounterRegisterType registerType, unsigned long long loadValue)
{
	FnLog log("ulCLoad()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				ctrDev->cLoad(counterNum, registerType, loadValue);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCClear(DaqDeviceHandle daqDeviceHandle, int counterNum)
{
	FnLog log("ulCClear()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				ctrDev->cClear(counterNum);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCConfigScan(DaqDeviceHandle daqDeviceHandle, int counterNum, CounterMeasurementType type, CounterMeasurementMode mode,
					  CounterEdgeDetection edgeDetection, CounterTickSize tickSize,
					  CounterDebounceMode debounceMode, CounterDebounceTime debounceTime, CConfigScanFlag flag)
{
	FnLog log("ulCConfigScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				ctrDev->cConfigScan(counterNum, type, mode,edgeDetection, tickSize, debounceMode, debounceTime, flag);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCInScan(DaqDeviceHandle daqDeviceHandle, int lowCounterNum, int highCounterNum, int samplesPerCounter, double* rate, ScanOption options, CInScanFlag flags, unsigned long long data[])
{
	FnLog log("ulCInScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				if(rate)
					*rate = ctrDev->cInScan(lowCounterNum, highCounterNum, samplesPerCounter, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	FnLog log("ulCInScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
				error = ctrDev->getStatus(status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulCInScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
				error = ctrDev->wait(waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCInScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulCInScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
				ctrDev->stopBackground();
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulCInSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
				ctrDev->setTrigger(type, trigChan, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulTmrPulseOutStart(DaqDeviceHandle daqDeviceHandle, int timerNum, double* frequency, double* dutyCycle, unsigned long long pulseCount, double* initialDelay, TmrIdleState idleState, PulseOutOption options)
{
	FnLog log("ulTmrPulseOutStart()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			TmrDevice* tmrDev = pDaqDevice->tmrDevice();

			if(tmrDev)
			{
				tmrDev->tmrPulseOutStart(timerNum, frequency, dutyCycle, pulseCount, initialDelay, idleState, options);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulTmrPulseOutStop(DaqDeviceHandle daqDeviceHandle, int timerNum)
{
	FnLog log("ulTmrPulseOutStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			TmrDevice* tmrDev = pDaqDevice->tmrDevice();

			if(tmrDev)
			{
				tmrDev->tmrPulseOutStop(timerNum);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}
UlError ulTmrPulseOutStatus(DaqDeviceHandle daqDeviceHandle, int timerNum, TmrStatus* status)
{
	FnLog log("ulTmrPulseOutStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			TmrDevice* tmrDev = pDaqDevice->tmrDevice();

			if(tmrDev)
			{
				tmrDev->tmrPulseOutStatus(timerNum, status);
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulTmrSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulTmrSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			TmrDevice* tmrDev = pDaqDevice->tmrDevice();

			if(tmrDev)
				tmrDev->setTrigger(type, trigChan, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqInScan(DaqDeviceHandle daqDeviceHandle, DaqInChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double* rate, ScanOption options, DaqInScanFlag flags, double data[])
{
	FnLog log("ulDaqInScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqIDevice* daqIDev = pDaqDevice->daqIDevice();

			if(daqIDev)
			{
				if(rate)
					*rate = daqIDev->daqInScan(chanDescriptors, numChans, samplesPerChan, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqInScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	FnLog log("ulDaqInScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqIDevice* daqIDev = pDaqDevice->daqIDevice();

			if(daqIDev)
				error = daqIDev->getStatus(status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqInScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulDaqInScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqIDevice* daqIDev = pDaqDevice->daqIDevice();

			if(daqIDev)
				error = daqIDev->wait(waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqInScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulAInScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqIDevice* daqIDev = pDaqDevice->daqIDevice();

			if(daqIDev)
				daqIDev->stopBackground();
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqInSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, DaqInChanDescriptor trigChanDescriptor, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulDaqInSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqIDevice* daqIDev = pDaqDevice->daqIDevice();

			if(daqIDev)
				daqIDev->setTrigger(type, trigChanDescriptor, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}




UlError ulDaqOutScan(DaqDeviceHandle daqDeviceHandle, DaqOutChanDescriptor chanDescriptors[], int numChans, int samplesPerChan, double* rate, ScanOption options, DaqOutScanFlag flags, double data[])
{
	FnLog log("ulDaqOutScan()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqODevice* daqODev = pDaqDevice->daqODevice();

			if(daqODev)
			{
				if(rate)
					*rate = daqODev->daqOutScan(chanDescriptors, numChans, samplesPerChan, *rate, options, flags, data);
				else
					error = ERR_BAD_ARG;
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqOutScanStatus(DaqDeviceHandle daqDeviceHandle, ScanStatus* status, TransferStatus* xferStatus)
{
	FnLog log("ulDaqInScanStatus()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqODevice* daqODev = pDaqDevice->daqODevice();

			if(daqODev)
				error = daqODev->getStatus(status, xferStatus);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqOutScanWait(DaqDeviceHandle daqDeviceHandle, WaitType waitType, long long waitParam, double timeout)
{
	FnLog log("ulDaqOutScanWait()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqODevice* daqODev = pDaqDevice->daqODevice();

			if(daqODev)
				error = daqODev->wait(waitType, waitParam, timeout);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqOutScanStop(DaqDeviceHandle daqDeviceHandle)
{
	FnLog log("ulAInScanStop()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqODevice* daqODev = pDaqDevice->daqODevice();

			if(daqODev)
				daqODev->stopBackground();
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqOutSetTrigger(DaqDeviceHandle daqDeviceHandle, TriggerType type, DaqInChanDescriptor trigChanDescriptor, double level, double variance, unsigned int retriggerCount)
{
	FnLog log("ulDaqOutSetTrigger()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqODevice* daqODev = pDaqDevice->daqODevice();

			if(daqODev)
				daqODev->setTrigger(type, trigChanDescriptor, level, variance, retriggerCount);
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulEnableEvent(DaqDeviceHandle daqDeviceHandle, DaqEventType eventTypes, unsigned long long eventParameter, DaqEventCallback eventCallbackFunction, void* userData)
{
	FnLog log("ulEnableEvent()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqEventHandler* eventHandler = pDaqDevice->eventHandler();
			eventHandler->enableEvent(eventTypes, eventParameter, eventCallbackFunction, userData);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}
UlError ulDisableEvent(DaqDeviceHandle daqDeviceHandle, DaqEventType eventTypes)
{
	FnLog log("ulDisableEvent()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DaqEventHandler* eventHandler = pDaqDevice->eventHandler();
			eventHandler->disableEvent(eventTypes);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulMemRead(DaqDeviceHandle daqDeviceHandle, MemRegion memRegion, unsigned int address, unsigned char* buffer, unsigned int count)
{
	FnLog log("ulMemRead()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			pDaqDevice->memRead(memRegion, address, buffer, count);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}
UlError ulMemWrite(DaqDeviceHandle daqDeviceHandle, MemRegion memRegion, unsigned int address, unsigned char* buffer, unsigned int count)
{
	FnLog log("ulMemWrite()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			pDaqDevice->memWrite(memRegion, address, buffer, count);
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}




UlError ulGetErrMsg(UlError errCode, char errMsg[ERR_MSG_LEN])
{
	UlError err = ERR_NO_ERROR;

	if(errMsg)
	{
		std::string msg = ErrorMap::instance().getErrorMsg(errCode);

		msg.copy(errMsg, msg.length());

		errMsg[msg.length()] = '\0';
	}
	else
		err = ERR_BAD_BUFFER;

	return err;
}

UlError ulGetInfoStr(UlInfoItemStr infoItem, unsigned int index, char* infoStr, unsigned int* maxConfigLen)
{
	FnLog log("ulGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	ulInit();

	try
	{
		switch(infoItem)
		{
		case UL_INFO_VER_STR:

			if(infoStr)
			{
				infoStr[0] = '\0';

				if(strlen(UL_VERSION) < *maxConfigLen)
				{
					memcpy(infoStr, UL_VERSION, strlen(UL_VERSION) + 1);
					*maxConfigLen = strlen(UL_VERSION) + 1;
				}
				else
				{
					*maxConfigLen = strlen(UL_VERSION) + 1;
					error = ERR_BAD_BUFFER_SIZE;
				}
			}
			else
				error = ERR_BAD_BUFFER;

			break;

		default:
			error = ERR_BAD_INFO_ITEM;
		}
	}
	catch(UlException& e)
	{
		error = e.getError();
	}
	catch(...)
	{
		error = ERR_UNHANDLED_EXCEPTION;
	}

	return error;
}

UlError ulSetConfig(UlConfigItem configItem, unsigned int index, long long configValue)
{
	FnLog log("ulSetConfig()");

	UlError error = ERR_NO_ERROR;

	ulInit();

	try
	{
		switch(configItem)
		{
		case UL_CFG_USB_XFER_PRIORITY:
			UsbDaqDevice::setUsbEventHandlerThreadPriority(configValue);
			break;

		default:
			error = ERR_BAD_CONFIG_ITEM;
		}
	}
	catch(UlException& e)
	{
		error = e.getError();
	}
	catch(...)
	{
		error = ERR_UNHANDLED_EXCEPTION;
	}

	return error;
}

UlError ulGetConfig(UlConfigItem configItem, unsigned int index, long long* configValue)
{
	FnLog log("ulGetConfig()");

	UlError error = ERR_NO_ERROR;

	ulInit();

	try
	{
		switch(configItem)
		{
		case UL_CFG_USB_XFER_PRIORITY:
			*configValue = UsbDaqDevice::getUsbEventHandlerThreadPriority();
			break;

		default:
			error = ERR_BAD_CONFIG_ITEM;
		}
	}
	catch(UlException& e)
	{
		error = e.getError();
	}
	catch(...)
	{
		error = ERR_UNHANDLED_EXCEPTION;
	}

	return error;
}

UlError ulDevGetInfo(DaqDeviceHandle daqDeviceHandle, DevInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulDevGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				const UlDaqDeviceInfo& devInfo = pDaqDevice->getDevInfo();

				switch(infoItem)
				{
				case DEV_INFO_HAS_AI_DEV:
					*infoValue = devInfo.hasAiDevice() ? 1 : 0;
					break;
				case DEV_INFO_HAS_AO_DEV:
					*infoValue = devInfo.hasAoDevice() ? 1 : 0;
					break;
				case DEV_INFO_HAS_DIO_DEV:
					*infoValue = devInfo.hasDioDevice() ? 1 : 0;
					break;
				case DEV_INFO_HAS_CTR_DEV:
					*infoValue = devInfo.hasCtrDevice() ? 1 : 0;
					break;
				case DEV_INFO_HAS_TMR_DEV:
					*infoValue = devInfo.hasTmrDevice() ? 1 : 0;
					break;
				case DEV_INFO_HAS_DAQI_DEV:
					*infoValue = devInfo.hasDaqIDevice() ? 1 : 0;
					break;
				case DEV_INFO_HAS_DAQO_DEV:
					*infoValue = devInfo.hasDaqODevice() ? 1 : 0;
					break;
				case DEV_INFO_DAQ_EVENT_TYPES:
					*infoValue = devInfo.getEventTypes();
					break;
				case DEV_INFO_MEM_REGIONS:
					*infoValue = devInfo.getMemInfo().getMemRegionTypes();
					break;

				default:
					error = ERR_BAD_INFO_ITEM;
				}
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDevSetConfig(DaqDeviceHandle daqDeviceHandle, DevConfigItem configItem, unsigned int index, long long configValue)
{
	FnLog log("ulDevSetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			UlDaqDeviceConfig& devConfig = pDaqDevice->getDevConfig();

			switch(configItem)
			{
			case DEV_CFG_CONNECTION_CODE:
				devConfig.setConnectionCode(configValue);
				break;
			case DEV_CFG_MEM_UNLOCK_CODE:
				devConfig.setMemUnlockCode(configValue);
				break;
			case DEV_CFG_RESET:
				devConfig.reset();
				break;

			default:
				error = ERR_BAD_CONFIG_ITEM;
			}
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDevGetConfig(DaqDeviceHandle daqDeviceHandle, DevConfigItem configItem, unsigned int index, long long* configValue)
{
	FnLog log("ulDevGetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			UlDaqDeviceConfig& devConfig = pDaqDevice->getDevConfig();

			switch(configItem)
			{
			case DEV_CFG_HAS_EXP:
				*configValue = devConfig.hasExp() ? 1 : 0;
				break;
			case DEV_CFG_CONNECTION_CODE:
				*configValue = devConfig.getConnectionCode();
				break;
			case DEV_CFG_MEM_UNLOCK_CODE:
				*configValue = devConfig.getMemUnlockCode();
				break;

			default:
				error = ERR_BAD_CONFIG_ITEM;
			}
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDevGetConfigStr(DaqDeviceHandle daqDeviceHandle, DevConfigItemStr configItem, unsigned int index, char* configStr, unsigned int* maxConfigLen)
{
	FnLog log("ulDevGetConfigStr()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			UlDaqDeviceConfig& devConfig = pDaqDevice->getDevConfig();

			switch(configItem)
			{
			case DEV_CFG_VER_STR:
				devConfig.getVersionStr((DevVersionType) index, configStr, maxConfigLen);
				break;
			case DEV_CFG_IP_ADDR_STR:
				devConfig.getIpAddressStr(configStr, maxConfigLen);
				break;
			case DEV_CFG_NET_IFC_STR:
				devConfig.getNetIfcNameStr(configStr, maxConfigLen);
				break;

			default:
				error = ERR_BAD_CONFIG_ITEM;
			}
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAISetConfig(DaqDeviceHandle daqDeviceHandle, AiConfigItem configItem, unsigned int index, long long configValue)
{
	FnLog log("ulSetAIConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				UlAiConfig& aiConfig = aiDev->getAiConfig();

				switch(configItem)
				{
				case AI_CFG_CHAN_TYPE:
					aiConfig.setChanType(index, (AiChanType) configValue);
					break;
				case AI_CFG_CHAN_TC_TYPE:
					aiConfig.setChanTcType(index, (TcType) configValue);
					break;
				/*case AI_CFG_SCAN_CHAN_TEMP_UNIT:
					aiConfig.setScanChanTempUnit(index, (TempUnit) configValue);
					break;*/
				case AI_CFG_SCAN_TEMP_UNIT:
					aiConfig.setScanTempUnit((TempUnit) configValue);
					break;
				case AI_CFG_ADC_TIMING_MODE:
					aiConfig.setAdcTimingMode((AdcTimingMode) configValue);
					break;
				case AI_CFG_AUTO_ZERO_MODE:
					aiConfig.setAutoZeroMode((AutoZeroMode) configValue);
					break;
				case AI_CFG_CHAN_IEPE_MODE:
					aiConfig.setChanIepeMode(index, (IepeMode) configValue);
					break;
				case AI_CFG_CHAN_COUPLING_MODE:
					aiConfig.setChanCouplingMode(index, (CouplingMode) configValue);
					break;
				case AI_CFG_CHAN_OTD_MODE:
					aiConfig.setChanOpenTcDetectionMode(index, (OtdMode) configValue);
					break;
				case AI_CFG_OTD_MODE:
					aiConfig.setOpenTcDetectionMode(index, (OtdMode) configValue);
					break;
				case AI_CFG_CAL_TABLE_TYPE:
					aiConfig.setCalTableType(index, (AiCalTableType) configValue);
					break;
				case AI_CFG_REJECT_FREQ_TYPE:
					aiConfig.setRejectFreqType(index, (AiRejectFreqType) configValue);
					break;

				default:
					error = ERR_BAD_CONFIG_ITEM;
				}
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAIGetConfig(DaqDeviceHandle daqDeviceHandle, AiConfigItem configItem, unsigned int index, long long* configValue)
{
	FnLog log("ulAIGetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(configValue)
		{
			try
			{
				AiDevice* aiDev = pDaqDevice->aiDevice();

				if(aiDev)
				{
					UlAiConfig& aiConfig = aiDev->getAiConfig();

					switch(configItem)
					{
					case AI_CFG_CHAN_TYPE:
						*configValue = aiConfig.getChanType(index);
						break;
					case AI_CFG_CHAN_TC_TYPE:
						*configValue = aiConfig.getChanTcType(index);
						break;
					/*case AI_CFG_SCAN_CHAN_TEMP_UNIT:
						*configValue = aiConfig.getScanChanTempUnit(index);
						break;*/
					case AI_CFG_SCAN_TEMP_UNIT:
						*configValue = aiConfig.getScanTempUnit();
						break;
					case AI_CFG_ADC_TIMING_MODE:
						*configValue = aiConfig.getAdcTimingMode();
						break;
					case AI_CFG_AUTO_ZERO_MODE:
						*configValue = aiConfig.getAutoZeroMode();
						break;
					case AI_CFG_CAL_DATE:
						*configValue = aiConfig.getCalDate(index);
						break;
					case AI_CFG_CHAN_IEPE_MODE:
						*configValue = aiConfig.getChanIepeMode(index);
						break;
					case AI_CFG_CHAN_COUPLING_MODE:
						*configValue = aiConfig.getChanCouplingMode(index);
						break;
					case AI_CFG_CHAN_SENSOR_CONNECTION_TYPE:
						*configValue = aiConfig.getChanSensorConnectionType(index);
						break;
					case AI_CFG_CHAN_OTD_MODE:
						*configValue = aiConfig.getChanOpenTcDetectionMode(index);
						break;
					case AI_CFG_OTD_MODE:
						*configValue = aiConfig.getOpenTcDetectionMode(index);
						break;
					case AI_CFG_CAL_TABLE_TYPE:
						*configValue = aiConfig.getCalTableType(index);
						break;
					case AI_CFG_REJECT_FREQ_TYPE:
						*configValue = aiConfig.getRejectFreqType(index);
						break;
					case AI_CFG_EXP_CAL_DATE:
						*configValue = aiConfig.getExpCalDate(index);
						break;

					default:
						error = ERR_BAD_CONFIG_ITEM;
					}

				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAISetConfigDbl(DaqDeviceHandle daqDeviceHandle, AiConfigItemDbl configItem, unsigned int index, double configValue)
{
	FnLog log("ulAISetConfigDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				UlAiConfig& aiConfig = aiDev->getAiConfig();

				switch(configItem)
				{
				case AI_CFG_CHAN_SLOPE:
					aiConfig.setChanSlope(index, configValue);
					break;
				case AI_CFG_CHAN_OFFSET:
					aiConfig.setChanOffset(index, configValue);
					break;
				case AI_CFG_CHAN_SENSOR_SENSITIVITY:
					aiConfig.setChanSensorSensitivity(index, configValue);
					break;
				case AI_CFG_CHAN_DATA_RATE:
					aiConfig.setChanDataRate(index, configValue);
					break;
				default:
					error = ERR_BAD_CONFIG_ITEM;
				}
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAIGetConfigDbl(DaqDeviceHandle daqDeviceHandle, AiConfigItemDbl configItem, unsigned int index, double* configValue)
{
	FnLog log("ulAIGetConfigDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(configValue)
		{
			try
			{
				AiDevice* aiDev = pDaqDevice->aiDevice();

				if(aiDev)
				{
					UlAiConfig& aiConfig = aiDev->getAiConfig();

					switch(configItem)
					{
					case AI_CFG_CHAN_SLOPE:
						*configValue = aiConfig.getChanSlope(index);
						break;
					case AI_CFG_CHAN_OFFSET:
						*configValue = aiConfig.getChanOffset(index);
						break;
					case AI_CFG_CHAN_SENSOR_SENSITIVITY:
						*configValue = aiConfig.getChanSensorSensitivity(index);
						break;
					case AI_CFG_CHAN_DATA_RATE:
						*configValue = aiConfig.getChanDataRate(index);
						break;

					default:
						error = ERR_BAD_CONFIG_ITEM;
					}

				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;

	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAIGetConfigStr(DaqDeviceHandle daqDeviceHandle, AiConfigItemStr configItem, unsigned int index, char* configStr, unsigned int* maxConfigLen)
{
	FnLog log("ulAIGetConfigStr()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AiDevice* aiDev = pDaqDevice->aiDevice();

			if(aiDev)
			{
				UlAiConfig& aiConfig = aiDev->getAiConfig();

				switch(configItem)
				{
				case AI_CFG_CAL_DATE_STR:
					aiConfig.getCalDateStr(index, configStr, maxConfigLen);
					break;
				case AI_CFG_CHAN_COEFS_STR:
					aiConfig.getChanCoefsStr(index, configStr, maxConfigLen);
					break;
				case AI_CFG_EXP_CAL_DATE_STR:
					aiConfig.getExpCalDateStr(index, configStr, maxConfigLen);
					break;

				default:
					error = ERR_BAD_CONFIG_ITEM;
				}

			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAIGetInfo(DaqDeviceHandle daqDeviceHandle, AiInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulAIGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				AiDevice* aiDev = pDaqDevice->aiDevice();

				if(aiDev)
				{
					const UlAiInfo& aiInfo = aiDev->getAiInfo();

					switch(infoItem)
					{
					case AI_INFO_RESOLUTION:
						*infoValue = aiInfo.getResolution();
						break;
					case AI_INFO_NUM_CHANS:
						*infoValue = aiInfo.getNumChans();
						break;
					case AI_INFO_NUM_CHANS_BY_MODE:
						*infoValue = aiInfo.getNumChansByMode((AiInputMode) index);
						break;
					case AI_INFO_NUM_CHANS_BY_TYPE:
						*infoValue = aiInfo.getNumChansByType((AiChanType) index);
						break;
					case AI_INFO_CHAN_TYPES:
						*infoValue = aiInfo.getChanTypes();
						break;
					case AI_INFO_SCAN_OPTIONS:
						*infoValue = aiInfo.getScanOptions();
						break;
					case AI_INFO_HAS_PACER:
						*infoValue = aiInfo.hasPacer();
						break;
					case AI_INFO_NUM_DIFF_RANGES:
						*infoValue = aiInfo.getRangeCountByMode(AI_DIFFERENTIAL);
						break;
					case AI_INFO_NUM_SE_RANGES:
						*infoValue = aiInfo.getRangeCountByMode(AI_SINGLE_ENDED);
						break;
					case AI_INFO_DIFF_RANGE:
						*infoValue = aiInfo.getRangeByMode(AI_DIFFERENTIAL, index);
						break;
					case AI_INFO_SE_RANGE:
						*infoValue = aiInfo.getRangeByMode(AI_SINGLE_ENDED, index);
						break;
					case AI_INFO_TRIG_TYPES:
						*infoValue = aiInfo.getTriggerTypes();
						break;
					case AI_INFO_MAX_QUEUE_LENGTH_BY_MODE:
						*infoValue = aiInfo.getMaxQueueLength((AiInputMode) index);
						break;
					case AI_INFO_QUEUE_TYPES:
						*infoValue = aiInfo.getQueueTypes();
						break;
					case AI_INFO_QUEUE_LIMITS:
						*infoValue = aiInfo.getChanQueueLimitations();
						break;
					case AI_INFO_FIFO_SIZE:
						*infoValue = aiInfo.getFifoSize();
						break;
					case AI_INFO_IEPE_SUPPORTED:
						*infoValue = aiInfo.supportsIepe();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulAIGetInfoDbl(DaqDeviceHandle daqDeviceHandle, AiInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulAIGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				AiDevice* aiDev = pDaqDevice->aiDevice();

				if(aiDev)
				{
					const UlAiInfo& aiInfo = aiDev->getAiInfo();

					switch(infoItem)
					{
					case AI_INFO_MIN_SCAN_RATE:
						*infoValue = aiInfo.getMinScanRate();
						break;
					case AI_INFO_MAX_SCAN_RATE:
						*infoValue = aiInfo.getMaxScanRate();
						break;
					case AI_INFO_MAX_THROUGHPUT:
						*infoValue = aiInfo.getMaxThroughput();
						break;
					case AI_INFO_MAX_BURST_RATE:
						*infoValue = aiInfo.getMaxBurstRate();
						break;
					case AI_INFO_MAX_BURST_THROUGHPUT:
						*infoValue = aiInfo.getMaxBurstThroughput();
					break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulAOSetConfig(DaqDeviceHandle daqDeviceHandle, AoConfigItem configItem, unsigned int index, long long configValue)
{
	FnLog log("ulAOSetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			AoDevice* aoDev = pDaqDevice->aoDevice();

			if(aoDev)
			{
				UlAoConfig& aoConfig = aoDev->getAoConfig();

				switch(configItem)
				{
				case AO_CFG_SYNC_MODE:
					aoConfig.setSyncMode((AOutSyncMode) configValue);
					break;
				case AO_CFG_CHAN_SENSE_MODE:
					aoConfig.setSenseMode(index, (AOutSenseMode) configValue);
					break;
				default:
					error = ERR_BAD_CONFIG_ITEM;
				}
			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulAOGetConfig(DaqDeviceHandle daqDeviceHandle, AoConfigItem configItem, unsigned int index, long long* configValue)
{
	FnLog log("ulAOGetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(configValue)
		{
			try
			{
				AoDevice* aoDev = pDaqDevice->aoDevice();

				if(aoDev)
				{
					UlAoConfig& aoConfig = aoDev->getAoConfig();

					switch(configItem)
					{
					case AO_CFG_SYNC_MODE:
						*configValue = aoConfig.getSyncMode();
						break;
					case AO_CFG_CHAN_SENSE_MODE:
						*configValue = aoConfig.getSenseMode(index);
					break;

					default:
						error = ERR_BAD_CONFIG_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOGetInfo(DaqDeviceHandle daqDeviceHandle, AoInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulAOGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				AoDevice* aoDev = pDaqDevice->aoDevice();

				if(aoDev)
				{
					const UlAoInfo& aoInfo = aoDev->getAoInfo();

					switch(infoItem)
					{
					case AO_INFO_RESOLUTION:
						*infoValue = aoInfo.getResolution();
						break;
					case AO_INFO_NUM_CHANS:
						*infoValue = aoInfo.getNumChans();
						break;
					case AO_INFO_SCAN_OPTIONS:
						*infoValue = aoInfo.getScanOptions();
						break;
					case AO_INFO_HAS_PACER:
						*infoValue = aoInfo.hasPacer();
						break;
					case AO_INFO_NUM_RANGES:
						*infoValue = aoInfo.getRangeCount();
						break;
					case AO_INFO_RANGE:
						*infoValue = aoInfo.getRange(index);
						break;
					case AO_INFO_TRIG_TYPES:
						*infoValue = aoInfo.getTriggerTypes();
						break;
					case AO_INFO_FIFO_SIZE:
						*infoValue = aoInfo.getFifoSize();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulAOGetInfoDbl(DaqDeviceHandle daqDeviceHandle, AoInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulAOGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				AoDevice* aoDev = pDaqDevice->aoDevice();

				if(aoDev)
				{
					const UlAoInfo& aoInfo = aoDev->getAoInfo();

					switch(infoItem)
					{
					case AO_INFO_MIN_SCAN_RATE:
						*infoValue = aoInfo.getMinScanRate();
						break;
					case AO_INFO_MAX_SCAN_RATE:
						*infoValue = aoInfo.getMaxScanRate();
						break;
					case AO_INFO_MAX_THROUGHPUT:
						*infoValue = aoInfo.getMaxThroughput();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDIOGetInfo(DaqDeviceHandle daqDeviceHandle, DioInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulDioGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				DioDevice* dioDev = pDaqDevice->dioDevice();

				if(dioDev)
				{
					const UlDioInfo& dioInfo = dioDev->getDioInfo();

					switch(infoItem)
					{
					case DIO_INFO_NUM_PORTS:
						*infoValue = dioInfo.getNumPorts();
						break;
					case DIO_INFO_PORT_TYPE:
						*infoValue = dioInfo.getPortType(index);
						break;
					case DIO_INFO_PORT_IO_TYPE:
						*infoValue = dioInfo.getPortIoType(index);
						break;
					case DIO_INFO_NUM_BITS:
						*infoValue = dioInfo.getNumBits(index);
						break;
					case DIO_INFO_HAS_PACER:
						*infoValue = dioInfo.hasPacer((DigitalDirection) index);
						break;
					case DIO_INFO_SCAN_OPTIONS:
						*infoValue = dioInfo.getScanOptions((DigitalDirection) index);
						break;
					case DIO_INFO_TRIG_TYPES:
						*infoValue = dioInfo.getTriggerTypes((DigitalDirection) index);
						break;
					case DIO_INFO_FIFO_SIZE:
						*infoValue = dioInfo.getFifoSize((DigitalDirection) index);
					break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDIOGetInfoDbl(DaqDeviceHandle daqDeviceHandle, DioInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulDIOGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				DioDevice* dioDev = pDaqDevice->dioDevice();

				if(dioDev)
				{
					const UlDioInfo& dioInfo = dioDev->getDioInfo();

					switch(infoItem)
					{
					case DIO_INFO_MIN_SCAN_RATE:
						*infoValue = dioInfo.getMinScanRate((DigitalDirection) index);
						break;
					case DIO_INFO_MAX_SCAN_RATE:
						*infoValue = dioInfo.getMaxScanRate((DigitalDirection) index);
						break;
					case DIO_INFO_MAX_THROUGHPUT:
						*infoValue = dioInfo.getMaxThroughput((DigitalDirection) index);
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDIOGetConfig(DaqDeviceHandle daqDeviceHandle, DioConfigItem configItem, unsigned int index, long long* configValue)
{
	FnLog log("ulDioGetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(configValue)
		{
			try
			{
				DioDevice* dioDev = pDaqDevice->dioDevice();

				if(dioDev)
				{
					UlDioConfig& dioConfig = dioDev->getDioConfig();

					switch(configItem)
					{
					case DIO_CFG_PORT_DIRECTION_MASK:
						*configValue = dioConfig.getPortDirectionMask(index);
					break;
					case DIO_CFG_PORT_ISO_FILTER_MASK:
						*configValue = dioConfig.getPortIsoMask(index);
						break;
					case DIO_CFG_PORT_LOGIC:
						*configValue = dioConfig.getPortLogic(index);
						break;

					default:
						error = ERR_BAD_CONFIG_ITEM;
					}

				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDIOSetConfig(DaqDeviceHandle daqDeviceHandle, DioConfigItem configItem, unsigned int index, long long configValue)
{
	FnLog log("ulDIOSetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			DioDevice* dioDev = pDaqDevice->dioDevice();

			if(dioDev)
			{
				UlDioConfig& dioConfig = dioDev->getDioConfig();

				switch(configItem)
				{
				case DIO_CFG_PORT_INITIAL_OUTPUT_VAL:
					dioConfig.setPortInitialOutputVal(index, configValue);
				break;
				case DIO_CFG_PORT_ISO_FILTER_MASK:
					dioConfig.setPortIsoMask(index, configValue);
				break;
				default:
					error = ERR_BAD_CONFIG_ITEM;
				}

			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}

	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulCtrGetInfo(DaqDeviceHandle daqDeviceHandle, CtrInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulCtrGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				CtrDevice* ctrDev = pDaqDevice->ctrDevice();

				if(ctrDev)
				{
					const UlCtrInfo& ctrInfo = ctrDev->getCtrInfo();

					switch(infoItem)
					{
					case CTR_INFO_NUM_CTRS:
						*infoValue = ctrInfo.getNumCtrs();
						break;
					case CTR_INFO_MEASUREMENT_TYPES:
						*infoValue = ctrInfo.getCtrMeasurementTypes(index);
						break;
					case CTR_INFO_MEASUREMENT_MODES:
						*infoValue = ctrInfo.getCtrMeasurementModes((CounterMeasurementType) index);
						break;
					case CTR_INFO_REGISTER_TYPES:
						*infoValue = ctrInfo.getRegisterTypes();
						break;
					case CTR_INFO_RESOLUTION:
						*infoValue = ctrInfo.getResolution();
						break;
					case CTR_INFO_HAS_PACER:
						*infoValue = ctrInfo.hasPacer();
						break;
					case CTR_INFO_SCAN_OPTIONS:
						*infoValue = ctrInfo.getScanOptions();
						break;
					case CTR_INFO_TRIG_TYPES:
						*infoValue = ctrInfo.getTriggerTypes();
						break;
					case CTR_INFO_FIFO_SIZE:
						*infoValue = ctrInfo.getFifoSize();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCtrGetInfoDbl(DaqDeviceHandle daqDeviceHandle, CtrInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulCtrGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				CtrDevice* ctrDev = pDaqDevice->ctrDevice();

				if(ctrDev)
				{
					const UlCtrInfo& ctrInfo = ctrDev->getCtrInfo();

					switch(infoItem)
					{
					case CTR_INFO_MIN_SCAN_RATE:
						*infoValue = ctrInfo.getMinScanRate();
						break;
					case CTR_INFO_MAX_SCAN_RATE:
						*infoValue = ctrInfo.getMaxScanRate();
						break;
					case CTR_INFO_MAX_THROUGHPUT:
						*infoValue = ctrInfo.getMaxThroughput();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCtrSetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long configValue)
{
	FnLog log("ulCtrSetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		try
		{
			CtrDevice* ctrDev = pDaqDevice->ctrDevice();

			if(ctrDev)
			{
				UlCtrConfig& ctrConfig = ctrDev->getCtrConfig();

				switch(configItem)
				{
				case CTR_CFG_REG:
					ctrConfig.setCtrCfgReg(index, configValue);
					break;

				default:
					error = ERR_BAD_CONFIG_ITEM;
				}

			}
			else
				error = ERR_BAD_DEV_TYPE;
		}
		catch(UlException& e)
		{
			error = e.getError();
		}
		catch(...)
		{
			error = ERR_UNHANDLED_EXCEPTION;
		}
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulCtrGetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long* configValue)
{
	FnLog log("ulCtrGetConfig()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(configValue)
		{
			try
			{
				CtrDevice* ctrDev = pDaqDevice->ctrDevice();

				if(ctrDev)
				{
					UlCtrConfig& ctrConfig = ctrDev->getCtrConfig();

					switch(configItem)
					{
					case CTR_CFG_REG:
						*configValue = ctrConfig.getCtrCfgReg(index);
						break;


					default:
						error = ERR_BAD_CONFIG_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulTmrGetInfo(DaqDeviceHandle daqDeviceHandle, TmrInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulTmrGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				TmrDevice* tmrDev = pDaqDevice->tmrDevice();

				if(tmrDev)
				{
					const UlTmrInfo& tmrInfo = tmrDev->getTmrInfo();

					switch(infoItem)
					{
					case TMR_INFO_NUM_TMRS:
						*infoValue = tmrInfo.getNumTimers();
						break;
					case TMR_INFO_TYPE:
						*infoValue = tmrInfo.getTimerType(index);
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}


UlError ulTmrGetInfoDbl(DaqDeviceHandle daqDeviceHandle, TmrInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulTmrGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				TmrDevice* tmrDev = pDaqDevice->tmrDevice();

				if(tmrDev)
				{
					const UlTmrInfo& tmrInfo = tmrDev->getTmrInfo();

					switch(infoItem)
					{
					case TMR_INFO_MIN_FREQ:
						*infoValue = tmrInfo.getMinFrequency();
						break;
					case TMR_INFO_MAX_FREQ:
						*infoValue = tmrInfo.getMaxFrequency();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqIGetInfo(DaqDeviceHandle daqDeviceHandle, DaqIInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulDaqIGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				DaqIDevice* DaqIDev = pDaqDevice->daqIDevice();

				if(DaqIDev)
				{
					const UlDaqIInfo& daqIInfo = DaqIDev->getDaqIInfo();

					switch(infoItem)
					{
					case DAQI_INFO_CHAN_TYPES:
						*infoValue = daqIInfo.getChannelTypes();
						break;
					case DAQI_INFO_SCAN_OPTIONS:
						*infoValue = daqIInfo.getScanOptions();
						break;
					case DAQI_INFO_TRIG_TYPES:
						*infoValue = daqIInfo.getTriggerTypes();
						break;
					case DAQI_INFO_FIFO_SIZE:
						*infoValue = daqIInfo.getFifoSize();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqIGetInfoDbl(DaqDeviceHandle daqDeviceHandle, DaqIInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulDaqIGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				DaqIDevice* DaqIDev = pDaqDevice->daqIDevice();

				if(DaqIDev)
				{
					const UlDaqIInfo& daqIInfo = DaqIDev->getDaqIInfo();

					switch(infoItem)
					{
					case DAQI_INFO_MIN_SCAN_RATE:
						*infoValue = daqIInfo.getMinScanRate();
						break;
					case DAQI_INFO_MAX_SCAN_RATE:
						*infoValue = daqIInfo.getMaxScanRate();
						break;
					case DAQI_INFO_MAX_THROUGHPUT:
						*infoValue = daqIInfo.getMaxThroughput();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqOGetInfo(DaqDeviceHandle daqDeviceHandle, DaqOInfoItem infoItem, unsigned int index, long long* infoValue)
{
	FnLog log("ulDaqOGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				DaqODevice* DaqODev = pDaqDevice->daqODevice();

				if(DaqODev)
				{
					const UlDaqOInfo& daqOInfo = DaqODev->getDaqOInfo();

					switch(infoItem)
					{
					case DAQO_INFO_CHAN_TYPES:
						*infoValue = daqOInfo.getChannelTypes();
						break;
					case DAQO_INFO_SCAN_OPTIONS:
						*infoValue = daqOInfo.getScanOptions();
						break;
					case DAQO_INFO_TRIG_TYPES:
						*infoValue = daqOInfo.getTriggerTypes();
						break;
					case DAQO_INFO_FIFO_SIZE:
						*infoValue = daqOInfo.getFifoSize();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulDaqOGetInfoDbl(DaqDeviceHandle daqDeviceHandle, DaqOInfoItemDbl infoItem, unsigned int index, double* infoValue)
{
	FnLog log("ulDaqOGetInfoDbl()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(infoValue)
		{
			try
			{
				DaqODevice* DaqODev = pDaqDevice->daqODevice();

				if(DaqODev)
				{
					const UlDaqOInfo& daqOInfo = DaqODev->getDaqOInfo();

					switch(infoItem)
					{
					case DAQO_INFO_MIN_SCAN_RATE:
						*infoValue = daqOInfo.getMinScanRate();
						break;
					case DAQO_INFO_MAX_SCAN_RATE:
						*infoValue = daqOInfo.getMaxScanRate();
						break;
					case DAQO_INFO_MAX_THROUGHPUT:
						*infoValue = daqOInfo.getMaxThroughput();
						break;

					default:
						error = ERR_BAD_INFO_ITEM;
					}
				}
				else
					error = ERR_BAD_DEV_TYPE;
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}

UlError ulMemGetInfo(DaqDeviceHandle daqDeviceHandle, MemRegion memRegion, MemDescriptor* memDescriptor)
{
	FnLog log("ulMemGetInfo()");

	UlError error = ERR_NO_ERROR;

	DaqDevice* pDaqDevice = DaqDeviceManager::getActualDeviceHandle(daqDeviceHandle);

	if(pDaqDevice)
	{
		if(memDescriptor != NULL)
		{
			try
			{
				UlMemRegionInfo& memRegionInfo = pDaqDevice->getDevInfo().getMemInfo().getMemRegionInfo(memRegion);

				memDescriptor->region = memRegion;
				memDescriptor->address = memRegionInfo.getAddress();
				memDescriptor->size = memRegionInfo.getSize();
				memDescriptor->accessTypes = memRegionInfo.getAccessTypes();
			}
			catch(UlException& e)
			{
				error = e.getError();
			}
			catch(...)
			{
				error = ERR_UNHANDLED_EXCEPTION;
			}
		}
		else
			error = ERR_BAD_ARG;
	}
	else
		error = ERR_BAD_DEV_HANDLE;

	return error;
}






