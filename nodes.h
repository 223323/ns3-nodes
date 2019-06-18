#ifndef NODES_H
#define NODES_H


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
	Ptr<ns3::Node> 	 			GetNode() { return m_node; }
	void 			 			PrintIps(std::ostream& o=std::cout, int indent=0);
	ns3::Ipv4Address 			GetAddress(int i);
	uint32_t 		 			GetChannelId(int i);
	int 			 			GetAddressNum();
	ns3::Ipv4InterfaceContainer GetInterfaces();
	ns3::NetDeviceContainer 	GetDevices();
	void 						AddDevice(Ptr<ns3::NetDevice> dev, bool isLeft);
	void 						AddSwitchDevice(Ptr<ns3::NetDevice> dev, bool isLeft);
	void						MapChirplets();
	int							GetChirplet(uint32_t ip);
	
private:
	Ptr<ns3::Node> 						 m_node;
	ns3::NetDeviceContainer 			 m_devices;
	ns3::NetDeviceContainer 			 m_left_devices;
	ns3::NetDeviceContainer 			 m_right_devices;
	ns3::NetDeviceContainer 			 m_switch_devices;
	ns3::Ipv4InterfaceContainer 		 m_interfaces;
	std::map<uint32_t,int>				 m_map_ipv4_to_chirplet;
};


class Node {
public:

	Node(std::string name, int nRows, int nCols, int nWaveNet, ns3::PointToPointHelper link);
	
	void ConnectToSwitch(Ptr<ns3::Node> sw_node, ns3::PointToPointHelper switch_link, ns3::Ipv4AddressHelper& Ips);
	void AssignIpv4Addresses (ns3::Ipv4AddressHelper& Ips);
	ns3::NodeContainer GetNodes() { return m_reaper_nodes; }
	Reaper& GetReaper(int i);
	std::vector<Reaper>& GetReapers();
	ApplicationContainer AddOnOffApplication(Reaper& reap1, Reaper& reap2, OnOffHelper onoff, int port);
	ApplicationContainer PacketSinkApplication(int port);
	void PrintIps(std::ostream& o=std::cout, int indent=0);
	Ptr<MyApp> GetApp(int i);
	
	void InstallApiApps(int node_num, Api* api);
	
	static ns3::InternetStackHelper internet;
private:
	void ConnectReapers(ns3::PointToPointHelper link);
	std::string 			  m_name;
	int 					  m_cols;
	int 					  m_rows;
	int 					  m_nReapers;
	int 					  m_nWaveNet;
	bool 					  m_reapers_connected;
	std::vector<ns3::NetDeviceContainer> m_dev_pairs;
	ns3::NodeContainer  	  m_reaper_nodes;
	std::vector<Reaper> 	  m_reapers;
	ns3::ApplicationContainer m_reaper_apps;

	const int Mtu = 16000;
};


}


#endif
