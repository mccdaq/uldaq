/*
 * NetDaqDevice.cpp
 *
 *     Author: Measurement Computing Corporation
 */
#include "NetDaqDevice.h"
#include "../DaqDeviceManager.h"
#include "../DaqEventHandler.h"
#include "../utility/UlLock.h"
#include "NetScanTransferIn.h"

#include <numeric>

namespace ul
{

NetDaqDevice::NetDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor) : DaqDevice(daqDeviceDescriptor)
{
	mConnectionCode = 0;
	mConnectionTimeout = DEFAULT_CONNECTION_TO;
	mIoTimeout = DEFAULT_IO_TO;

	UlLock::initMutex(mConnectionMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mUdpMutex, PTHREAD_MUTEX_RECURSIVE);
	UlLock::initMutex(mTcpCmdMutex, PTHREAD_MUTEX_RECURSIVE);

	/* mNetDiscoveryInfo will be set in connectToDevice() method later.
	   Set the discovery info for now so the getConfig is able to provide the ip address and the interface name */
	mNetDiscoveryInfo = NetDiscovery::getDiscoveryInfo(daqDeviceDescriptor.uniqueId);

	mScanTransferIn = new NetScanTransferIn(*this);
}

NetDaqDevice::~NetDaqDevice()
{
	disconnect();

	delete mScanTransferIn;

	UlLock::destroyMutex(mConnectionMutex);
	UlLock::destroyMutex(mUdpMutex);
	UlLock::destroyMutex(mTcpCmdMutex);
}

void NetDaqDevice::connect()
{
	FnLog log("NetDaqDevice::connect");

	UlLock lock(mConnectionMutex);

	if(mConnected)
	{
		UL_LOG("Device is already connected, disconnecting...");

		disconnect();
	}

	establishConnection();

	mConnected = true;

	initilizeHardware();

	initializeIoDevices();

	// start the daq event handler if daq events are already enabled
	if(mEventHandler->getEnabledEventTypes())
		mEventHandler->start();
}

void NetDaqDevice::disconnect()
{
	FnLog log("NetDaqDevice::disconnect");

	if(mConnected)
	{
		DaqDevice::disconnect();

		releaseNetResources();
	}
}

void NetDaqDevice::releaseNetResources()
{
	FnLog log("NetDaqDevice::releaseUsbResources");

	closeSockets();
}

void NetDaqDevice::closeSockets()
{
	UlLock lock(mTcpCmdMutex);

	if(mSockets.udp != -1)
	{
		close(mSockets.udp);
		mSockets.udp = -1;
	}

	if(mSockets.tcpCmd != -1)
	{
		shutdown(mSockets.tcpCmd, SHUT_RDWR);
		close(mSockets.tcpCmd);
		mSockets.tcpCmd = -1;
	}

	if(mSockets.tcpData != -1)
	{
		shutdown(mSockets.tcpData, SHUT_RDWR);
		close(mSockets.tcpData);
		mSockets.tcpData = -1;
	}
}

void NetDaqDevice::establishConnection()
{
	FnLog log("NetDaqDevice::establishConnection");

	NetDiscovery::NetDiscoveryInfo discoveryInfo = NetDiscovery::getDiscoveryInfo(mDaqDeviceDescriptor.uniqueId);

	if(!discoveryInfo.valid)
		discoveryInfo = mNetDiscoveryInfo;  // use the last discovery info if discovery info for the specified device is not available anymore

	if(discoveryInfo.valid)
	{
		if(NetDiscovery::isNetIfcAvaiable(discoveryInfo.ifcName))
		{
			NetDiscovery::NetIfcDesc ifcDesc = NetDiscovery::getNetIfcDescs(discoveryInfo.ifcName)[0];

			UlError err = initUdpSocket(ifcDesc, discoveryInfo);

			if(!err)
			{
				// make sure the mac address of the connected device match what we are looking for. This check is performed
				// to make sure that the DHCP server has not assigned the ip address of the target device to another device since
				// user has called getDeviceInventory
				if(isValidDevice(discoveryInfo.macAddr))
				{
					err = initTcpCmdSocket(ifcDesc, discoveryInfo);

					if(!err)
					{
						mNetDiscoveryInfo = discoveryInfo;
						mNetIfcDesc = ifcDesc;
						mRawFwVersion = discoveryInfo.fwVer;
					}
				}
				else
					err = ERR_NET_CONNECTION_FAILED;
			}

			if(err)
			{
				closeSockets();
				throw UlException(err);
			}
		}
		else
			throw UlException(ERR_NET_IFC_UNAVAILABLE);

	}
	else
		throw UlException(ERR_DEV_NOT_FOUND);
}

