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
#include "ns3/csma-helper.h"
#include "reaper.h"
#include "ns3/radvd-helper.h"

using namespace ns3;

namespace Sim {
class Api;
class MyApp;


typedef ns3::PointToPointHelper SpineLink;
typedef ns3::PointToPointHelper RackLink;
typedef ns3::PointToPointHelper ReaperLink;

class Node {
	friend class Api;
public:

	Node(int nRows, int nCols, int nWaveNet);
	
	void 					ConnectToSwitch(Ptr<ns3::Node> sw_node, int inst);
	void 					AssignIpv4Addresses (ns3::Ipv4AddressHelper Ips);
	void 					AssignIpv6Addresses (ns3::Ipv6AddressHelper Ips);
	ns3::NodeContainer 		GetReapersNodeContainer() { return m_reaper_nodes; }
	Reaper& 				GetReaper(int i);
	std::vector<Reaper>& 	GetReapers();
	ApplicationContainer 	AddOnOffApplication(Reaper& reap1, Reaper& reap2, ns3::OnOffHelper onoff, int port);
	ApplicationContainer 	PacketSinkApplication(int port);
	void 					PrintIps(std::ostream& o=std::cout, int indent=0);
	Ptr<MyApp> 				GetApp(int i);
	int 					GetNodeIndex();
	void 					InstallApiApps(int node_num, Api* api);
	
	static ns3::InternetStackHelper internet;
private:
	void ConnectReapers();
	std::string 			  m_name;
	int 					  m_cols;
	int 					  m_rows;
	
	int 					  m_nReapers;
	int 					  m_nWaveNet;
	ns3::NodeContainer  	  m_reaper_nodes;
	std::vector<Reaper> 	  m_reapers;
	ns3::ApplicationContainer m_reaper_apps;
	ns3::RadvdHelper		  m_radvd_helper;
	ns3::NodeContainer		  m_switch_nodes;
	
	const int Mtu = 16000;
	
	int 					  m_node_idx;
	static int 				  m_node_count;
};


}


#endif
