/*
 * UsbDtDevice.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_USBDTDEVICE_H_
#define USB_USBDTDEVICE_H_

#include "UsbDaqDevice.h"

namespace ul
{

class UL_LOCAL UsbDtDevice: public UsbDaqDevice
{
public:
	UsbDtDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbDtDevice();

	static void readSerialNumber(libusb_device* dev, libusb_device_descriptor descriptor, char* serialNum);

	static unsigned int getVirtualProductId(libusb_device* dev, libusb_device_descriptor descriptor);
	static unsigned int getActualProductId(unsigned int vProductId);

	virtual void messageHandler(const unsigned char* messageBuffer) {};

	virtual void flashLed(int flashCount) const;

protected:
	void setCmdInEndpointAddr(unsigned char addr){ mCmdInEndpointAddr = addr; }
	unsigned char getCmdInEndpointAddr() const { return mCmdInEndpointAddr; }

	void setCmdOutEndpointAddr(unsigned char addr) { mCmdOutEndpointAddr = addr; }
	unsigned char getCmdOutEndpointAddr() const { return mCmdOutEndpointAddr; }

	void setMsgInEndpointAddr(unsigned char addr){ mMsgInEndpointAddr = addr; }

	void startMsgReader() const;
	void stopMsgReader() const;


private:
	static void LIBUSB_CALL msgCallback(libusb_transfer* transfer); // callback for message endpoint. handles overrun and underrun

private:
	unsigned char mCmdInEndpointAddr;
	unsigned char mCmdOutEndpointAddr;
	unsigned char mMsgInEndpointAddr;

	mutable ThreadEvent mMsgXferDoneEvent;
	mutable bool mMsgXferPending;

	mutable struct
	{
		libusb_transfer* transfer;
		unsigned char buffer[512];
	} mMsgXfer;
};

} /* namespace ul */

#endif /* USB_USBDTDEVICE_H_ */