UlError NetDaqDevice::initUdpSocket(const NetDiscovery::NetIfcDesc& ifcDesc, const NetDiscovery::NetDiscoveryInfo& discoveryInfo) const
{
	FnLog log("NetDaqDevice::initUdpSocket");

	UlError err = ERR_NET_CONNECTION_FAILED; // don't change

	mSockets.udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(mSockets.udp != -1)
	{
		// bind to network interface (any port)
		if(bind(mSockets.udp, (sockaddr*) &ifcDesc.addr, sizeof(sockaddr)) == 0)
		{
			sockaddr_in targetAddr = {0};
			targetAddr.sin_family = AF_INET;
			targetAddr.sin_addr = discoveryInfo.ipAddr;
			targetAddr.sin_port = htons(discoveryInfo.discoveryPort);

			// bind to DAQ device so we can use send() rather than sendto()
			if(::connect(mSockets.udp, (sockaddr*) &targetAddr, (socklen_t) sizeof(sockaddr)) == 0)
			{
				err = ERR_NO_ERROR;
			}
			else
				UL_LOG("UDP socket connection failed, connect() error: " << strerror(errno));
		}
		else
			UL_LOG("UDP socket bind failed, bind() error: " << strerror(errno));

		if(err)
		{
			close(mSockets.udp);
			mSockets.udp = -1;
		}
	}
	else
		UL_LOG("UDP socket creation failed, socket() error: " << strerror(errno));

	return err;
}

UlError NetDaqDevice::initTcpCmdSocket(const NetDiscovery::NetIfcDesc& ifcDesc, const NetDiscovery::NetDiscoveryInfo& discoveryInfo) const
{
	FnLog log("NetDaqDevice::initTcpCmdSocket");

	UlError err = sendConnectionCode();

	if(err)
		return err;

	err = ERR_NET_CONNECTION_FAILED; // don't change

	mSockets.tcpCmd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(mSockets.tcpCmd != -1)
	{
		 linger sl;
		 sl.l_onoff = 1;
		 sl.l_linger = 0;

		 // set the socket option to hard shutdown
		 if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		 int keepAlive = 1;
		 if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		 timeval to = NetDiscovery::convertTimeout(mConnectionTimeout);
		 if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

#ifdef __APPLE__
		// MSG_NOSIGNAL flag is not defined in macOS, SO_NOSIGPIPE socket option must be enabled instead.
		int nosig = 1;
		if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_NOSIGPIPE, &nosig, sizeof(nosig)) == -1)
			print_setsockopt_error(errno, __FILE__, __LINE__);
#endif

		if(bind(mSockets.tcpCmd, (sockaddr*) &ifcDesc.addr, sizeof(sockaddr)) == 0)
		{
			sockaddr_in targetAddr = {0};
			targetAddr.sin_family = AF_INET;
			targetAddr.sin_addr = discoveryInfo.ipAddr;
			targetAddr.sin_port = htons(discoveryInfo.discoveryPort);

			if(::connect(mSockets.tcpCmd, (sockaddr*) &targetAddr, (socklen_t) sizeof(sockaddr)) == 0)
			{
				err = ERR_NO_ERROR;
			}
			else
				UL_LOG("TCP cmd socket connection failed, connect() error: " << strerror(errno));
		}
		else
			UL_LOG("TCP cmd socket bind failed, bind() error: " << strerror(errno));

		if(err)
		{
			close(mSockets.tcpCmd);
			mSockets.tcpCmd = -1;
		}
	}
	else
		UL_LOG("TCP cmd socket creation failed, socket(() error: " << strerror(errno));

	return err;
}

