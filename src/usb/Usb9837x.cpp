/*
 * Usb9837x.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "Usb9837x.h"
#include "./daqi/DaqIUsb9837x.h"
#include "./ai/AiUsb9837x.h"
#include "./ao/AoUsb9837x.h"
#include "./ctr/CtrUsb9837x.h"


#include <unistd.h>
#include <stdlib.h>

// CT22150F module constaints
#define VCO_MIN_FREQ	100// MHz
#define VCO_MAX_FREQ	400// MHz
#define Q_COUNTER_MIN_FREQ	0.190// MHz
#define HALF_Q_COUNTER_MIN_FREQ	0.095 //MHz

#define ASIZE	0x100

namespace ul
{
Usb9837x::Usb9837x(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDtDevice(daqDeviceDescriptor)
{
	FnLog log("Usb9837x::Usb9837x");

	mDaqDeviceInfo.setClockFreq(24000000);

	setDaqIDevice(new DaqIUsb9837x(*this));
	setAiDevice(new AiUsb9837x(*this));

	if(getDeviceType() == DaqDeviceId::UL_DT9837_A || getDeviceType() == DaqDeviceId::UL_DT9837_B)
		setCtrDevice(new CtrUsb9837x(*this, 3));

	if(getDeviceType() == DaqDeviceId::UL_DT9837_A || getDeviceType() == DaqDeviceId::UL_DT9837_C)
		setAoDevice(new AoUsb9837x(*this, 1));

	setCmdInEndpointAddr(Usb9837xDefs::READ_CMD_PIPE);
	setCmdOutEndpointAddr(Usb9837xDefs::WRITE_CMD_PIPE);

	setMsgInEndpointAddr(Usb9837xDefs::READ_MSG_PIPE);

	if(mDaqDeviceInfo.hasAoDevice())
		mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);
	else
		mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

}

Usb9837x::~Usb9837x()
{
	FnLog log("UsbQuadxx::~UsbQuadxx");

	if(mConnected)
	{
		disconnect();
	}

}

void Usb9837x::disconnect()
{
	FnLog log("UsbDaqDevice::disconnect");

	if(mConnected)
	{
		stopMsgReader();

		UsbDtDevice::disconnect();
	}
}

void Usb9837x::initilizeHardware() const
{

	/*Note: On Windows 10 systems, if powerAlwaysOn flag is not set in EEPROM then calling cmdPowerDevice(true) will result in
	 * device reset, as a workaround the windows kernel driver was modified to set the powerAlwaysOn flag in EEPROM before the power on
	 * command is sent. If the same behavior is experienced on Linux systems then uncomment the following code */

	/*if(!isPowerAlwaysOn())
		writePowerAlwaysOnToEeprom();*/

	cmdPowerDevice( true );

	usleep(100000);

	startMsgReader();
}

void Usb9837x::readIdentifier(libusb_device *dev, libusb_device_descriptor descriptor, unsigned int* identifier)
{
	libusb_device_handle* devHandle = NULL;

	int status = libusb_open(dev, &devHandle);

	if (status == LIBUSB_SUCCESS)
	{
		status = libusb_claim_interface(devHandle, 0);

		if (status == LIBUSB_SUCCESS)
		{
#pragma pack(1)
			union
			{
				struct
				{
					unsigned int cmd;
					unsigned char numReads;
					struct
					{
						unsigned char addess;
						unsigned char offset;
					}info[2];
				};
				unsigned char buffer[64];
			}msg;
#pragma pack()

			memset(&msg, 0, sizeof(msg));

			msg.cmd = Usb9837xDefs::R_MULTI_BYTE_I2C_REG;
			msg.numReads = 2; // serial number is stored in 4 bytes

			for(int i = 0; i < msg.numReads; i++)
			{
				msg.info[i].addess = Usb9837xDefs::EEPROM_DEV_ADR;
				msg.info[i].offset = 0x05 + i; // id starts at offset 0x05 for 9837x series.
			}

			int transferred = 0;

			unsigned char endpoint = Usb9837xDefs::WRITE_CMD_PIPE;

			status = libusb_bulk_transfer(devHandle, endpoint, (unsigned char*) &msg, sizeof(msg), &transferred, 2000);

			if(status == LIBUSB_SUCCESS)
			{
				endpoint = Usb9837xDefs::READ_CMD_PIPE;

				unsigned short id = 0;
				transferred = 0;

				status = libusb_bulk_transfer(devHandle, endpoint, (unsigned char*) &id, sizeof(id), &transferred, 2000);

				if (status == LIBUSB_SUCCESS)
				{
					if(transferred > 0 )
					{
						*identifier = id;
					}
				}
				else
				{
					UL_LOG("#### libusb_bulk_transfer failed : " << libusb_error_name(status));
				}
			}
			else
			{
				UL_LOG("#### libusb_bulk_transfer failed : " << libusb_error_name(status));
			}

			libusb_release_interface(devHandle, 0);
		}
		else
		{
			UL_LOG("#### libusb_claim_interface() failed: " << libusb_error_name(status));
		}

		libusb_close(devHandle);
	}
}

