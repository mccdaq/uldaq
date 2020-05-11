/*
 * NetDiscovery.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "NetDiscovery.h"
#include "../DaqDeviceManager.h"

namespace ul
{

pthread_mutex_t NetDiscovery::mDiscoveryMutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<NetDiscovery::NetDiscoveryInfo> NetDiscovery::mAutoDiscoveryList;
std::vector<NetDiscovery::NetDiscoveryInfo> NetDiscovery::mManualDiscoveryList;

std::vector<DaqDeviceDescriptor> NetDiscovery::findDaqDevices()
{
	UlLock lock(mDiscoveryMutex);

	mAutoDiscoveryList.clear();

	std::vector<DaqDeviceDescriptor> descriptorList;

	FnLog log("NetDiscovery::findDaqDevices");

	try
	{
		std::vector<NetDiscoveryInfo> netDiscoveryInfo = discoverDevices();

		for(unsigned int i = 0; i < netDiscoveryInfo.size(); i++)
		{
			if(DaqDeviceManager::isDaqDeviceSupported(netDiscoveryInfo[i].productId, 0))
			{
				DaqDeviceDescriptor daqDevDescriptor;
				memset(&daqDevDescriptor, 0,sizeof(DaqDeviceDescriptor));

				daqDevDescriptor.productId = netDiscoveryInfo[i].productId;
				daqDevDescriptor.devInterface = ETHERNET_IFC;
				std::string productName = DaqDeviceManager::getDeviceName(daqDevDescriptor.productId, 0);

				std::string devString = netDiscoveryInfo[i].netBiosName;

				if(netDiscoveryInfo[i].wifi)
				{
					devString.append("-WIFI");
				}

				strncpy(daqDevDescriptor.productName, productName.c_str(), sizeof(daqDevDescriptor.productName) - 1);
				strncpy(daqDevDescriptor.devString, devString.c_str(), sizeof(daqDevDescriptor.devString) - 1);
				strncpy(daqDevDescriptor.uniqueId, netDiscoveryInfo[i].macAddr.c_str(), sizeof(daqDevDescriptor.uniqueId) - 1);

				UL_LOG("-----------------------");
				UL_LOG("Product ID : 0x" << std::hex << daqDevDescriptor.productId << std::dec);
				UL_LOG("Product Name: "<< daqDevDescriptor.productName);
				UL_LOG("NetBIOS Name: "<< daqDevDescriptor.devString);
				UL_LOG("MAC Address : "<< daqDevDescriptor.uniqueId);
				UL_LOG("IP Address : " << inet_ntoa(netDiscoveryInfo[i].ipAddr));
				UL_LOG("-----------------------");

				descriptorList.push_back(daqDevDescriptor);

				removeFromManualDiscoveryList(netDiscoveryInfo[i].macAddr);
				mAutoDiscoveryList.push_back(netDiscoveryInfo[i]);
			}
		}
	}
	catch(...)
	{
		std::cout << "NetDiscovery::findDaqDevices(), unhandled exception " << std::endl;
	}

	return descriptorList;
}

DaqDeviceDescriptor NetDiscovery::findDaqDevice(std::string host, unsigned short port, std::string ifcName, int timeout)
{
	UlLock lock(mDiscoveryMutex);

	DaqDeviceDescriptor daqDevDescriptor = {0};

	FnLog log("NetDiscovery::findDaqDevice");

	int devIndex = -1;

	std::vector<NetDiscoveryInfo> netDiscoveryInfo = discoverDevices(host, port, ifcName, timeout);

	for(unsigned int i = 0; i < netDiscoveryInfo.size(); i++)
	{
		if(DaqDeviceManager::isDaqDeviceSupported(netDiscoveryInfo[i].productId, 0))
		{
			if(devIndex == -1)
				devIndex = i;

			// if interface is specified then return the first match, otherwise, look for an interface which is in
			// the same subnet as the daq device. if no interface was detected in the same subnet then return the first device
			if(!ifcName.empty())
				break;
			else if(hostAndDevInSameSubnet(netDiscoveryInfo[i]))
			{
				devIndex = i;
				break;
			}
		}
	}

	if(devIndex != -1)
	{
		memset(&daqDevDescriptor, 0,sizeof(DaqDeviceDescriptor));

		daqDevDescriptor.productId = netDiscoveryInfo[devIndex].productId;
		daqDevDescriptor.devInterface = ETHERNET_IFC;
		std::string productName = DaqDeviceManager::getDeviceName(daqDevDescriptor.productId, 0);

		strncpy(daqDevDescriptor.productName, productName.c_str(), sizeof(daqDevDescriptor.productName) - 1);
		strncpy(daqDevDescriptor.devString, netDiscoveryInfo[devIndex].netBiosName.c_str(), sizeof(daqDevDescriptor.devString) - 1);
		strncpy(daqDevDescriptor.uniqueId, netDiscoveryInfo[devIndex].macAddr.c_str(), sizeof(daqDevDescriptor.uniqueId) - 1);

		removeFromAutoDiscoveryList(netDiscoveryInfo[devIndex].macAddr);
		removeFromManualDiscoveryList(netDiscoveryInfo[devIndex].macAddr);

		mManualDiscoveryList.push_back(netDiscoveryInfo[devIndex]);

		UL_LOG("-----------------------");
		UL_LOG("Product ID : 0x" << std::hex << daqDevDescriptor.productId << std::dec);
		UL_LOG("Product Name: "<< daqDevDescriptor.productName);
		UL_LOG("NetBIOS Name: "<< daqDevDescriptor.devString);
		UL_LOG("MAC Address : "<< daqDevDescriptor.uniqueId);
		UL_LOG("IP Address : " << inet_ntoa(netDiscoveryInfo[devIndex].ipAddr));
		UL_LOG("-----------------------");
	}
	else
	{
		throw UlException(ERR_DEV_NOT_FOUND);
	}

	return daqDevDescriptor;
}

std::vector<NetDiscovery::NetDiscoveryInfo> NetDiscovery::discoverDevices(std::string host, unsigned short port, std::string ifcName, int timeout)
{
	int sockfd;
	std::vector<NetDiscoveryInfo> discoveryInfo;
	timeval recvTimeout = convertTimeout(timeout);
	int broadCast = 1;
	unsigned int targetAddr = htonl(INADDR_BROADCAST);

	if(!host.empty())  // if daq device host address or name specified then the broadcast mode must be disabled
	{
		broadCast = 0;
		sockaddr_in addr = getHostAddress(host);
		targetAddr = addr.sin_addr.s_addr;
	}

	//find network interfaces
	std::vector<NetIfcDesc> netIfcDescs = getNetIfcDescs(ifcName);

	for(unsigned int i = 0; i < netIfcDescs.size(); i++)
	{
		sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if(sockfd != -1)
		{
			bool setOptErr = false;

			if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadCast, sizeof(broadCast)) == -1)
				setOptErr = true;

			if(setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*) &recvTimeout, sizeof(recvTimeout)) == -1)
				setOptErr = true;

			if(setOptErr)
				std::cerr << "setsockopt() failed, error: " << strerror(errno) << " file: " << __FILE__ << " line: " << __LINE__ << std::endl;

			if(bind(sockfd, (sockaddr*) &netIfcDescs[i].addr, sizeof(sockaddr)) != -1)
			{
				bool success = sendDiscovery(sockfd, targetAddr, port);

				if(success)
				{
					NetDiscoveryInfo netDiscoveryInfo;

					while(detectNetDevice(sockfd, netDiscoveryInfo))
					{
						netDiscoveryInfo.ifcName = netIfcDescs[i].name;
						netDiscoveryInfo.discoveryPort = port;
						netDiscoveryInfo.valid = true;

						discoveryInfo.push_back(netDiscoveryInfo);

						/*std::cout << "Interface name: " << netDiscoveryInfo.ifcName << std::endl;
						std::cout << "Interface IP: " << inet_ntoa(netIfcDescs[i].addr.sin_addr) << std::endl;
						std::cout << "===============================" << std::endl;*/

						netDiscoveryInfo.clear();

						 // if daq device host address or name specified then desired device is detected, exit the loop
						if(!host.empty())
							break;
					}

				}
			}
			else
			{
				UL_LOG("bind() error: " << strerror(errno));
			}

			close(sockfd);
		}
	}

	return discoveryInfo;
}