UlError NetDaqDevice::initTcpDataSocket(int timeout /* ms */) const
{
	FnLog log("NetDaqDevice::initTcpDataSocket");

	UlError err = ERR_DATA_SOCKET_CONNECTION_FAILED;

	if(mSockets.tcpData != -1)
	{
		shutdown(mSockets.tcpData, SHUT_RDWR);
		close(mSockets.tcpData);
		mSockets.tcpData = -1;
	}

	mSockets.tcpData = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(mSockets.tcpData != -1)
	{
		 linger sl;
		 sl.l_onoff = 1;
		 sl.l_linger = 0;

		 // set the socket option to hard shutdown
		 if(setsockopt(mSockets.tcpData, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		 int keepAlive = 1;
		 if(setsockopt(mSockets.tcpData, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		 timeval to = NetDiscovery::convertTimeout(mConnectionTimeout);
		 if(setsockopt(mSockets.tcpData, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		if(bind(mSockets.tcpData, (sockaddr*) &mNetIfcDesc.addr, sizeof(sockaddr)) == 0)
		{
			sockaddr_in targetAddr = {0};
			targetAddr.sin_family = AF_INET;
			targetAddr.sin_addr = mNetDiscoveryInfo.ipAddr;
			targetAddr.sin_port = htons(mNetDiscoveryInfo.discoveryPort + 1);

			if(::connect(mSockets.tcpData, (sockaddr*) &targetAddr, (socklen_t) sizeof(sockaddr)) == 0)
			{
				to = NetDiscovery::convertTimeout(timeout);

				if(setsockopt(mSockets.tcpData, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) == -1)
					 print_setsockopt_error(errno, __FILE__, __LINE__);

				err = ERR_NO_ERROR;
			}
			else
				UL_LOG("TCP data socket connection failed, connect() error: " << strerror(errno));
		}
		else
			UL_LOG("TCP data socket bind failed, bind() error: " << strerror(errno));

		if(err)
		{
			close(mSockets.tcpData);
			mSockets.tcpData = -1;
		}
	}

	return err;
}

std::string NetDaqDevice::getMacAddress()
{
	std::string macAddr = "";

	char sendBuf = NetDiscovery::DISCOVERY_CMD;

	NetDiscovery::NetDevDesc netDevDesc;
	char buffer[NetDiscovery::UDP_MSG_MAX_LEN];

	unsigned int bufferLen =  NetDiscovery::UDP_MSG_MAX_LEN;

	UlError err = queryUdp(&sendBuf, sizeof(sendBuf), buffer, &bufferLen, mIoTimeout);

	if(err == ERR_NO_ERROR)
	{
		memcpy(&netDevDesc, &buffer[1], sizeof(netDevDesc));

		char mac[18];
		snprintf(mac,sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", netDevDesc.mac[0], netDevDesc.mac[1], netDevDesc.mac[2], netDevDesc.mac[3], netDevDesc.mac[4], netDevDesc.mac[5]);

		macAddr = mac;
	}

	return macAddr;
}

bool NetDaqDevice::isValidDevice(std::string macAddr)
{
	FnLog log("NetDaqDevice::isValidDevice");

	bool validDev = false;

	std::string devMacAddr = getMacAddress();

	if(macAddr == devMacAddr)
		validDev = true;

	return validDev;
}

void NetDaqDevice::resetDevice()
{
	unsigned char cmd = CMD_RESET;

	queryCmd(cmd);

	disconnect();

	// allow 4 seconds for device to bootup and obtain ip address (2.5 seconds seems to enough, just to be safe wait 4 seconds)
	usleep(4000000);
}

UlError NetDaqDevice::queryUdp(char* sendBuf, unsigned int sendBufLen, char* receiveBuf, unsigned int* receiveBufLen, int timeout) const
{
	FnLog log("NetDaqDevice::queryUdp");

	UlLock lock(mUdpMutex);

	UlError err = ERR_NO_ERROR;

	timeval to = NetDiscovery::convertTimeout(timeout);

	if(mSockets.udp != -1)
	{
		if(setsockopt (mSockets.udp, SOL_SOCKET, SO_RCVTIMEO, (char*) &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		int sent = send(mSockets.udp, sendBuf, sendBufLen, 0);

		if(sent == (int)sendBufLen)
		{
			int received = recv(mSockets.udp, receiveBuf, *receiveBufLen, 0);

			if(received > 0)
				*receiveBufLen = received;
			else
			{
				UL_LOG("UDP receive failed , recv() error: " << strerror(errno));
				err = ERR_DEAD_DEV;
			}
		}
		else
		{
			UL_LOG("UDP send , send() error: " << strerror(errno));
			err = ERR_DEAD_DEV;
		}
	}
	else
	{
		UL_LOG("Invalid UDP socket");

		err = ERR_DEV_NOT_CONNECTED;
	}

	return err;
}

void NetDaqDevice::queryCmd(unsigned char cmd) const
{
	UlError err = queryTcp(cmd, NULL, 0, NULL, 0, NULL, NULL, mIoTimeout);

	if(err)
		throw UlException(err);
}
void NetDaqDevice::queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen) const
{
	UlError err = queryTcp(cmd, sendBuf, sendBufLen, NULL, 0, NULL, NULL, mIoTimeout);

	if(err)
		throw UlException(err);
}

void NetDaqDevice::queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* status) const
{
	UlError err = queryTcp(cmd, sendBuf, sendBufLen, NULL, 0, NULL, status, mIoTimeout);

	if(err)
		throw UlException(err);
}

unsigned int NetDaqDevice::queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen , unsigned char* receiveDataBuf, unsigned short receiveDataBufLen) const
{
	unsigned short bytesReceived = 0;
	UlError err = queryTcp(cmd, sendBuf, sendBufLen, receiveDataBuf, receiveDataBufLen, &bytesReceived, NULL, mIoTimeout);

	if(err)
		throw UlException(err);

	return bytesReceived;
}

unsigned int NetDaqDevice::queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen , unsigned char* receiveDataBuf, unsigned short receiveDataBufLen, unsigned char* status) const
{
	unsigned short bytesReceived = 0;
	UlError err = queryTcp(cmd, sendBuf, sendBufLen, receiveDataBuf, receiveDataBufLen, &bytesReceived, status, mIoTimeout);

	if(err)
		throw UlException(err);

	return bytesReceived;
}

UlError NetDaqDevice::queryTcp(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* receiveBuf, unsigned short receiveBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const
{
	FnLog log("NetDaqDevice::query");

	UlError err = ERR_NO_ERROR;

	UlLock lock(mTcpCmdMutex);

	static unsigned char frameId = 0;

	int retry = 2;

	do
	{
		frameId += 1;

		err = sendFrame(cmd, frameId, sendBuf, sendBufLen, timeout);

		if(!err)
		{
			err = receiveFrame(cmd, frameId, receiveBuf, receiveBufLen, bytesReceived, status, timeout);

			if(err == ERR_BAD_NET_FRAME)
			{
				clearSocketInputQueue();
				retry--;
			}
		}
	}
	while(err == ERR_BAD_NET_FRAME && retry > 0);

	return err;
}

UlError NetDaqDevice::sendFrame(unsigned char cmd, unsigned char frameId, unsigned char* buf, unsigned short bufLen, int timeout) const
{
	FnLog log("NetDaqDevice::sendFrame");
	UlError err = ERR_NO_ERROR;

	if(mConnected)
	{
		/*if(mSockets.tcpCmd == -1)
		{
			NetDiscovery::NetIfcDesc ifcDesc = NetDiscovery::getNetIfcDescs(mNetDiscoveryInfo.ifcName)[0];
			err = initTcpCmdSocket(ifcDesc, mNetDiscoveryInfo);

			if(err)
				return err;
		}*/

		int frameSize = sizeof(NetFrame) + bufLen; // no need to subtract one (checksum is not member of the NET_FRAME structure)
		int chksumIndex = frameSize - 1;

		if(frameSize > MAX_SEND_FRAME_SIZE)
			return ERR_BAD_BUFFER_SIZE;

		unsigned char frameBuf[MAX_SEND_FRAME_SIZE];
		memset(frameBuf, 0, MAX_SEND_FRAME_SIZE);
		NetFrame* frame = (NetFrame*) frameBuf;

		frame->delimiter = FRAME_START;
		frame->command = cmd;
		frame->frameId = frameId;
		frame->count = Endian::cpu_to_le_ui16(bufLen);
		memcpy(frame->data, buf, bufLen);

		unsigned char chksum = std::accumulate(&frameBuf[0], &frameBuf[chksumIndex], 0);
		chksum = (unsigned char)(0xff) - chksum;

		frameBuf[chksumIndex] = chksum;

		timeval to = NetDiscovery::convertTimeout(timeout);
		if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		int flag = 0;

#ifndef __APPLE__
		// MSG_NOSIGNAL flag is not defined in macOS. For macOS, we set the SO_NOSIGPIPE socket option when socket is created instead.
		flag = MSG_NOSIGNAL;
#endif

		int sent = send(mSockets.tcpCmd, frameBuf, frameSize, flag);

		if(sent != frameSize)
		{
			err = ERR_DEV_NOT_CONNECTED;//ERR_NET_CONNECTION_FAILED;

			if(sent == -1)
				UL_LOG("sendFrame failed, send() error: " << strerror(errno));
		}
	}
	else
		err = ERR_DEV_NOT_CONNECTED;

	return err;
}

UlError NetDaqDevice::receiveFrame(unsigned char cmd, unsigned char frameId, unsigned char* dataBuf, unsigned short dataBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const
{
	FnLog log("NetDaqDevice::receiveFrame");
	UlError err = ERR_NO_ERROR;

	if(mConnected)
	{
		unsigned char frameBuf[MAX_SEND_FRAME_SIZE];
		NetFrame* frame = (NetFrame*) frameBuf;
		//unsigned int frameBufLen = sizeof(frameBuf);
		int frameSize = 0;

		if(bytesReceived)
			*bytesReceived = 0;

		if(status)
			*status = 0;

		timeval to = NetDiscovery::convertTimeout(timeout);
		if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		NetFrame frameHeader;

		// peek at the frame header to obtain the actual frame size
		int received = recv(mSockets.tcpCmd, (char*)&frameHeader, sizeof(NetFrame), MSG_PEEK | MSG_WAITALL);

		if(received == sizeof(frameHeader))
		{
			frameSize = frameHeader.count + sizeof(NetFrame);

			received = recv(mSockets.tcpCmd, frameBuf, frameSize, MSG_WAITALL);

			if(received == frameSize)
			{
				unsigned char chksum = std::accumulate(&frameBuf[0], &frameBuf[received], 0);

				if(chksum != 0xff)
				{
					UL_LOG("Invalid frame checksum!!!!");
					err = ERR_BAD_NET_FRAME;
				}
				else if(frame->command != (cmd | 0x80))
				{
					UL_LOG("Invalid frame command!!!!");
					err = ERR_BAD_NET_FRAME;
				}
				else if(frame->frameId != frameId)
				{
					UL_LOG("Invalid frame ID!!!!");
					err = ERR_BAD_NET_FRAME;
				}
				else if (frame->count > dataBufLen)
				{
					UL_LOG("Invalid buffer size!!!!");
					err = ERR_BAD_BUFFER_SIZE;
				}
				else
				{
					if(dataBuf)
					{
						memcpy(dataBuf, frame->data, frame->count);

						if(bytesReceived) // if the pointer is not null set its value
							*bytesReceived = frame->count;
					}

					if(status)
						*status = frame->status;

					if(frame->status)
						UL_LOG("receiveFrame failed, frame status: " << frame->status);
				}
			}
			else
			{
				err = ERR_DEAD_DEV;

				if(received == -1)
					UL_LOG("receiveFrame failed, recv() error: " << strerror(errno));
			}
		}
		else
		{
			if(received == -1)
			{
				UL_LOG("receiveFrame failed, recv() error: " << strerror(errno));

				if(isDevSocketConnected())
				{
					err = ERR_NET_TIMEOUT;
				}
				else
				{
					err = ERR_DEV_NOT_CONNECTED;
				}
			}
		}
	}
	else
		err = ERR_DEV_NOT_CONNECTED;

	return err;
}

void NetDaqDevice::queryCmdVir(unsigned short cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* status) const
{
	UlError err = queryTcpVir(cmd, sendBuf, sendBufLen, NULL, 0, NULL, status, mIoTimeout);

	if(err)
		throw UlException(err);
}

unsigned int NetDaqDevice::queryCmdVir(unsigned short cmd, unsigned char* sendBuf, unsigned short sendBufLen , unsigned char* receiveDataBuf, unsigned short receiveDataBufLen, unsigned char* status) const
{
	unsigned short bytesReceived = 0;
	UlError err = queryTcpVir(cmd, sendBuf, sendBufLen, receiveDataBuf, receiveDataBufLen, &bytesReceived, status, mIoTimeout);

	if(err)
		throw UlException(err);

	return bytesReceived;
}

UlError NetDaqDevice::queryTcpVir(unsigned short cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* receiveBuf, unsigned short receiveBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const
{
	FnLog log("NetDaqDevice::queryTcpVir");

	UlError err = ERR_NO_ERROR;

	UlLock lock(mTcpCmdMutex);

	static unsigned char frameId = 0;

	int retry = 2;

	do
	{
		frameId += 1;

		err = sendFrameVir(cmd, frameId, sendBuf, sendBufLen, timeout);

		if(!err)
		{
			err = receiveFrameVir(cmd, frameId, receiveBuf, receiveBufLen, bytesReceived, status, timeout);

			if(err == ERR_BAD_NET_FRAME)
			{
				clearSocketInputQueue();
				retry--;
			}
		}
	}
	while(err == ERR_BAD_NET_FRAME && retry > 0);

	return err;
}

UlError NetDaqDevice::sendFrameVir(unsigned short cmd, unsigned char frameId, unsigned char* buf, unsigned short bufLen, int timeout) const
{
	FnLog log("NetDaqDevice::sendFrameVir");
	UlError err = ERR_NO_ERROR;

	if(mConnected)
	{
		int frameSize = sizeof(TNetFrameVir) + bufLen; // no need to subtract one (checksum is not member of the NET_FRAME structure)
		int chksumIndex = frameSize - 1;

		if(frameSize > MAX_SEND_FRAME_SIZE)
			return ERR_BAD_BUFFER_SIZE;

		unsigned char frameBuf[MAX_SEND_FRAME_SIZE];
		memset(frameBuf, 0, MAX_SEND_FRAME_SIZE);
		TNetFrameVir* frame = (TNetFrameVir*) frameBuf;

		frame->delimiter = FRAME_START;
		frame->command = cmd;
		frame->frameId = frameId;
		frame->count = Endian::cpu_to_le_ui16(bufLen);
		memcpy(frame->data, buf, bufLen);

		unsigned char chksum = std::accumulate(&frameBuf[0], &frameBuf[chksumIndex], 0);
		chksum = (unsigned char)(0xff) - chksum;

		frameBuf[chksumIndex] = chksum;

		timeval to = NetDiscovery::convertTimeout(timeout);
		if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		int flag = 0;

#ifndef __APPLE__
		// MSG_NOSIGNAL flag is not defined in macOS. For macOS, we set the SO_NOSIGPIPE socket option when socket is created instead.
		flag = MSG_NOSIGNAL;
#endif

		int sent = send(mSockets.tcpCmd, frameBuf, frameSize, flag);

		if(sent != frameSize)
		{
			err = ERR_DEV_NOT_CONNECTED;

			if(sent == -1)
				UL_LOG("sendFrame failed, send() error: " << strerror(errno));
		}
	}
	else
		err = ERR_DEV_NOT_CONNECTED;

	return err;
}

UlError NetDaqDevice::receiveFrameVir(unsigned short cmd, unsigned char frameId, unsigned char* dataBuf, unsigned short dataBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const
{
	FnLog log("NetDaqDevice::receiveFrameVir");
	UlError err = ERR_NO_ERROR;

	if(mConnected)
	{
		unsigned char frameBuf[MAX_SEND_FRAME_SIZE];
		TNetFrameVir* frame = (TNetFrameVir*) frameBuf;
		//unsigned int frameBufLen = sizeof(frameBuf);
		int frameSize = 0;

		if(bytesReceived)
			*bytesReceived = 0;

		if(status)
			*status = 0;

		timeval to = NetDiscovery::convertTimeout(timeout);
		if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) == -1)
			 print_setsockopt_error(errno, __FILE__, __LINE__);

		TNetFrameVir frameHeader;

		// peek at the frame header to obtain the actual frame size
		int received = recv(mSockets.tcpCmd, (char*)&frameHeader, sizeof(TNetFrameVir), MSG_PEEK | MSG_WAITALL);

		if(received == sizeof(frameHeader))
		{
			frameSize = frameHeader.count + sizeof(TNetFrameVir);

			received = recv(mSockets.tcpCmd, frameBuf, frameSize, MSG_WAITALL);

			if(received == frameSize)
			{
				unsigned char chksum = std::accumulate(&frameBuf[0], &frameBuf[received], 0);

				if(chksum != 0xff)
				{
					UL_LOG("Invalid frame checksum!!!!");
					err = ERR_BAD_NET_FRAME;
				}
				else if(frame->command != (cmd | 0x8000))
				{
					UL_LOG("Invalid frame command!!!!");
					err = ERR_BAD_NET_FRAME;
				}
				else if(frame->frameId != frameId)
				{
					UL_LOG("Invalid frame ID!!!!");
					err = ERR_BAD_NET_FRAME;
				}
				else if (frame->count > dataBufLen)
				{
					UL_LOG("Invalid buffer size!!!!");
					err = ERR_BAD_BUFFER_SIZE;
				}
				else
				{
					if(dataBuf)
					{
						memcpy(dataBuf, frame->data, frame->count);

						if(bytesReceived) // if the pointer is not null set its value
							*bytesReceived = frame->count;
					}

					if(status)
						*status = frame->status;

					if(frame->status)
						UL_LOG("receiveFrame failed, frame status: " << frame->status);
				}
			}
			else
			{
				err = ERR_DEAD_DEV;

				if(received == -1)
					UL_LOG("receiveFrame failed, recv() error: " << strerror(errno));
			}
		}
		else
		{
			if(received == -1)
			{
				UL_LOG("receiveFrame failed, recv() error: " << strerror(errno));

				if(isDevSocketConnected())
				{
					err = ERR_NET_TIMEOUT;
				}
				else
				{
					err = ERR_DEV_NOT_CONNECTED;
				}
			}
		}
	}
	else
		err = ERR_DEV_NOT_CONNECTED;

	return err;
}

UlError NetDaqDevice::readScanData(unsigned char* buf, unsigned int bufSize, unsigned int* bytesRead) const
{
	UlError err = ERR_NO_ERROR;
	int received = 0;

	received = recv(mSockets.tcpData, buf, bufSize, 0);

	if(received != -1)
	{
		*bytesRead = received;
	}
	else
	{
		*bytesRead = 0;

		if(errno == EAGAIN)
		{
			err = ERR_NET_TIMEOUT;
		}
		else
		{
			err = ERR_DATA_SOCKET_CONNECTION_FAILED;

			UL_LOG("readScanData failed, recv() error: " << strerror(errno));
		}
	}

	return err;
}

bool NetDaqDevice::isDevSocketConnected() const
{
	FnLog log("NetDaqDevice::isDevSocketConnected !!!!!!");

	bool connected = false;

	UlError err = sendConnectionCode();

	if(err == ERR_NET_DEV_IN_USE)
		connected = true;

	return connected;
}

void NetDaqDevice::clearSocketInputQueue() const
{
	FnLog log("NetDaqDevice::clearSocketInputQueue !!!!!!");

	unsigned char frameBuf[MAX_SEND_FRAME_SIZE];
	unsigned int frameBufLen = sizeof(frameBuf);

	timeval to = NetDiscovery::convertTimeout(100); // 100 ms
	if(setsockopt(mSockets.tcpCmd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) == -1)
		 print_setsockopt_error(errno, __FILE__, __LINE__);

	int received = 0;

	do
	{
		received = recv(mSockets.tcpCmd, frameBuf, frameBufLen, 0);
	}
	while(received > 0);
}

UlError NetDaqDevice::openDataSocket(int timeout /* ms */) const
{
	UlError err = ERR_NO_ERROR;

	bool opened = false;

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
	}

	if(!opened)
		err = ERR_DATA_SOCKET_CONNECTION_FAILED;

	return err;
}

void NetDaqDevice::closeDataSocket() const
{
	if(mSockets.tcpData != -1)
	{
		shutdown(mSockets.tcpData, SHUT_RDWR);
		close(mSockets.tcpData);
		mSockets.tcpData = -1;

		//closeDataSocketFromDevice();
	}
}

bool NetDaqDevice::isDataSocketReady() const
{
	unsigned short status = 0;
	bool ready = false;

	int retryCount = 10;
	int retryNum = 0;

	do
	{
		retryNum++;

		status = readStatus();
	}
	while(!(status & STATUS_DATA_SOCKET_CONNECTED) && retryNum <= retryCount);

	ready = status & STATUS_DATA_SOCKET_CONNECTED ? true : false;

	return ready;
}

void NetDaqDevice::closeDataSocketFromDevice() const
{
	unsigned char cmd = 0x13; // CMD_AINSTOP command
	unsigned char	closeSocket = 1;

	queryCmd(cmd, &closeSocket, sizeof(closeSocket));
}

UlError NetDaqDevice::sendConnectionCode() const
{
	UlError err = ERR_NO_ERROR;

#pragma pack(1)
	struct
	{
		unsigned char cmd;
		unsigned int code;
	}sendBuf;
#pragma pack()

	sendBuf.cmd = CONNECTION_CMD;
	sendBuf.code = Endian::cpu_to_le_ui32(mConnectionCode);

	unsigned char buffer[2];
	unsigned int bufferLen = 2;

	err = queryUdp((char*) &sendBuf, sizeof(sendBuf), (char*) buffer, &bufferLen, mConnectionTimeout);


	if(err == ERR_NO_ERROR)
	{
		switch(buffer[1])
		{
		case 1:
			err = ERR_BAD_CONNECTION_CODE;
			break;
		case 2:
			err = ERR_CONNECTION_CODE_IGNORED;
			break;
		case 3:
			err = ERR_NET_DEV_IN_USE;
			break;
		}
	}

	return err;
}


// this function just sends a dummy byte to force ack to sent
// immediately for the previous transaction
void NetDaqDevice::flushCmdSocket() const
{
	UlLock lock(mTcpCmdMutex);

	unsigned char buf = 0;

	// coverity[check_return]
	send(mSockets.tcpCmd, &buf, sizeof(buf), 0);
}

unsigned short NetDaqDevice::readStatus() const
{
	unsigned short status = 0;

	queryCmd(CMD_STATUS, NULL, 0, (unsigned char*) &status, sizeof(status));

	return status;
}

void NetDaqDevice::flashLed(int flashCount) const
{
	unsigned char blinkCount = flashCount;

	queryCmd(CMD_BLINKLED, &blinkCount, sizeof(blinkCount));
}

int NetDaqDevice::memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_READ, address, buffer, count, false);

	unsigned short bytesToRead = 0;
	unsigned short addr = 0;
	int totalBytesRead = 0;
	int bytesRead = 0;
	int remaining = count;
	unsigned char cmd;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	const int maxTransfer = 512;

	/*if(memRegionType == MR_CAL)
		cmd = CMD_CAL_MEM_R;
	else if(memRegionType == MR_USER)
		cmd = CMD_USER_MEM_R;
	else if(memRegionType == MR_SETTINGS)
		cmd = CMD_SETTINGS_MEM_R;
	else
		throw UlException(ERR_BAD_MEM_REGION);*/

	cmd = getMemCmd(memRegionType, false);

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

		bytesRead  = queryCmd(cmd, (unsigned char*)&params, sizeof(params), readBuff, bytesToRead);

		remaining-= bytesRead;
		totalBytesRead += bytesRead;
		addr += bytesRead;
		readBuff += bytesRead;
	}
	while(remaining > 0);

	return totalBytesRead;
}
int NetDaqDevice::memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const
{
	check_MemRW_Args(memRegionType, MA_WRITE, address, buffer, count, false);

	unsigned short bytesToWrite = 0;
	unsigned short addr = 0;
	int totalBytesWritten = 0;
	int bytesWritten = 0;
	int bytesRemaining = count;
	unsigned char cmd;

	if(buffer == NULL)
		throw UlException(ERR_BAD_BUFFER);

	const int maxTransfer = 512 - 2;

	/*if(memRegionType == MR_USER)
		cmd = CMD_USER_MEM_W;
	else if(memRegionType == MR_SETTINGS)
	{
		if(getMemUnlockCode() != MEM_UNLOCK_CODE)
			throw UlException(ERR_MEM_ACCESS_DENIED);

		cmd = CMD_SETTINGS_MEM_W;
	}
	else
		throw UlException(ERR_BAD_MEM_REGION);*/

	cmd = getMemCmd(memRegionType, true);

	if(memRegionType == MR_SETTINGS && getMemUnlockCode() != MEM_UNLOCK_CODE)
		throw UlException(ERR_MEM_ACCESS_DENIED);

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

	return totalBytesWritten;
}

unsigned char NetDaqDevice::getMemCmd(MemRegion memRegionType, bool writeAccess) const
{
	unsigned char cmd = 0;

	if(writeAccess)
	{
		if(memRegionType == MR_USER)
			cmd = CMD_USER_MEM_W;
		else if(memRegionType == MR_SETTINGS)
			cmd = CMD_SETTINGS_MEM_W;
		else
			throw UlException(ERR_BAD_MEM_REGION);
	}
	else
	{
		if(memRegionType == MR_CAL)
			cmd = CMD_CAL_MEM_R;
		else if(memRegionType == MR_USER)
			cmd = CMD_USER_MEM_R;
		else if(memRegionType == MR_SETTINGS)
			cmd = CMD_SETTINGS_MEM_R;
		else
			throw UlException(ERR_BAD_MEM_REGION);
	}

	return cmd;
}

NetScanTransferIn* NetDaqDevice::scanTranserIn() const
{
	if(mScanTransferIn == NULL)
		UL_LOG("mScanTransferIn is NULL");

	return mScanTransferIn;
}

void NetDaqDevice::connectionCode(long long code)
{
	if( code < 0 || code > 999999999)
		throw UlException(ERR_BAD_CONNECTION_CODE);

	mConnectionCode = code;
}

long long NetDaqDevice::readConnectionCode() const
{
	unsigned int code = 0;

	memRead(MT_EEPROM, MR_SETTINGS, CONNECTION_CODE_ADDR, (unsigned char*) &code, sizeof(code));

	code = Endian::le_ui32_to_cpu(code);

	return code;
}

void NetDaqDevice::writeConnectionCode(long long code) const
{
	if( code < 0 || code > 999999999)
		throw UlException(ERR_BAD_CONNECTION_CODE);

	unsigned int connectionCode = Endian::cpu_to_le_ui32(code);

	memWrite(MT_EEPROM, MR_SETTINGS, CONNECTION_CODE_ADDR, (unsigned char*) &connectionCode, sizeof(connectionCode));
}



//////////////////////          Configuration functions          /////////////////////////////////

long long NetDaqDevice::getCfg_ConnectionCode() const
{
	return readConnectionCode();
}

void NetDaqDevice::setCfg_ConnectionCode(long long code)
{
	writeConnectionCode(code);
}

void NetDaqDevice::getCfg_IpAddress(char* address, unsigned int* maxStrLen) const
{
	std::string addressStr = inet_ntoa(mNetDiscoveryInfo.ipAddr);

	if (addressStr.length() < *maxStrLen)
	{
		memset(address, 0, *maxStrLen);
		strcpy(address, addressStr.c_str());
		*maxStrLen = addressStr.length();
	}
	else
	{
		*maxStrLen = addressStr.length();
		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}

void NetDaqDevice::getCfg_NetIfcName(char* ifcName, unsigned int* maxStrLen) const
{
	if (mNetDiscoveryInfo.ifcName.length() < *maxStrLen)
	{
		memset(ifcName, 0, *maxStrLen);
		strcpy(ifcName, mNetDiscoveryInfo.ifcName.c_str());
		*maxStrLen = mNetDiscoveryInfo.ifcName.length();
	}
	else
	{
		*maxStrLen = mNetDiscoveryInfo.ifcName.length();
		throw UlException(ERR_BAD_BUFFER_SIZE);
	}
}

long long NetDaqDevice::getCfg_MemUnlockCode() const
{
	return getMemUnlockCode();
}

void NetDaqDevice::setCfg_MemUnlockCode(long long code)
{
	setMemUnlockCode(code);
}

void NetDaqDevice::setCfg_Reset()
{
	resetDevice();
}

void NetDaqDevice::print_setsockopt_error(int errnum, const char* file, int line) const
{
	std::cerr << "setsockopt() failed, error: " << strerror(errnum) << " file: " << file << " line: " << line << std::endl;
}
} /* namespace ul */