void Usb9837x::readSerialNumber(libusb_device *dev, libusb_device_descriptor descriptor, char* serialNum)
{
	libusb_device_handle* devHandle = NULL;

	int status = libusb_open(dev, &devHandle);


	if (status == LIBUSB_SUCCESS)
	{
		status = libusb_claim_interface(devHandle, 0);

		if (status == LIBUSB_SUCCESS)
		{
#pragma pack(1)
			union
			{
				struct
				{
					unsigned int cmd;
					unsigned char numReads;
					struct
					{
						unsigned char addess;
						unsigned char offset;
					}info[4];
				};
				unsigned char buffer[64];
			}msg;
#pragma pack()

			memset(&msg, 0, sizeof(msg));

			msg.cmd = Usb9837xDefs::R_MULTI_BYTE_I2C_REG;
			msg.numReads = 4; // serial number is stored in 4 bytes

			for(int i = 0; i < msg.numReads; i++)
			{
				msg.info[i].addess = Usb9837xDefs::EEPROM_DEV_ADR;
				msg.info[i].offset = 0x08 + i; // id starts at offset 0x08 for 9837x series.
			}

			int transferred = 0;

			unsigned char endpoint = Usb9837xDefs::WRITE_CMD_PIPE;

			status = libusb_bulk_transfer(devHandle, endpoint, (unsigned char*) &msg, sizeof(msg), &transferred, 2000);

			if(status == LIBUSB_SUCCESS)
			{
				endpoint = Usb9837xDefs::READ_CMD_PIPE;
				unsigned int serial = 0;
				transferred = 0;

				status = libusb_bulk_transfer(devHandle, endpoint, (unsigned char*) &serial, sizeof(serial), &transferred, 2000);

				if (status == LIBUSB_SUCCESS)
				{
					if(transferred > 0 )
					{
						sprintf(serialNum, "%d", serial);
					}
				}
				else
				{
					UL_LOG("#### libusb_bulk_transfer failed : " << libusb_error_name(status));
				}
			}
			else
			{
				UL_LOG("#### libusb_bulk_transfer failed : " << libusb_error_name(status));
			}

			libusb_release_interface(devHandle, 0);
		}
		else
		{
			strcpy(serialNum, "INTERFACE CLAIMED");

			UL_LOG("#### libusb_claim_interface() failed: " << libusb_error_name(status));
		}

		libusb_close(devHandle);
	}
	else
	{
		if(status == LIBUSB_ERROR_ACCESS)
			strcpy(serialNum, NO_PERMISSION_STR);
	}
}

unsigned int Usb9837x::getVirtualProductId(libusb_device* dev, libusb_device_descriptor descriptor)
{
	unsigned int vProductId = descriptor.idProduct;

	if(descriptor.idVendor == DT_USB_VID && descriptor.idProduct == DaqDeviceId::DT9837_ABC)
	{
		unsigned int id = 0;
		readIdentifier(dev, descriptor, &id);

		if(id == 1)
			vProductId = DaqDeviceId::UL_DT9837_A;
		else if(id == 2)
			vProductId = DaqDeviceId::UL_DT9837_B;
		else if(id == 4)
			vProductId = DaqDeviceId::UL_DT9837_C;
	}

	return vProductId;
}

void Usb9837x::messageHandler(const unsigned char* messageBuffer)
{
	const Usb9837xDefs::USB_MSG* msgBuffer = (Usb9837xDefs::USB_MSG*) messageBuffer;

	//from CDt9837aDevice::MessageHandler

	DaqIUsb9837x* daqiDev = (DaqIUsb9837x*) daqIDevice();
	AoUsb9837x* aoDev = (AoUsb9837x*) aoDevice();

	switch(msgBuffer->MsgType)
	{
		case Usb9837xDefs::ADC_OVERRUN_MSG:
			if(daqiDev)
			{
				daqiDev->overrunOccured();
				daqiDev->setscanErrorFlag();
			}

			//std::cout << "overrun occurred" << std::endl;
			break;
		case Usb9837xDefs::DAC_THRESHOLD_REACHED_MSG:
			//std::cout << "DAC_THRESHOLD_REACHED_MSG" << std::endl;
			break;
		case Usb9837xDefs::DAC_OVER_SAMPLE_MSG:
			if (aoDev)
			{
				aoDev->underrunOccured();
				aoDev->setscanErrorFlag();
			}
			//std::cout << "underrun occurred" << std::endl;
			break;

		default:
			break;
	};
}

bool Usb9837x::isPowerAlwaysOn() const
{
	bool bAlwaysOn = false;
	unsigned char alwaysOn;
	unsigned char address = Usb9837xDefs::EEPROM_OFFSET_POWER_OVERRIDE_REG;

	Cmd_ReadDevMultipleRegs(Usb9837xDefs::EEPROM_DEV_ADR, 1, &address, &alwaysOn);

	if(alwaysOn)
		bAlwaysOn = true;

	return bAlwaysOn;
}

void Usb9837x::writePowerAlwaysOnToEeprom() const
{
	unsigned char alwaysOn = 1;
	unsigned char address = Usb9837xDefs::EEPROM_OFFSET_POWER_OVERRIDE_REG;

	Cmd_WriteDevMultipleRegs(Usb9837xDefs::EEPROM_DEV_ADR, 1, &address, &alwaysOn);
}

void Usb9837x::cmdPowerDevice( bool bPowerOn ) const
{
	Usb9837xDefs::Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::DATA_ACQ_POWER;
	Cmd.d.WriteBoardPowerInfo.PowerStatus = bPowerOn ? 1 : 0;

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}

