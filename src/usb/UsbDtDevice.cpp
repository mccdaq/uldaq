/*
 * UsbDtDevice.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "UsbDtDevice.h"
#include "Usb9837x.h"
#include <stdlib.h>
#include <unistd.h>

namespace ul
{
UsbDtDevice::UsbDtDevice(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	FnLog log("UsbDtDevice::UsbDtDevice");

	mCmdInEndpointAddr = 0;
	mCmdOutEndpointAddr = 0;
	mMsgInEndpointAddr = 0;

	memset(&mMsgXfer, 0, sizeof(mMsgXfer));

	mMsgXferPending = false;
}

UsbDtDevice::~UsbDtDevice()
{
	FnLog log("UsbDtDevice::~UsbDtDevice");
}

void UsbDtDevice::readSerialNumber(libusb_device *dev, libusb_device_descriptor descriptor, char* serialNum)
{
	if(descriptor.idProduct == DaqDeviceId::DT9837_ABC)
	{
		Usb9837x::readSerialNumber(dev, descriptor, serialNum);
	}
}

unsigned int UsbDtDevice::getVirtualProductId(libusb_device* dev, libusb_device_descriptor descriptor)
{
	unsigned int vProductId = descriptor.idProduct;

	if(descriptor.idVendor == DT_USB_VID)
	{
		if(descriptor.idProduct == DaqDeviceId::DT9837_ABC)
		{
			vProductId = Usb9837x::getVirtualProductId(dev, descriptor);
		}
	}

	return vProductId;
}

unsigned int UsbDtDevice::getActualProductId(unsigned int vProductId)
{
	unsigned int productId;

	switch(vProductId)
	{
	case DaqDeviceId::UL_DT9837_A:
	case DaqDeviceId::UL_DT9837_B:
	case DaqDeviceId::UL_DT9837_C:
		productId = DaqDeviceId::DT9837_ABC;
		break;
	default:
		productId = vProductId;
	}

	return productId;
}

void UsbDtDevice::startMsgReader() const
{
	clearHalt(mMsgInEndpointAddr);

	mMsgXferDoneEvent.reset();

	mMsgXfer.transfer = allocTransfer();
	UlError err = asyncBulkTransfer(mMsgXfer.transfer, mMsgInEndpointAddr, mMsgXfer.buffer, sizeof(mMsgXfer.buffer), msgCallback, (void*) this,  0);

	if(err == ERR_NO_ERROR)
		mMsgXferPending = true;
}

void UsbDtDevice::stopMsgReader() const
{
	if(mMsgXfer.transfer)
	{
		libusb_cancel_transfer(mMsgXfer.transfer);

		if(mMsgXferPending)
		{
			mMsgXferDoneEvent.wait_for_signal(100000); // must wait, calling libusb_free_transfer while request cancellation is not finalized will cause segmentation fault
		}

		libusb_free_transfer(mMsgXfer.transfer);

		mMsgXfer.transfer = NULL;
	}
}


void LIBUSB_CALL UsbDtDevice::msgCallback(libusb_transfer* transfer)
{
	UsbDtDevice* This = (UsbDtDevice*)transfer->user_data;

	if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
	{
		This->messageHandler(transfer->buffer);

		libusb_submit_transfer(This->mMsgXfer.transfer); // resubmit the request
	}
	else
	{
		This->mMsgXferPending = false;
		This->mMsgXferDoneEvent.signal();
	}
}

void UsbDtDevice::flashLed(int flashCount) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}

} /* namespace ul */
