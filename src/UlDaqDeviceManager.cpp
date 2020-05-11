/*
 * UlDaqDeviceManager.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "./UlDaqDeviceManager.h"

#include "DaqDeviceId.h"
#include "DaqDeviceManager.h"
#include "./usb/UsbDaqDevice.h"
#include "./hid/HidDaqDevice.h"
#include "./usb/Usb1208fsPlus.h"
#include "./usb/Usb1608fsPlus.h"
#include "./usb/Usb20x.h"

#include "./usb/Usb1208hs.h"
#include "./usb/Usb1608g.h"
#include "./usb/Usb1808.h"
#include "./usb/Usb26xx.h"
#include "./usb/UsbDio32hs.h"
#include "./usb/UsbCtrx.h"
#include "./hid/UsbDio96h.h"
#include "./hid/UsbDio24.h"
#include "./hid/UsbPdiso8.h"
#include "./hid/UsbSsrxx.h"
#include "./hid/UsbErbxx.h"
#include "./hid/Usb3100.h"
#include "./hid/UsbTemp.h"
#include "./hid/UsbTempAi.h"

#include "./usb/fw/Fx2FwLoader.h"
#include "./usb/fw/DtFx2FwLoader.h"
#include "./usb/UsbQuad08.h"
#include "./usb/Usb9837x.h"
#include "./usb/Usb2001tc.h"
#include "./usb/Usb24xx.h"
#include "./usb/Usb2020.h"
#include "./usb/Usb1608hs.h"
#include "./usb/UsbTc32.h"

#include "./net/NetDiscovery.h"
#include "./net/NetDaqDevice.h"
#include "./net/E1608.h"
#include "./net/EDio24.h"
#include "./net/ETc.h"
#include "./net/ETc32.h"

#include "./net/E1808.h"

#include <iostream>
#include <cstring>
#include <vector>


namespace ul
{

static pthread_mutex_t mInitMutex = PTHREAD_MUTEX_INITIALIZER;
static bool mInitialized = false;

UlDaqDeviceManager::UlDaqDeviceManager()
{

}

UlDaqDeviceManager::~UlDaqDeviceManager()
{

}

UlError UlDaqDeviceManager::init()
{
	UlError err = ERR_NO_ERROR;

	if(!mInitialized)
	{
		pthread_mutex_lock(&mInitMutex);

		if(!mInitialized)
		{
			UsbDaqDevice::usb_init();
			HidDaqDevice::hidapi_init();
			SuspendMonitor::init();

			mInitialized = true;
		}

		pthread_mutex_unlock(&mInitMutex);
	}

	return err;
}

std::vector<DaqDeviceDescriptor> UlDaqDeviceManager::getDaqDeviceInventory(DaqDeviceInterface InterfaceType)
{
	FnLog log("UlDaqDeviceManager::getDaqDeviceInventory");

	init();

	std::vector<DaqDeviceDescriptor> daqDeviceList;

	if(InterfaceType & USB_IFC)
	{
		Fx2FwLoader::prepareHardware();
		DtFx2FwLoader::prepareHardware();

		std::vector<DaqDeviceDescriptor> usbDaqDeviceList = UsbDaqDevice::findDaqDevices();

		std::vector<DaqDeviceDescriptor> hidDaqDeviceList = HidDaqDevice::findDaqDevices();

		for(unsigned int i = 0; i < usbDaqDeviceList.size(); i++)
			daqDeviceList.push_back(usbDaqDeviceList[i]);

		for(unsigned int i = 0; i < hidDaqDeviceList.size(); i++)
			daqDeviceList.push_back(hidDaqDeviceList[i]);
	}

	if(InterfaceType & ETHERNET_IFC)
	{
		std::vector<DaqDeviceDescriptor> netDaqDeviceList = NetDiscovery::findDaqDevices();

		for(unsigned int i = 0; i < netDaqDeviceList.size(); i++)
			daqDeviceList.push_back(netDaqDeviceList[i]);
	}

	return daqDeviceList;
}

DaqDeviceDescriptor UlDaqDeviceManager::getNetDaqDeviceDescriptor(const char* host, unsigned short port, const char* ifcName, double timeout)
{
	init();

	DaqDeviceDescriptor descriptor;

	std::string hostStr = "";
	std::string ifcNameStr = "";

	if(host != NULL)
		hostStr = host;

	if(ifcName != NULL)
		ifcNameStr = ifcName;

	if(timeout == 0.0)
		timeout = 0.001; // 1ms

	int to = timeout > 0.0 ? (int)(timeout * 1000) : -1;

	descriptor = NetDiscovery::findDaqDevice(hostStr, port, ifcNameStr, to);

	return descriptor;
}

UlDaqDevice& UlDaqDeviceManager::createDaqDevice(const DaqDeviceDescriptor& daqDevDescriptor)
{
	init();

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

		case DaqDeviceId::USB_DIO96H:
		case DaqDeviceId::USB_DIO96H_50:
			daqDev = new UsbDio96h(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_1024LS:
		case DaqDeviceId::USB_1024HLS:
		case DaqDeviceId::USB_DIO24:
		case DaqDeviceId::USB_DIO24H:
			daqDev = new UsbDio24(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_PDISO8:
		case DaqDeviceId::USB_PDISO8_40:
			daqDev = new UsbPdiso8(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_SSR24:
		case DaqDeviceId::USB_SSR08:
			daqDev = new UsbSsrxx(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_ERB24:
		case DaqDeviceId::USB_ERB08:
			daqDev = new UsbErbxx(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_3101:
		case DaqDeviceId::USB_3102:
		case DaqDeviceId::USB_3103:
		case DaqDeviceId::USB_3104:
		case DaqDeviceId::USB_3105:
		case DaqDeviceId::USB_3106:
		case DaqDeviceId::USB_3110:
		case DaqDeviceId::USB_3112:
		case DaqDeviceId::USB_3114:
			daqDev = new Usb3100(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_TEMP:
		case DaqDeviceId::USB_TC:
			daqDev = new UsbTemp(daqDevDescriptor);
		break;
		case DaqDeviceId::USB_TEMP_AI:
		case DaqDeviceId::USB_TC_AI:
			daqDev = new UsbTempAi(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_QUAD08:
			daqDev = new UsbQuad08(daqDevDescriptor);
		break;

		case DaqDeviceId::UL_DT9837_A:
		case DaqDeviceId::UL_DT9837_B:
		case DaqDeviceId::UL_DT9837_C:
			daqDev = new Usb9837x(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_2001_TC:
			daqDev = new Usb2001tc(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_2408:
		case DaqDeviceId::USB_2408_2AO:
		case DaqDeviceId::USB_2416:
		case DaqDeviceId::USB_2416_4AO:
			daqDev = new Usb24xx(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_2020:
			daqDev = new Usb2020(daqDevDescriptor, "usb_2020.bin");
		break;

		case DaqDeviceId::USB_1608HS:
		case DaqDeviceId::USB_1608HS_2AO:
			daqDev = new Usb1608hs(daqDevDescriptor);
		break;

		case DaqDeviceId::USB_TC_32:
			daqDev = new UsbTc32(daqDevDescriptor);
		break;


		case DaqDeviceId::E_1608:
			daqDev = new E1608(daqDevDescriptor);
		break;

		case DaqDeviceId::E_DIO24:
			daqDev = new EDio24(daqDevDescriptor);
		break;

		case DaqDeviceId::E_TC:
			daqDev = new ETc(daqDevDescriptor);
		break;

		case DaqDeviceId::E_TC_32:
			daqDev = new ETc32(daqDevDescriptor);
		break;

		// virtual Net devices

		case DaqDeviceId::E_1808:
		case DaqDeviceId::E_1808X:
			daqDev = new E1808(daqDevDescriptor);
		break;
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