/*
OLSTATUS CDt9837aDevice::Cmd_ReadUsbReg( BYTE Address, BYTE *pDataVal )
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	OLSTATUS olStat = OLGENERALFAILURE;

	t << "CDt9837aDevice::Cmd_ReadUsbReg() Entry\n";

	Cmd.CmdCode = R_BYTE_USB_REG;
	Cmd.d.ReadByteInfo.Address = Address;

	ULONG ulBytesSent = 0;
	if ( !NT_SUCCESS(WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ))
	{
		t << " ERROR! CDt9837aDevice::Cmd_ReadUsbReg() (WriteData) failed\n";
	}
	else
	{
		ULONG ulBytesRead = 0;
		if( !NT_SUCCESS(ReadData( pDataVal, sizeof(BYTE), &ulBytesRead, m_Pipe[READ_CMD_PIPE] ) ) )
		{
			t << " ERROR! CDt9837aDevice::Cmd_ReadUsbReg() (ReadData) failed\n";
		}
		else
		{
			t << "\tAddress="<<Address<<" Value=" <<*pDataVal <<"\n";
			olStat = OLSUCCESS;
		}
	}
	return olStat;
}

OLSTATUS CDt9837aDevice::Cmd_WriteUsbReg( BYTE Address, BYTE DataVal )
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	OLSTATUS olStat = OLGENERALFAILURE;

	t << "CDt9837aDevice::Cmd_WriteUsbReg() Entry\n";

	Cmd.CmdCode = W_BYTE_USB_REG;
	Cmd.d.WriteByteInfo.Address = Address;
	Cmd.d.WriteByteInfo.DataVal = DataVal;

	t << "size of USB command = " << (DWORD) sizeof (Usb9837xDefs::USB_CMD) << "\n";

	t << " INFO! CDt9837aDevice::Cmd_WriteUsbReg()" << Address <<"Data\n"<<DataVal;

	ULONG ulBytesSent = 0;
	if ( !NT_SUCCESS(WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_WriteUsbReg() failed\n";
	}
	else
	{
		t << ulBytesSent << " bytes sent\n";
		olStat = OLSUCCESS;
	}
	return olStat;
}
*/


void Usb9837x::Cmd_ReadSingleWordFromLocalBus( unsigned short Address, unsigned short *pDataVal ) const
{
	Usb9837xDefs::Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::R_SINGLE_WORD_LB;

	// Flip the address big/little endian
	Cmd.d.ReadWordInfo.Address = Endian::cpu_to_be_ui16(Address);

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();
	unsigned char inEndpoint = getCmdInEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err == ERR_NO_ERROR)
	{
		transferred = 0;
		unsigned short val;

		err = syncBulkTransfer(inEndpoint, (unsigned char*) &val, sizeof(unsigned short), &transferred, 1000);

		*pDataVal = Endian::be_ui16_to_cpu(val);
	}

	if(err)
		throw UlException(err);
}

void Usb9837x::Cmd_WriteSingleWordToLocalBus( unsigned short Address, unsigned short DataVal ) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::W_SINGLE_WORD_LB;
	Cmd.d.WriteWordInfo.Address = Endian::cpu_to_be_ui16(Address);
	Cmd.d.WriteWordInfo.DataVal = Endian::cpu_to_be_ui16(DataVal);

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}

void Usb9837x::Cmd_RMWSingleWordToLocalBus( unsigned short Address, unsigned short AndMask, unsigned short OrVal) const
{
	// NOTE: AndMask : set "1" for each bit where values will be modified.
	//       OrVal   : set "1" or "0" for each bit defined in AndMask. These are the new
	//                 values for these bits.
	// Example: If you want to change bit<2> = 1 and bit<0> = 0 and leave everything
	//          else unchanged, set AndMask=0x05 and OrVal=0x04
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::RMW_SINGLE_WORD_LB;
	Cmd.d.RMWWordInfo.Address = Endian::cpu_to_be_ui16(Address);
	Cmd.d.RMWWordInfo.AndMask = Endian::cpu_to_be_ui16(AndMask);
	Cmd.d.RMWWordInfo.OrVal = Endian::cpu_to_be_ui16(OrVal);

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}
/*
OLSTATUS
CDt9837aDevice::Cmd_ReadMultipleUsbReg( BYTE NumReads, BYTE *pAddresses, BYTE *pData )
{
	t << "CDt9837aDevice::Cmd_ReadMultipleUsbReg() Entry\n";

	if (NumReads > MAX_NUM_MULTI_BYTE_RDS)
	{
		t << "\tERROR! NumReads="<< NumReads << "too large\n";
		return OLGENERALFAILURE;
	}

	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	Cmd.CmdCode = Usb9837xDefs::R_MULTI_BYTE_USB_REG;
	Cmd.d.ReadMultiInfo.NumReads = NumReads;
	BYTE *pAdrOutPtr = Cmd.d.ReadMultiInfo.Addresses;

	for (int i=0; i< NumReads; i++)
	{
		*pAdrOutPtr = *pAddresses;
		pAdrOutPtr++;
		pAddresses++;
	}

	NTSTATUS ntStatus;
	OLSTATUS olStat;
	ULONG ulBytesSent = 0;
	ntStatus = WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] );
	if ( !NT_SUCCESS( ntStatus ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_ReadMultipleUsbReg() (WriteData) failed\n";
		olStat = OLGENERALFAILURE;
	}
	else
	{
		ULONG ulBytesRead = 0;
		ntStatus = ReadData( pData, NumReads, &ulBytesRead, m_Pipe[READ_CMD_PIPE] );
		if( !NT_SUCCESS( ntStatus ) )
		{
			t << " ERROR! CDt9837aDevice::Cmd_ReadMultipleUsbReg() (ReadData) failed\n";
			olStat = OLGENERALFAILURE;
		}
		else
			olStat = OLSUCCESS;
	}
	return olStat;
}

NTSTATUS
CDt9837aDevice::Cmd_WriteMultipleUsbReg( BYTE NumWrites, WRITE_BYTE_INFO *pWrite )
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

	t << "CDt9837aDevice::Cmd_WriteMultipleUsbReg() Entry\n";

	if (NumWrites > MAX_NUM_MULTI_BYTE_WRTS)
	{
		t << "\tERROR! NumWrites="<< NumWrites << "too large\n";
		return ntStatus;
	}

	Cmd.CmdCode = W_MULTI_BYTE_USB_REG;
	Cmd.d.WriteMultiInfo.NumWrites = NumWrites;

	for (int i=0; i< NumWrites; i++)
	{
		Cmd.d.WriteMultiInfo.Write[i] = pWrite[i];
	}

	ULONG ulBytesSent = 0;
	if ( !NT_SUCCESS( ntStatus = WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_WriteMultipleUsbReg() failed\n";
	}

	return ntStatus;
}*/

