/*
 * NetDiscovery.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef NET_NETDISCOVERY_H_
#define NET_NETDISCOVERY_H_

#include "../ul_internal.h"
#include "../UlException.h"
#include "../utility/UlLock.h"
#include "../utility/FnLog.h"
#include "../utility/Endian.h"
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

namespace ul
{

class UL_LOCAL NetDiscovery
{
public:
	NetDiscovery(){};
	virtual ~NetDiscovery(){};

	typedef struct
	{
		std::string name;			// Network interface name
		sockaddr_in addr;			// Network address of this interface.
		sockaddr_in netmask; 		// Netmask of this interface.
	} NetIfcDesc;

	typedef struct
	{
		std::string macAddr;
		int productId;
		unsigned short fwVer;
		std::string netBiosName;
		in_addr ipAddr;
		std::string ifcName;  // client ifc name
		bool wifi; // this is only valid for virnet devices

		int discoveryPort;
		bool valid;

		void clear()
		{
			macAddr.clear();
			productId = 0;
			fwVer = 0;
			netBiosName.clear();
			ipAddr.s_addr = 0;
			ifcName.clear();
			wifi = false;
			discoveryPort = 0;
			valid = false;
		};
	}NetDiscoveryInfo;

public:
	static std::vector<DaqDeviceDescriptor> findDaqDevices();
	static DaqDeviceDescriptor findDaqDevice(std::string host, unsigned short port, std::string ifcName, int timeout);

	static NetDiscoveryInfo getDiscoveryInfo(std::string mac);
	static bool isNetIfcAvaiable(std::string ifcName);
	static std::vector<NetIfcDesc> getNetIfcDescs(std::string ifcName = "");

	static timeval convertTimeout(int timeout /* in ms*/);

private:

	static std::vector<NetDiscoveryInfo> discoverDevices(std::string host = "", unsigned short port = DISCOVERY_PORT, std::string ifcName = "", int timeout = DISCOVERY_TO);
	static void removeFromAutoDiscoveryList(const std::string& macAddr);
	static void removeFromManualDiscoveryList(const std::string& macAddr);
	static bool hostAndDevInSameSubnet(NetDiscoveryInfo discoveryInfo);


	static bool sendDiscovery(int sockfd, unsigned int addr, unsigned short port);
	static bool detectNetDevice(int sockfd, NetDiscoveryInfo& netDiscoveryInfo);
	static sockaddr_in getHostAddress(std::string host);

public:
	enum { DISCOVERY_TO = 250, DISCOVERY_PORT = 54211, DISCOVERY_CMD = 0x44, UDP_MSG_MAX_LEN =  512};

#pragma pack(1)

		typedef struct
		{
			unsigned char	mac[6];
			unsigned short	pid;
			unsigned short  fwVer;
			char			netbios[16];
			unsigned short	cmdPort;
			unsigned short	dataInPort;
			unsigned short	reserved;
			unsigned short	status;
			unsigned char	hostIpAddr[4]; //38
			unsigned short  blFwVer;
			unsigned char	wifi; // added for virnet devices
			char			reserved1[22];

		} NetDevDesc;
	#pragma pack()

private:
	static pthread_mutex_t mDiscoveryMutex;
	static std::vector<NetDiscoveryInfo> mAutoDiscoveryList;
	static std::vector<NetDiscoveryInfo> mManualDiscoveryList;

};

} /* namespace ul */

#endif /* NET_NETDISCOVERY_H_ */
