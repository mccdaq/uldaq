/*
 * VirNetDaqDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "VirNetDaqDevice.h"

namespace ul
{

VirNetDaqDevice::VirNetDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor) : NetDaqDevice(daqDeviceDescriptor)
{
	// TODO Auto-generated constructor stub

}

VirNetDaqDevice::~VirNetDaqDevice()
{
	// TODO Auto-generated destructor stub
}

// this function just sends a dummy byte to force ack to sent
// immediatly for the previous transaction
void VirNetDaqDevice::flushCmdSocket() const
{
	//TODO use base class or implement

	/*UlLock lock(mTcpCmdMutex);

	unsigned char buf = 0;
	send(mSockets.tcpCmd, &buf, sizeof(buf), 0);*/
}

unsigned short VirNetDaqDevice::readStatus() const
{
	unsigned short status = 0;

	//TODO use base class or implement

	//queryCmdVir(CMD_STATUS, NULL, 0, (unsigned char*) &status, sizeof(status));

	return status;
}

void VirNetDaqDevice::flashLed(int flashCount) const
{
	TFlashLedParams params;

	params.flashCount = flashCount;

	unsigned char remote_ul_err = 0;

	queryCmdVir(VNC_FLASH_LED, (unsigned char*)&params, sizeof(params), &remote_ul_err);

	if(remote_ul_err)
		throw UlException((UlError) remote_ul_err);
}

int VirNetDaqDevice::memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count, false);

	int totalBytesRead = 0;

	//TODO use base class or implement

/*	unsigned short bytesToRead = 0;
	unsigned short addr = 0;
	int totalBytesRead = 0;
	int bytesRead = 0;
	int remaining = count;
	unsigned char cmd;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	const int maxTransfer = 512;

	if(memRegionType == MR_CAL)
		cmd = CMD_CAL_MEM_R;
	else if(memRegionType == MR_USER)
		cmd = CMD_USER_MEM_R;
	else if(memRegionType == MR_SETTINGS)
		cmd = CMD_SETTINGS_MEM_R;
	else
		throw UlException(ERR_BAD_MEM_REGION);

#pragma pack(1)
	struct
	{
		unsigned short 	address;
		unsigned short  bytesToRead;
	}params;
#pragma pack()

	unsigned char* readBuff = buffer;

	addr = address;

	do
	{
		bytesToRead = remaining > maxTransfer ? maxTransfer : remaining;

		params.address = Endian::cpu_to_le_ui16(addr);
		params.bytesToRead = Endian::cpu_to_le_ui16(bytesToRead);

		bytesRead  = queryCmdVir(cmd, (unsigned char*)&params, sizeof(params), readBuff, bytesToRead);

		remaining-= bytesRead;
		totalBytesRead += bytesRead;
		addr += bytesRead;
		readBuff += bytesRead;
	}
	while(remaining > 0);*/

	return totalBytesRead;
}
int VirNetDaqDevice::memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_WRITE, address, buffer, count, false);

	int totalBytesWritten = 0;

	//TODO use base class or implement

	/*
	unsigned short bytesToWrite = 0;
	unsigned short addr = 0;
	int bytesWritten = 0;
	int bytesRemaining = count;
	unsigned char cmd;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);



	const int maxTransfer = 512 - 2;

	if(memRegionType == MR_USER)
		cmd = CMD_USER_MEM_W;
	else if(memRegionType == MR_SETTINGS)
	{
		if(getMemUnlockCode() != MEM_UNLOCK_CODE)
			throw UlException(ERR_MEM_ACCESS_DENIED);

		cmd = CMD_SETTINGS_MEM_W;
	}
	else
		throw UlException(ERR_BAD_MEM_REGION);

#pragma pack(1)
	struct
	{
		unsigned short address;
		unsigned char  buf[maxTransfer];
	}params;
#pragma pack()

	addr = address;

	unsigned char* writeBuff = buffer;

	while(bytesRemaining > 0)
	{
		bytesToWrite = bytesRemaining > maxTransfer ? maxTransfer : bytesRemaining;

		params.address = Endian::cpu_to_le_ui16(addr);
		memcpy(params.buf, writeBuff, bytesToWrite);

		queryCmd(cmd, (unsigned char*)&params, bytesToWrite + 2);

		bytesWritten = bytesToWrite;
		bytesRemaining -= bytesWritten;
		totalBytesWritten += bytesWritten;
		addr += bytesWritten;
		writeBuff += bytesWritten;
	}
*/
	return totalBytesWritten;
}



UlError VirNetDaqDevice::openDataSocket(int timeout /* ms */) const
{
	UlError err = ERR_NO_ERROR;
	bool opened = false;

	err = initTcpDataSocket(timeout);

	if(!err)
	{
		opened = isDataSocketReady();
	}
	else
		std::cout << "$$$$$$$$$$$ initTcpDataSocket failed $$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;

	if(!opened)
		err = ERR_DATA_SOCKET_CONNECTION_FAILED;

	//TODO use base class or implement

	/*bool opened = false;

	// Sometimes status command does not return  connected status so we try connecting twice
	for(int retry = 0; retry < 2; retry++)
	{
		err = initTcpDataSocket(timeout);

		if(!err)
		{
			opened = isDataSocketReady();

			if(opened)
				break;
			else
			{
				if(mSockets.tcpData != -1)
				{
					shutdown(mSockets.tcpData, SHUT_RDWR);
					close(mSockets.tcpData);
					mSockets.tcpData = -1;
				}

				// also close the socket from the device end
				closeDataSocketFromDevice();
				usleep(10000);
			}
		}
	}*/

	return err;
}

bool VirNetDaqDevice::isDataSocketReady() const
{
	//unsigned short status = 0;
	bool ready = false;
	int retryCount = 10;
	int retryNum = 0;

	TXferInState state;

	do
	{
		if(retryNum != 0)
		{
			usleep(100);
		}

		retryNum++;

		state = getXferInState();
	}
	while(!state.dataSocketReady && retryNum <= retryCount);

	ready = state.dataSocketReady;


	//TODO use the base class or implement

	/*
	int retryCount = 10;
	int retryNum = 0;

	do
	{
		retryNum++;

		status = readStatus();
	}
	while(!(status & STATUS_DATA_SOCKET_CONNECTED) && retryNum <= retryCount);

	ready = status & STATUS_DATA_SOCKET_CONNECTED ? true : false;*/

	return ready;
}

void VirNetDaqDevice::closeDataSocketFromDevice() const
{
	//TODO use the base class or implement

	/*
	unsigned char cmd = 0x13; // CMD_AINSTOP command
	unsigned char	closeSocket = 1;

	queryCmd(cmd, &closeSocket, sizeof(closeSocket));*/
}

TXferInState VirNetDaqDevice::getXferInState() const
{
	TXferInState state;

	unsigned char status = 0;

	queryCmdVir(VNC_XFER_IN_STATE, NULL, 0, (unsigned char*) &state, sizeof(state), &status);

	return state;
}




} /* namespace ul */