void Usb9837x::Cmd_WriteMultiplePLLReg(Usb9837xDefs::SUBSYSTEM_TYPE subsystemType, unsigned char deviceAddress, unsigned char NumWrites, Usb9837xDefs::WRITE_BYTE_INFO* pWrite ) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	/*NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

	t << "CDt9837aDevice::Cmd_WriteMultiplePLLReg() Entry\n";*/

	if (NumWrites > (Usb9837xDefs::MAX_NUM_MULTI_BYTE_WRTS - 2))
	{
		std::cout << "ERROR! NumWrites="<< NumWrites << "too large\n";
		return;
	}

	Cmd.d.WriteMultiPllInfo.SubsystemType = subsystemType;
	Cmd.d.WriteMultiPllInfo.DevAddr = deviceAddress;
	Cmd.CmdCode = Usb9837xDefs::W_MULTI_BYTE_PLL_REG;
	Cmd.d.WriteMultiPllInfo.NumWrites = NumWrites;

	for (int i=0; i< NumWrites; i++)
	{
		Cmd.d.WriteMultiPllInfo.Write[i] = pWrite[i];
      //t << "\tCmd.d.WriteMultiPllInfo.DevAddr"<< Cmd.d.WriteMultiPllInfo.Write[i].Address <<"\n";
      //t << "\tCmd.d.WriteMultiPllInfo.DevAddr"<< Cmd.d.WriteMultiPllInfo.Write[i].DataVal <<"\n";
	}

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}