bool NetDiscovery::sendDiscovery(int sockfd, unsigned int addr, unsigned short port)
{
	sockaddr_in	targetAddr;
	memset(&targetAddr, 0, sizeof(targetAddr));
	bool success = false;

	char cmd = DISCOVERY_CMD;
	int sent = 0;

	targetAddr.sin_family = AF_INET;
	targetAddr.sin_port = htons(port);
	targetAddr.sin_addr.s_addr =  addr; //htonl(addr);

	sent = sendto(sockfd, (const char*) &cmd, sizeof(cmd), 0, (const sockaddr*) &targetAddr, sizeof(targetAddr));

	if(sent > 0)
		success = true;
	else
		UL_LOG("broadcast failed, sendto() error: " << strerror(errno));

	return success;
}

bool NetDiscovery::detectNetDevice(int sockfd, NetDiscoveryInfo& netDiscoveryInfo)
{
	bool found = false;
	sockaddr_in	daqDevAddr;
	int received = 0;
	NetDevDesc netDevDesc;
	char buffer[UDP_MSG_MAX_LEN];
	socklen_t addrLen =  sizeof(sockaddr_in);

	do
	{
		memset(buffer, 0, sizeof(buffer));
		received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (sockaddr *) &daqDevAddr,  &addrLen);

		if(received > 0)
		{
			if((received == (sizeof(netDevDesc) + 1)) && buffer[0] == DISCOVERY_CMD)
			{
				memcpy(&netDevDesc, &buffer[1], sizeof(netDevDesc));

				char macAddr[18];
				snprintf(macAddr,sizeof(macAddr), "%02X:%02X:%02X:%02X:%02X:%02X", netDevDesc.mac[0], netDevDesc.mac[1], netDevDesc.mac[2], netDevDesc.mac[3], netDevDesc.mac[4], netDevDesc.mac[5]);

				netDiscoveryInfo.macAddr = macAddr;
				netDiscoveryInfo.ipAddr = daqDevAddr.sin_addr;
				netDiscoveryInfo.productId = Endian::le_ui16_to_cpu(netDevDesc.pid);
				netDiscoveryInfo.fwVer = Endian::le_ui16_to_cpu(netDevDesc.fwVer);

				netDiscoveryInfo.netBiosName.append(netDevDesc.netbios, sizeof(netDevDesc.netbios) - 1);

				// trim the white spaces
				std::size_t pos = netDiscoveryInfo.netBiosName.find_last_not_of(" ");
				if (pos != std::string::npos)
					netDiscoveryInfo.netBiosName.erase(pos + 1);

				netDiscoveryInfo.wifi = netDevDesc.wifi ? true : false; // only virnet


				/*std::cout << "===============================" << std::endl;
				std::cout << "NetBIOS name: " << netDiscoveryInfo.netBiosName << std::endl;
				std::cout << "MAC address: " << netDiscoveryInfo.macAddr << std::endl;
				std::cout << "DAQ Device IP: " << inet_ntoa(netDiscoveryInfo.ipAddr) << std::endl;*/

				found = true;
			}
		}
	}
	while(received > 0 && !found);

	return found;
}


