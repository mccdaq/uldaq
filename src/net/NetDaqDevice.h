/*
 * NetDaqDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_NETDAQDEVICE_H_
#define NET_NETDAQDEVICE_H_

#include "../uldaq.h"
#include "../DaqDevice.h"
#include "../UlException.h"
#include "NetDiscovery.h"

#include <vector>
#include "../virnet.h"

namespace ul
{

class NetScanTransferIn;

class UL_LOCAL NetDaqDevice: public DaqDevice
{
public:
	NetDaqDevice(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~NetDaqDevice();

	virtual void connect();
	virtual void disconnect();
	virtual void connectionCode(long long code);

	NetScanTransferIn* scanTranserIn() const;

	virtual void flashLed(int flashCount) const;
	virtual unsigned short readStatus() const;

	virtual UlError openDataSocket(int timeout /* ms */) const;
	virtual void closeDataSocket() const;
	virtual bool isDataSocketReady() const;
	virtual void flushCmdSocket() const;

	void queryCmd(unsigned char cmd) const;
	void queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen) const;
	void queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* status) const;
	unsigned int queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen , unsigned char* receiveDataBuf, unsigned short receiveDataBufLen) const;
	unsigned int queryCmd(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen , unsigned char* receiveDataBuf, unsigned short receiveDataBufLen, unsigned char* status) const;

	void queryCmdVir(unsigned short cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* status) const;
	unsigned int queryCmdVir(unsigned short cmd, unsigned char* sendBuf, unsigned short sendBufLen , unsigned char* receiveDataBuf, unsigned short receiveDataBufLen, unsigned char* status) const;


	UlError readScanData(unsigned char* buf, unsigned int bufSize, unsigned int* bytesRead) const;

	virtual int memRead(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;
	virtual int memWrite(MemoryType memType, MemRegion memRegionType, unsigned int address, unsigned char* buffer, unsigned int count) const;

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual long long getCfg_ConnectionCode() const;
	virtual void setCfg_ConnectionCode(long long code);
	virtual long long getCfg_MemUnlockCode() const;
	virtual void setCfg_MemUnlockCode(long long code);
	virtual void setCfg_Reset();

	virtual void getCfg_IpAddress(char* address, unsigned int* maxStrLen) const;
	virtual void getCfg_NetIfcName(char* ifcName, unsigned int* maxStrLen) const;

protected:
	UlError initTcpDataSocket(int timeout /* ms */) const;

	virtual long long readConnectionCode() const;
	virtual void writeConnectionCode(long long code) const;

	virtual unsigned char getMemCmd(MemRegion memRegionType, bool writeAccess) const;

private:
	virtual void establishConnection();
	virtual void initilizeHardware() const {};
	void releaseNetResources();

	UlError initUdpSocket(const NetDiscovery::NetIfcDesc& ifcDesc, const NetDiscovery::NetDiscoveryInfo& discoveryInfo) const;
	UlError initTcpCmdSocket(const NetDiscovery::NetIfcDesc& ifcDesc, const NetDiscovery::NetDiscoveryInfo& discoveryInfo) const;

	void closeSockets();
	void resetDevice();

	UlError queryUdp(char* sendBuf, unsigned int sendBufLen, char* receiveBuf, unsigned int* receiveBufLen, int timeout) const;

	UlError queryTcp(unsigned char cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* receiveBuf, unsigned short receiveBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const;
	UlError sendFrame(unsigned char cmd, unsigned char frameId, unsigned char* buf, unsigned short bufLen, int timeout) const;
	UlError receiveFrame(unsigned char cmd, unsigned char frameId, unsigned char* dataBuf, unsigned short dataBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const;

	UlError queryTcpVir(unsigned short cmd, unsigned char* sendBuf, unsigned short sendBufLen, unsigned char* receiveBuf, unsigned short receiveBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const;
	UlError sendFrameVir(unsigned short cmd, unsigned char frameId, unsigned char* buf, unsigned short bufLen, int timeout) const;
	UlError receiveFrameVir(unsigned short cmd, unsigned char frameId, unsigned char* dataBuf, unsigned short dataBufLen, unsigned short* bytesReceived, unsigned char* status, int timeout) const;


	void clearSocketInputQueue() const;

	std::string getMacAddress();
	bool isValidDevice(std::string macAddr);
	UlError sendConnectionCode() const;
	bool isDevSocketConnected() const;
	virtual void closeDataSocketFromDevice() const;

	void print_setsockopt_error(int errnum, const char* file, int line) const;

private:
	enum { /*DISCOVERY_CMD = 0x44,*/ CONNECTION_CMD = 0x43};
	enum { /*DEFAULT_DISCOVERY_TO = 250,*/ DEFAULT_CONNECTION_TO = 3000, DEFAULT_IO_TO = 3000};
	enum { MAX_SEND_FRAME_SIZE	= 1024, FRAME_START	= 0xDB };
	enum { CMD_BLINKLED = 0x50, CMD_RESET = 0x51, CMD_STATUS = 0x52, CMD_NETCONFIG = 0x54};
	enum { CMD_CAL_MEM_R = 0x40, CMD_USER_MEM_R = 0x42, CMD_USER_MEM_W = 0x43, CMD_SETTINGS_MEM_R = 0x44, CMD_SETTINGS_MEM_W = 0x45};
	enum { MEM_UNLOCK_CODE = 0xAA55 };
	enum { CONNECTION_CODE_ADDR = 0x12 };
	enum { STATUS_DATA_SOCKET_CONNECTED = 1, STATUS_DATAOVERRUN = 4};

private:
	NetDiscovery::NetDiscoveryInfo mNetDiscoveryInfo;
	NetDiscovery::NetIfcDesc mNetIfcDesc;
	mutable pthread_mutex_t mConnectionMutex;
	mutable pthread_mutex_t mUdpMutex;
	mutable pthread_mutex_t mTcpCmdMutex;
	int mConnectionTimeout;
	int mIoTimeout;
	unsigned int mConnectionCode;

	NetScanTransferIn* mScanTransferIn;

	mutable struct NetSockets
	{
		int udp;
		int tcpCmd;
		int tcpData;

		NetSockets(){udp = -1; tcpCmd = -1;  tcpData = -1;}
	}mSockets;


#pragma pack(1)
	typedef struct
	{
		unsigned char delimiter;
		unsigned char command;
		unsigned char frameId;
		unsigned char status;
		unsigned short count;
		unsigned char data[1];

	}NetFrame;
#pragma pack()
};

} /* namespace ul */

#endif /* NET_NETDAQDEVICE_H_ */