/*
NTSTATUS
CDt9837aDevice::Cmd_ModifyUsbReg( BYTE Address, BYTE AndMask, BYTE OrVal )
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

	t << "CDt9837aDevice::Cmd_ModifyUsbReg() Entry\n";

	Cmd.CmdCode = RMW_BYTE_USB_REG;
	Cmd.d.RMWByteInfo.Address = Address;
	Cmd.d.RMWByteInfo.AndMask = AndMask;
	Cmd.d.RMWByteInfo.OrVal = OrVal;

	ULONG ulBytesSent = 0;

	if ( !NT_SUCCESS( ntStatus = WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_ModifyUsbReg() (WriteData) failed\n";
	}

	return ntStatus;
}


OLSTATUS
CDt9837aDevice::Cmd_ModifyMultipleUsbReg( BYTE NumRegs, RMW_BYTE_INFO *pByteInfo )
{
	t << "CDt9837aDevice::Cmd_ModifyMultipleUsbReg() Entry\n";

	OLSTATUS olStat = OLGENERALFAILURE;

	if (NumRegs > MAX_NUM_MULTI_BYTE_RMWS)
	{
		t << "Too many RMWs requested (" << NumRegs << ")\n";
		return (OLGENERALFAILURE);
	}

	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	Cmd.CmdCode = RMW_MULTI_BYTE_USB_REG;
	Cmd.d.RMWMultiInfo.NumRMWs = NumRegs;
	for (int i=0; i<NumRegs; i++)
	{
		Cmd.d.RMWMultiInfo.ByteInfo[i] = *pByteInfo;
		t << " ByteInfo[i].Address"<< Cmd.d.RMWMultiInfo.ByteInfo[i].Address<<"\n";
		t << " ByteInfo[i].AndMask"<< Cmd.d.RMWMultiInfo.ByteInfo[i].AndMask<<"\n";
		t << " ByteInfo[i].OrVal"<< Cmd.d.RMWMultiInfo.ByteInfo[i].OrVal<<"\n";


		pByteInfo++;
	}

	ULONG ulBytesSent = 0;
	if ( !NT_SUCCESS(WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_ModifyUsbReg() (WriteData) failed\n";
		olStat = OLGENERALFAILURE;
	}
	else
		olStat = OLSUCCESS;

	return olStat;
}




NTSTATUS
CDt9837aDevice::DoReadCommand( PUCHAR pucBuffer )
{
	NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	ULONG ulBytesSent = 0;
	static Trap = 0;

//	t << "DoReadCommand ENTERING >>>> ";
	if (Trap)
		t << "\n************* DoReadCommand RECURSION *************\n";

	if ( NT_SUCCESS( ntStatus = WriteData( pucBuffer, COMMAND_SIZE, &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		ULONG ulBytesRead = 0;

		if( !NT_SUCCESS( ntStatus = ReadData( pucBuffer, COMMAND_SIZE, &ulBytesRead, m_Pipe[READ_CMD_PIPE] ) ) )
		{
			t << " ERROR! CDtUsbDevice::DoReadCommand() failed\n";
		}
	}
	t << " <<<< EXITING DoReadCommand \n";
	return ntStatus;
}


NTSTATUS
CDt9837aDevice::StartDataStream( ULONG BlockSize )
{

	IncrementOutstandingRequestCount();

	return m_DtUsbBulkXferInFifo.Start( BlockSize );

}

NTSTATUS CDt9837aDevice::ReadDataStream( PUCHAR pDestBuffer,
										 ULONG ulBytesRequested,
										 ULONG& ulBytesRead )
{
	return m_DtUsbBulkXferInFifo.ReadBulkXferFifo( pDestBuffer,
																 ulBytesRequested,
																 ulBytesRead );
}


NTSTATUS CDt9837aDevice::StopDataStream()
{

	DecrementOutstandingRequestCount();

	return m_DtUsbBulkXferInFifo.Stop();
}
*/
void Usb9837x::Cmd_StartSubsystem (Usb9837xDefs::pSUBSYSTEM_INFO pSubsystemInfo) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	//t << "CDt9837aDevice::Cmd_StartSubsystem() Entry\n";

	Cmd.CmdCode = Usb9837xDefs::START_SUBSYSTEM;
	memcpy (&Cmd.d.SubsystemInfo, pSubsystemInfo, sizeof(Usb9837xDefs::SUBSYSTEM_INFO));

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}
/*
OLSTATUS CDt9837aDevice::Cmd_SimStartSubsystem (pSUBSYSTEM_INFO pSubsystemInfo) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));
	t << "CDt9837aDevice::Cmd_SimStartSubsystem() Entry\n";

	Cmd.CmdCode = SIM_START_SUBSYSTEM;
	memcpy (&Cmd.d.SubsystemInfo, pSubsystemInfo, sizeof(SUBSYSTEM_INFO));

	ULONG ulBytesSent = 0;
	OLSTATUS olStat;;
	if ( !NT_SUCCESS(WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_SimStartSubsystem()\n";
		olStat = OLGENERALFAILURE;
	}
	else
		olStat = OLSUCCESS;

	return olStat;
}*/



void Usb9837x::Cmd_StopSubsystem (Usb9837xDefs::pSUBSYSTEM_INFO pSubsystemInfo) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	//t << "CDt9837aDevice::Cmd_StopSubsystem() Entry\n";

	Cmd.CmdCode = Usb9837xDefs::STOP_SUBSYSTEM;
	memcpy (&Cmd.d.SubsystemInfo, pSubsystemInfo, sizeof(Usb9837xDefs::SUBSYSTEM_INFO));

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}


void Usb9837x::Cmd_WriteSingleValue (Usb9837xDefs::pWRITE_SINGLE_VALUE_INFO pWriteSingleValueInfo) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::W_SINGLE_VALUE_CMD;
	memcpy (&Cmd.d.WriteSingleValueInfo, pWriteSingleValueInfo, sizeof(Usb9837xDefs::WRITE_SINGLE_VALUE_INFO));

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}

void Usb9837x::Cmd_SetDaFifoSize(unsigned int dacFifoSize) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::W_DATA_BLOCK_GPIF_BUS ;

	Cmd.d.FifoSizeInfo.FifoSize.DWordVal = (unsigned int)dacFifoSize;

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);

}

/*
OLSTATUS CDt9837aDevice::Cmd_ReadSingleValues (BYTE NumChans, BYTE gain, DWORD *pData) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	t << "CDt9837aDevice::Cmd_ReadSingleValues() Entry\n";

	Cmd.CmdCode = R_SINGLE_VALUES_CMD;
	Cmd.d.ReadSingleValuesInfo.NumChans = NumChans;
	Cmd.d.ReadSingleValuesInfo.Gain = gain;

	ULONG ulBytesSent = 0;
	OLSTATUS olStat = OLSUCCESS;
	if ( !NT_SUCCESS(WriteData( (BYTE*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &ulBytesSent, m_Pipe[WRITE_CMD_PIPE] ) ) )
	{
		t << " ERROR! CDt9837aDevice::Cmd_ReadSingleValue()\n";
		olStat = OLGENERALFAILURE;
	}
	else
	{
		ULONG ulBytesRead = 0;
		ULONG ulBytesToRead = NumChans * sizeof (DWORD);
		if( !NT_SUCCESS(ReadData( (BYTE*)pData, ulBytesToRead, &ulBytesRead, m_Pipe[READ_CMD_PIPE] ) ) )
		{
			t << " ERROR! CDt9837aDevice::Cmd_ReadSingleValue() (ReadData) failed\n";
			olStat = OLGENERALFAILURE;
		}
	}

	return olStat;
}*/