NetDiscovery::NetDiscoveryInfo NetDiscovery::getDiscoveryInfo(std::string macAddr)
{
	NetDiscoveryInfo discoveryInfo;
	discoveryInfo.clear();

	for(unsigned int i = 0; i < mAutoDiscoveryList.size(); i++)
	{
		if(mAutoDiscoveryList[i].macAddr == macAddr)
		{
			discoveryInfo = mAutoDiscoveryList[i];
			break;
		}
	}

	if(!discoveryInfo.valid)
	{
		for(unsigned int i = 0; i < mManualDiscoveryList.size(); i++)
		{
			if(mManualDiscoveryList[i].macAddr == macAddr)
			{
				discoveryInfo = mManualDiscoveryList[i];
				break;
			}
		}
	}

	return discoveryInfo;
}

std::vector<NetDiscovery::NetIfcDesc> NetDiscovery::getNetIfcDescs(std::string ifcName)
{
	std::vector<NetIfcDesc> netIfcDescs;
	ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) != -1)
	{
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr == NULL)
				continue;

			if ((ifa->ifa_addr->sa_family == AF_INET) && !(ifa->ifa_flags & IFF_LOOPBACK))
			{
				NetIfcDesc desc;

				desc.name = ifa->ifa_name;
				desc.addr = *((sockaddr_in*) ifa->ifa_addr);
				desc.netmask = *((sockaddr_in*) ifa->ifa_netmask);
				//desc.broadaddr = *((sockaddr_in*) ifa->ifa_ifu.ifu_broadaddr);

				if(ifcName.empty() || (!ifcName.empty() && ifcName == desc.name))
					netIfcDescs.push_back(desc);
			}
		}

		freeifaddrs(ifaddr);
	}
	else
	{
		UL_LOG("unable to detect network interfaces, getifaddrs() error: " << strerror(errno));
	}

	if(!ifcName.empty() && netIfcDescs.empty())
		throw UlException(ERR_BAD_NET_IFC);

	return netIfcDescs;
}

