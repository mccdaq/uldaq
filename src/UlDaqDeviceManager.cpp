/*
 * UlDaqDeviceManager.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "./UlDaqDeviceManager.h"

#include "DaqDeviceId.h"
#include "DaqDeviceManager.h"
#include "./usb/UsbDaqDevice.h"
//#include "./hid/HidDaqDevice.h"
#include "./usb/Usb1208fsPlus.h"
#include "./usb/Usb1608fsPlus.h"
#include "./usb/Usb20x.h"

#include "./usb/Usb1208hs.h"
#include "./usb/Usb1608g.h"
#include "./usb/Usb1808.h"
#include "./usb/Usb26xx.h"
#include "./usb/UsbDio32hs.h"
#include "./usb/UsbCtrx.h"
//#include "./hid/UsbTc.h"

#include <iostream>
#include <cstring>
#include <vector>


namespace ul
{

UlDaqDeviceManager::UlDaqDeviceManager()
{

}

UlDaqDeviceManager::~UlDaqDeviceManager()
{

}

std::vector<DaqDeviceDescriptor> UlDaqDeviceManager::getDaqDeviceInventory(DaqDeviceInterface InterfaceType)
{
	FnLog log("UlDaqDeviceManager::getDaqDeviceInventory");

	std::vector<DaqDeviceDescriptor> daqDeviceList;

	std::vector<DaqDeviceDescriptor> usbDaqDeviceList = UsbDaqDevice::findDaqDevices();

	//std::vector<DaqDeviceDescriptor> hidDaqDeviceList = HidDaqDevice::findDaqDevices();

	for(unsigned int i = 0; i < usbDaqDeviceList.size(); i++)
		daqDeviceList.push_back(usbDaqDeviceList[i]);

	/*for(unsigned int i = 0; i < hidDaqDeviceList.size(); i++)
		daqDeviceList.push_back(hidDaqDeviceList[i]);*/

	return daqDeviceList;
}

UlDaqDevice& UlDaqDeviceManager::createDaqDevice(DaqDeviceDescriptor daqDevDescriptor)
{
	DaqDevice* daqDev = DaqDeviceManager::getDaqDevice(daqDevDescriptor); // Don't recreate a new DaqDevice object if it already exists for the specified descriptor

	if(daqDev == NULL)
	{
		switch(daqDevDescriptor.productId)
		{


		case DaqDeviceId::USB_1208FS_PLUS:
		case DaqDeviceId::USB_1408FS_PLUS:
			daqDev = new Usb1208fs_Plus(daqDevDescriptor);
			break;

		case DaqDeviceId::USB_1608FS_PLUS:
			daqDev = new Usb1608fs_Plus(daqDevDescriptor);
			break;

		case DaqDeviceId::USB_201:
		case DaqDeviceId::USB_202:
		case DaqDeviceId::USB_204:
		case DaqDeviceId::USB_205:
			daqDev = new Usb20x(daqDevDescriptor);
			break;

		case DaqDeviceId::USB_1208HS:
		case DaqDeviceId::USB_1208HS_2AO:
		case DaqDeviceId::USB_1208HS_4AO:
			daqDev = new Usb1208hs(daqDevDescriptor, "USB_1208HS.rbf");
			break;

		case DaqDeviceId::USB_1608G:
		case DaqDeviceId::USB_1608GX:
		case DaqDeviceId::USB_1608GX_2AO:
			daqDev = new Usb1608g(daqDevDescriptor, "USB_1608G.rbf");
			break;
		case DaqDeviceId::USB_1608G_2:
		case DaqDeviceId::USB_1608GX_2:
		case DaqDeviceId::USB_1608GX_2AO_2:
			daqDev = new Usb1608g(daqDevDescriptor, "USB_1608G_2.rbf");
		break;

		case DaqDeviceId::USB_1808:
		case DaqDeviceId::USB_1808X:
			daqDev = new Usb1808(daqDevDescriptor, "USB_1808.bin");
		break;

		case DaqDeviceId::USB_2623:
		case DaqDeviceId::USB_2627:
		case DaqDeviceId::USB_2633:
		case DaqDeviceId::USB_2637:
			daqDev = new Usb26xx(daqDevDescriptor, "USB_26xx.rbf");
		break;

		case DaqDeviceId::USB_DIO32HS:
			daqDev = new UsbDio32hs(daqDevDescriptor, "USB_DIO32HS.bin");
		break;

		case DaqDeviceId::USB_CTR04:
		case DaqDeviceId::USB_CTR08:
			daqDev = new UsbCtrx(daqDevDescriptor, "USB_CTR.bin");
		break;

		/*case DaqDeviceId::USB_TC:
			daqDev = new UsbTc(daqDevDescriptor);
		break;*/
		}

		if(daqDev)
		{
			DaqDeviceManager::addToCreatedList(daqDev);
		}
		else
			throw UlException(ERR_BAD_DEV_TYPE);
	}

	return *daqDev;
}

void UlDaqDeviceManager::releaseDaqDevice(UlDaqDevice& daqDevice)
{
	DaqDeviceManager::releaseDevice(((DaqDevice&)daqDevice).getDeviceNumber());
}


} /* namespace ul */