void Usb9837x::Cmd_ReadSingleValue (Usb9837xDefs::pREAD_SINGLE_VALUE_INFO pReadSingleValueInfo, unsigned int* pData) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	//t<< "CDt9837aDevice::Cmd_ReadSingleValue() Entry\n";

	Cmd.CmdCode = Usb9837xDefs::R_SINGLE_VALUE_CMD;
	memcpy (&Cmd.d.ReadSingleValueInfo, pReadSingleValueInfo, sizeof(Usb9837xDefs::READ_SINGLE_VALUE_INFO));

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();
	unsigned char inEndpoint = getCmdInEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err == ERR_NO_ERROR)
	{
		transferred = 0;
		unsigned int val;

		err = syncBulkTransfer(inEndpoint, (unsigned char*) &val, sizeof(val), &transferred, 1000);

		*pData = Endian::le_ui32_to_cpu(val);
	}

	if(err)
		throw UlException(err);
}


void Usb9837x::Cmd_ReadDevMultipleRegs( unsigned char DevAddress, unsigned char NumRegs, unsigned char *pRegisters, unsigned char *pData ) const
{
	if (NumRegs > Usb9837xDefs::MAX_NUM_MULTI_BYTE_WRTS)
	{
		std::cout << "ERROR! NumRegs="<< NumRegs << "too large" << std::endl;
	}

	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::R_MULTI_BYTE_I2C_REG;
	Cmd.d.ReadI2CMultiInfo.NumReads = NumRegs;

	Usb9837xDefs::pREAD_I2C_BYTE_INFO pReadDevByteInfo = Cmd.d.ReadI2CMultiInfo.Read;

	for (int i = 0; i < NumRegs; i++)
	{
		pReadDevByteInfo->DevAddress = DevAddress;
		pReadDevByteInfo->Register = *pRegisters;
		pReadDevByteInfo++;
		pRegisters++;
	}

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();
	unsigned char inEndpoint = getCmdInEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err == ERR_NO_ERROR)
	{
		transferred = 0;

		err = syncBulkTransfer(inEndpoint, pData, NumRegs, &transferred, 1000);
	}

	if(err)
		throw UlException(err);
}

void Usb9837x::Cmd_WriteDevMultipleRegs( unsigned char DevAddress, unsigned char NumRegs, unsigned char *pRegisters, unsigned char *pData ) const
{
	if (NumRegs > Usb9837xDefs::MAX_NUM_MULTI_BYTE_WRTS)
	{
		std::cout << "Too many Entries requested NumRegs="<< NumRegs << "too large" << std::endl;
	}

	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	Cmd.CmdCode = Usb9837xDefs::W_MULTI_BYTE_I2C_REG;
	Cmd.d.WriteI2CMultiInfo.NumWrites = NumRegs;

	Usb9837xDefs::pWRITE_I2C_BYTE_INFO pWriteDevByteInfo = Cmd.d.WriteI2CMultiInfo.Write;

	for (int i = 0; i < NumRegs; i++)
	{
		pWriteDevByteInfo->DevAddress = DevAddress;
		pWriteDevByteInfo->Register = *pRegisters;
		pWriteDevByteInfo->DataVal = *pData;
		pWriteDevByteInfo++;
		pRegisters++;
		pData++;
	}

	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);
}

void Usb9837x::CmdSetAnalogTriggerThreshold( double volts) const
{
	Usb9837xDefs::USB_CMD	Cmd;
	memset(&Cmd, 0, sizeof(Cmd));

	unsigned char AnalogThreshold;

	// Voltage range for the threshold trigger is 0.2 V to 9.8 V

	// fix any out of range requests before converting
	double VoltageRangeHigh = 9.8;
	double VoltageRangeLow = 0.2;
	int Resolution = 8;

	//int intVolts = (int)(volts * 1000);

	double dfFullScaleRange = VoltageRangeHigh - VoltageRangeLow;

	//double oneLsb = dfFullScaleRange / (1 <<  Resolution);
	/*if ((volts >= VoltageRangeHigh) || (volts <= VoltageRangeLow))
	{
		t << "Threshold volts is out of range"<<"\n";
		return OLBADVALUE;

	}*/

	double dfRatioVoltsToFullScale = ( ( volts) - VoltageRangeLow ) / dfFullScaleRange;

	AnalogThreshold = (unsigned char)(dfRatioVoltsToFullScale * ( (double) (1 <<  Resolution)));

	//t << "CmdSetAnalogTriggerThreshold: returned "<< AnalogThreshold <<"\n";

	Cmd.CmdCode = Usb9837xDefs::WRITE_CAL_POT;
	Cmd.d.WriteCalPotInfo.ChipNum = 0x2;
	Cmd.d.WriteCalPotInfo.PotNum = 3;
	Cmd.d.WriteCalPotInfo.RegNum = 0xFF; // Wiper
	Cmd.d.WriteCalPotInfo.DataVal = AnalogThreshold;


	UlError err = ERR_NO_ERROR;
	int transferred = 0;

	unsigned char outEndpoint = getCmdOutEndpointAddr();

	UlLock lock(mIoMutex);

	err = syncBulkTransfer(outEndpoint, (unsigned char*)&Cmd, sizeof (Usb9837xDefs::USB_CMD), &transferred, 1000);

	if(err)
		throw UlException(err);

	//send the command to the board
	/*ULONG ulBytesSent = 0;
	ntStatus = m_rDt9837Device.WriteData( (BYTE*)&Cmd, sizeof (USB_CMD), &ulBytesSent,
											m_rDt9837Device.m_Pipe[WRITE_CMD_PIPE] );
	return ntStatus;*/
}