void NetDiscovery::removeFromAutoDiscoveryList(const std::string& macAddr)
{
	for (std::vector<NetDiscovery::NetDiscoveryInfo>::iterator it = mAutoDiscoveryList.begin(); it != mAutoDiscoveryList.end(); it++)
	{
		if(it->macAddr == macAddr)
		{
			mAutoDiscoveryList.erase(it);
			break;
		}
	}
}

void NetDiscovery::removeFromManualDiscoveryList(const std::string& macAddr)
{
	for (std::vector<NetDiscovery::NetDiscoveryInfo>::iterator it = mManualDiscoveryList.begin(); it != mManualDiscoveryList.end(); it++)
	{
		if(it->macAddr == macAddr)
		{
			mManualDiscoveryList.erase(it);
			break;
		}
	}
}

sockaddr_in NetDiscovery::getHostAddress(std::string host)
{
	bool found = false;

	addrinfo hints;
	struct addrinfo* addr;
	sockaddr_in hostAddr;

	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;      	/* Allow IPv4 */
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_UDP;

	int err = getaddrinfo(host.c_str(), NULL, &hints, &addr);
	if(!err)
	{
		hostAddr = *((sockaddr_in*)addr->ai_addr);

		found = true;

		freeaddrinfo(addr);
	}
	else
	{
		UL_LOG("unable to get host address, getaddrinfo() error: " << gai_strerror(err));
	}

	if(!found)
		throw UlException(ERR_BAD_NET_HOST);

	return hostAddr;
}

bool NetDiscovery::hostAndDevInSameSubnet(NetDiscoveryInfo discoveryInfo )
{
	bool sameSubnet = false;

	std::vector<NetIfcDesc> IfcDescs = getNetIfcDescs(discoveryInfo.ifcName);

	if(IfcDescs.size() > 0)
	{
		in_addr ifc, dev;
		ifc.s_addr= IfcDescs[0].netmask.sin_addr.s_addr & IfcDescs[0].addr.sin_addr.s_addr;
		dev.s_addr= IfcDescs[0].netmask.sin_addr.s_addr & discoveryInfo.ipAddr.s_addr;

		if(ifc.s_addr == dev.s_addr)
			sameSubnet = true;
	}

	return sameSubnet;
}

bool NetDiscovery::isNetIfcAvaiable(std::string ifcName)
{
	bool available = false;

	if(getNetIfcDescs(ifcName).size())
		available = true;

	return available;
}

timeval NetDiscovery::convertTimeout(int timeout /* in ms*/)
{
	timeval to;

	if(timeout < 0)
		timeout = 0;

	to.tv_sec = timeout / 1000;
	to.tv_usec = (timeout % 1000) * 1000;

	return to;
}
} /* namespace ul */
