#ifndef REAPER_H
#define REAPER_H

#include <iostream>
#include <sstream>

#include "ns3/core-module.h"
//#include "ns3/simulator-module.h"
//#include "ns3/node-module.h"
#include "ns3/network-module.h"
//#include "ns3/helper-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/on-off-helper.h"
//#include "ns3/random-variable.h"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink-helper.h"

using namespace ns3;

namespace Sim {
class Api;
class MyApp;


class Reaper {
friend class Node;
public:
	Reaper(Ptr<ns3::Node> node) : m_node(node) { }
	void 			 			AssignIpv4Addresses(ns3::Ipv4AddressHelper& Ips);
	void 			 			AssignIpv6Addresses(ns3::Ipv6AddressHelper& Ips);
	Ptr<ns3::Node> 	 			GetNode() { return m_node; }
	void 			 			PrintIps(std::ostream& o=std::cout, int indent=0);
	
	ns3::Ipv4Address 			GetAddress(int i);
	ns3::Ipv6Address 			GetAddressV6(int i);
	ns3::Ipv4Address 			GetWNAddress(int i);
	ns3::Ipv6Address 			GetWNAddressV6(int i);
	
	uint32_t 		 			GetChannelId(int i);
	uint32_t 		 			GetWNChannelId(int i);
	
	int 			 			GetAddressNum();
	int 			 			GetWNAddressNum();
	int 			 			GetWNAddressNumV6();
	
	ns3::Ipv4InterfaceContainer GetInterfaces();
	ns3::Ipv4InterfaceContainer GetWNInterfaces();
	ns3::Ipv6InterfaceContainer GetInterfacesV6();
	ns3::Ipv6InterfaceContainer GetWNInterfacesV6();
	
	void 						AddInterface(ns3::Ipv4InterfaceContainer::Iterator it);
	void 						AddInterfaceV6(ns3::Ipv6InterfaceContainer::Iterator it);
	void 						AddWNInterface(ns3::Ipv4InterfaceContainer::Iterator it);
	void 						AddWNInterfaceV6(ns3::Ipv6InterfaceContainer::Iterator it);
	
	void 						AddDevice(Ptr<ns3::NetDevice> dev, bool isLeft);
	void 						AddSwitchDevice(Ptr<ns3::NetDevice> dev);
	void 						AddWNDevice(Ptr<ns3::NetDevice> dev);
	
	void						MapChirplets();
	int							GetChirplet(uint32_t ip);
	
private:
	Ptr<ns3::Node> 				m_node;
	
	ns3::NetDeviceContainer 	m_devices;
	ns3::NetDeviceContainer 	m_left_devices;
	ns3::NetDeviceContainer 	m_right_devices;
	ns3::NetDeviceContainer 	m_switch_devices;
	ns3::NetDeviceContainer 	m_wn_devices;
	
	ns3::Ipv4InterfaceContainer m_interfaces;
	ns3::Ipv6InterfaceContainer m_interfaces_v6;
	ns3::Ipv4InterfaceContainer m_wn_interfaces;
	ns3::Ipv6InterfaceContainer m_wn_interfaces_v6;
	
	std::map<uint32_t,int>		m_map_ipv4_to_chirplet;
};

}


#endif