/*****************************************************
Description: Calculates the VCO Frequency
Parameter1: Input frequency Mhz
Parameter2: Address of variable to hold Vco Frequency
Parameter3: Address of variable to hold DivIn
Output:		 NONE
******************************************************/
void  Usb9837x::FindVCOfreq (double infreq, double* vcofreq, int* DivIn)
{
	double initvcofreq = 0.0;

	initvcofreq = infreq * (*DivIn);

	if(initvcofreq < 100){
		(*DivIn) = (*DivIn) + 1;// inc up 1
	}
	if(initvcofreq > 400){
		(*DivIn) = (*DivIn) - 1;// dec down 1
	}
	*vcofreq = infreq * (*DivIn);
}

/*****************************************************
Description: Calculates the Maximum Q allowed
Parameter1: Input reference frequency Mhz
Output:		 returns QMax
******************************************************/
int Usb9837x::FindQMax(double freqref)
{
	double qtotal = 0;
	int q;

	// round qtotal
	qtotal = (freqref + HALF_Q_COUNTER_MIN_FREQ) / (double)Q_COUNTER_MIN_FREQ;

	q =(int)qtotal;

	if(q < 2){
		q = 2;
	}
	else if (q > 129){
		q = 129;
	}

	return q;
}

/*****************************************************
Description: Calculates the minimumDivider Ratio
Parameter1: Frequency Mhz
Parameter2: Address of variable to hold DivIn
Output:		NONE
******************************************************/
void Usb9837x::FindDivRatioIn (double freq, int* DivIn)
{
	double DivMin =0;
	int div = 0;

	DivMin = (VCO_MIN_FREQ + ((double) freq / (double)2))/ freq;

	div =(int)DivMin;

	// is divmin within required range?
	if(div <4){
		div = 4;
	}
	else if(div > 128){
		div = 128;
	}
	*DivIn = div;
}

/*****************************************************
Description: Calculates quotient P / Q using fref and vco frequency
Parameter1: VCO frequency Mhz
Parameter2: Ref frequency Mhz
Output:		 returns QMax
******************************************************/
double Usb9837x::PdivByQ(double vcofreq, double fref)
{
	double PdivQ = 99999.22222;;

	PdivQ = vcofreq / fref;

	return PdivQ;
}

// used by qsort in optimumPQF()
int compare ( const void *x, const void *y)
{
	int *a, *b;

	a = (int*)x;
	b = (int*)y;

	return (*a - *b);
}

/****************************************************************************/
/* Using P/Q ratio find optimum values for PTot and QTot that will give frequency */
/* as close to desired as possible. Pass back P, Q, and Freqout,				*/
/****************************************************************************/
/*****************************************************
Description: Using P/Q ratio finds optimum values for
			 PTot and QTot that will give frequency as
			 close to desired as possible.
Parameter1:	Desired frequency Mhz
Parameter2:	Pdiv_Q value
Parameter3:	Reference frequency Mhz
Parameter4:	qmax
Parameter5:	address to hold Popt
Parameter6:	address to hold Qopt
Parameter7:	address to hold fout
Parameter7:	address to hold errorout
Parameter7:	address to hold count
Output:		NONE

******************************************************/
void Usb9837x::optimumPQF(double desiredfreq, double Pdiv_Q, double fref, int qmax,
						  int* Popt, int* Qopt, double* fout, int* errorout, int* count)
{

	int i,q, minbin, P, qcount;
	double p, y, n, error;
	int* pArray;
	double* freqoutArray;
	int* sortingArray;
	int* errorArray;

	pArray = (int *) new int [ASIZE];
	freqoutArray = (double *) new double [ASIZE];
	sortingArray = (int *) new int [ASIZE];
	errorArray = (int *) new int [ASIZE];

	// zero out the arrays memset
	for(i = 0; i < ASIZE; i++){
	freqoutArray[i] = 0.0;
	pArray[i] = errorArray[i] = 0;
	}

	minbin = 3;// set inital value for minbin

	for (q = 3; q<= qmax; q++){
	p = (double)(Pdiv_Q * q);// find decimal vale for p/q
	n = (int) (Pdiv_Q * q);

	y = p - n;


	if (y >= 0.5)
		P =(int) n + 1;
	else
		P =(int) n;

	if (P>2055)
		break;

	pArray[q-3] = P;// save the value of P
	freqoutArray[q-3] = (double)(fref * ((double)P/(double)q) );// calculate and save vco frequency for eac
																// set of p and q
	// error = fabs ( (10000*(freqoutArray[q-3] - desiredfreq)) );// calculate the error
	error = ( (10000*(freqoutArray[q-3] - desiredfreq)) );// calculate the error
	n = (int)error;

	if (n <0)
	{
		error = error * -1;
		n = (int)error;
	}


	y = error - n;

	if(y >=0.5)
		n++;

	errorArray[q-3] = sortingArray[q-3] = (int)n; // store error in two arrays,
												//one used to sort and one to compare to
	}
	qcount = q-3;
	*count = qcount;

	qsort(sortingArray, qcount, sizeof(int), compare);// sort in ascending order

	for(i = 0; i < qcount; i++){// fond first occurence of error in unsorted error array
		if(errorArray[i] == sortingArray[0]){
			minbin = i+3;
			break;
		}
	}
	*Qopt = minbin;
	*Popt = pArray[minbin-3];
	*fout = freqoutArray[minbin-3];
	*errorout = sortingArray[0];

	delete [] errorArray;
	delete [] sortingArray;
	delete [] freqoutArray;
	delete [] pArray;
}

/*****************************************************
Description: Calculates P and Q values
Parameter1:	PTotal
Parameter2:	QTotal
Parameter3:	Address to hold value for p
Parameter4:	Address to hold value for q
Output:		NONE
******************************************************/
void Usb9837x::CalculatePQValues(int PTotal, int QTotal, int* P, int* Q)
{
	int P0 = 0;

	if(PTotal % 2 == 0)
		P0 = 0;
	else
		P0 = 1;

	*P = (PTotal -P0)/2 - 4;
	*Q = (P0 * 128) + (QTotal - 2);
}

/*****************************************************
Description: Calculates Charge Pump settings
Parameter1:	PVal
Output:		returns charge pump value
******************************************************/
int Usb9837x::FindChargePumpSetting(int PVal)
{
	int pump=0;
	if(PVal >=16 && PVal <45)pump = 0;
	if(PVal >=45 && PVal<480)pump = 1;
	if (PVal >=480 && PVal<640)pump = 2;
	if (PVal >=640 && PVal<800)pump = 3;
	if (PVal >=800 && PVal<1024)pump = 4;
	return pump;
}

/*****************************************************
Description: Programs PLL Oscillator Clock
Parameter1:	Input frequency Hz
Parameter2:	Reference Frequency Mhz
Parameter3:	Address of variable to hold actual frequency programmed
Parameter4:	Address of register structure variable to hold PLL
				register values
Output:		NONE
******************************************************/
void Usb9837x::programClock(double FreqIn,double FreqRef ,double* actualFreq, Usb9837xDefs::CY22150REGISTERS *CY22150Registers, double* fOut, unsigned char* pDivider,bool isDac)
{
	unsigned char dividers [4] = {2,4,8,16};

	int Error, count, Qopt, Popt, QMax, DivN, P, Q, chargePump;
	double FreqOut, VCOFreq, PdivQ;
	double ClockTicks;

	if (isDac)
	{
		ClockTicks = 512.0;
	}
	else
	{
		// conversion constant depends on desired frequency
		if (FreqIn > (double)ADS1271_LOW_POWER_UPPER_FREQ)
			ClockTicks = 256.0;
		else
			ClockTicks = 512.0;
	}

	DivN = 0;
	chargePump = 0;
	Q= 0;
	P= 0;

	// Set the divider to the minimum and keep dividing until we
	// establish a DivN <=127 which is the maximum value to divide by
	double curFreq = FreqIn;
	for ( int i =0; i < 4;i++)
	{
		*pDivider = dividers[i];

		curFreq = FreqIn * ClockTicks * dividers[i] / 1000000.0;

		// calculate parameters
		QMax = FindQMax(FreqRef);
		FindDivRatioIn (curFreq, &DivN);
		FindVCOfreq (curFreq, &VCOFreq, &DivN);

		PdivQ = PdivByQ(VCOFreq,FreqRef);

		optimumPQF(VCOFreq, PdivQ, FreqRef, QMax,
				&Popt, &Qopt, &FreqOut, &Error, &count);

		chargePump = FindChargePumpSetting(Popt);
		CalculatePQValues(Popt, Qopt, &P, &Q);

		*actualFreq = 1000000.0 * FreqOut / (double)DivN / (ClockTicks * dividers[i]);
		*fOut = FreqOut;
		*pDivider = dividers[i];

		if (DivN < 128)
			break;
	}

	// update structure register variables
	CY22150Registers->InputCrysOscCtrl = 0; // assume ref oscillator in range of 25 - 50 MHz
	CY22150Registers->Div1Src = (unsigned char)DivN;// use reference divided by 55
	CY22150Registers->Div2Src = (unsigned char)183;// divider ratio and set vco mode
	CY22150Registers->ChargePump = (unsigned char)((P >> 8) | 0xC0);// upper P values
	CY22150Registers->ChargePump = (unsigned char)(CY22150Registers->ChargePump | (chargePump << 2));// charge pump reg values
	CY22150Registers->PBCounter = (unsigned char)(P & 0xFF);// lower P values
	CY22150Registers->POQCounter = (unsigned char)(Q & 0xFF);// q values
	CY22150Registers->CrossPointSw0 = 0x3F;// set up LCLK1 for DIV1-x
	CY22150Registers->CrossPointSw1 = 0xFF;//0;// disconnect LCLK3, LCLK4, LCLK5, LCLK6
	CY22150Registers->CrossPointSw2 = 0xFF;// disconnect LCLk6
	CY22150Registers->ClkOe = 0x01;//bmBIT0;// enable LCLK1 output
}

} /* namespace ul */
