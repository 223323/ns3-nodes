#include "node.h"
#include "app.h"
#include "api.h"
#include "ns3/csma-module.h"
#include "ns3/bridge-helper.h"
#include "ns3/csma-helper.h"

#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/radvd.h"
#include "ns3/radvd-interface.h"
#include "ns3/radvd-prefix.h"
#include "ns3/radvd-helper.h"
#include "ns3/ipv6-static-routing-helper.h"

NS_LOG_COMPONENT_DEFINE ("Nodes");

namespace Sim {

Node::Node(int nRows, int nCols, int nWaveNet)
	: m_cols(nCols),
	  m_rows(nRows),
	  m_nReapers(nCols*nRows),
	  m_nWaveNet(nWaveNet) 
{
	m_node_idx = m_node_count;
	m_name = std::string("node") + std::to_string(m_node_idx);
	m_reaper_nodes.Create(m_nReapers);
	for(int i=0; i < m_nReapers; i++) {
		m_reapers.emplace_back(m_reaper_nodes.Get(i));
		Names::Add(m_name + "-" + std::to_string(i), m_reaper_nodes.Get(i));
	}
	Api::internet.Install(m_reaper_nodes);
	
	
	m_node_count++;
}



void
Node::AssignIpv4Addresses (ns3::Ipv4AddressHelper Ips) {
	for (auto &r : m_reapers) {
		r.AssignIpv4Addresses(Ips);
	}
}

void
Node::AssignIpv6Addresses (ns3::Ipv6AddressHelper Ips) {
	for (auto &r : m_reapers) {
		r.AssignIpv6Addresses(Ips);
	}
}

int Node::m_node_count = 0;

void
Node::ConnectReapers() {
	bool loop_connect = false;
	
	ns3::PointToPointHelper link;
	link.SetDeviceAttribute  ("DataRate", StringValue (Api::m_reaper_link.speed));
	link.SetChannelAttribute ("Delay", StringValue (Api::m_reaper_link.delay));
	
	std::cout << "Node " << m_node_idx << "\n";
	for(int x=0; x < m_cols-1 + (int)Api::m_reapers_circular; x++) {
		for(int y=0; y < m_rows; y++) {
			
			int n1 = m_cols * y + x;
			auto node1 = m_reapers[n1].GetNode();
			
			for(int y1=0; y1 < m_rows + loop_connect ? 1 : 0; y1++) {
				int n2 = m_cols * y1 + (x+1) % m_cols;
				auto node2 = m_reapers[n2].GetNode();
				
				std::cout << "\tLink: " << n1 << " : " << n2 << "\n";
				ns3::NetDeviceContainer devs = link.Install(node1, node2);
				
				m_reapers[n1].AddDevice( devs.Get(0), true );
				m_reapers[n2].AddDevice( devs.Get(1), false );
				
				devs.Get(0)->SetMtu(Mtu);
				devs.Get(1)->SetMtu(Mtu);
			}
		}
	}
	
	ns3::Ipv4AddressHelper ipv4;
	char base_ip[21];
	sprintf(base_ip, "10.%d.0.0", m_node_idx);
	ipv4.SetBase (base_ip, "255.255.255.0");
	AssignIpv4Addresses(ipv4);
}

Reaper&
Node::GetReaper(int i) {
	return m_reapers[i];
}

std::vector<Reaper>&
Node::GetReapers() {
	return m_reapers;
}

void
Node::PrintIps(std::ostream& o, int indent) {
	int i=0;
	for(auto &r : m_reapers) {
		o.write("\t\t", indent);
		o << "Reaper " << i << "\n";
		r.PrintIps(o, indent+1);
		i++;
	}
}

ns3::ApplicationContainer
Node::AddOnOffApplication(Reaper& reap1, Reaper& reap2, OnOffHelper onoff, int port) {
	AddressValue remoteAddress (InetSocketAddress (reap2.GetAddress(0), port));
	onoff.SetAttribute("Remote", remoteAddress);
	ApplicationContainer apps;
	apps.Add( onoff.Install(reap1.GetNode()) );
	return apps;
}

ns3::ApplicationContainer
Node::PacketSinkApplication(int port) {
	ns3::PacketSinkHelper serverHelper("ns3::UdpSocketFactory", Address());
	ApplicationContainer serverApps;
	
	for(auto &r : m_reapers) {
		AddressValue local(InetSocketAddress(Ipv4Address::GetAny(), port));
		serverHelper.SetAttribute("Local", local);
		serverApps.Add(serverHelper.Install(r.GetNode()));
	}
	return serverApps;
}

int
Node::GetNodeIndex() {
	return m_node_idx;
}

void
Node::ConnectToSwitch(Ptr<ns3::Node> sw_node, int inst) {
	int r_idx=0;
	m_switch_nodes.Add(sw_node);
	ns3::CsmaHelper csma;
	csma.SetChannelAttribute  ("DataRate", StringValue (Api::m_rack_link.speed));
	csma.SetChannelAttribute ("Delay", StringValue (Api::m_rack_link.delay));
	
	ns3::PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate",  StringValue (Api::m_rack_link.speed));
	p2p.SetChannelAttribute("Delay",  StringValue (Api::m_rack_link.delay));
	
	char base_ip[31];
	Ipv4AddressHelper Ips;
	Ipv6AddressHelper Ips6;
	// Ips6.SetBase(
	sprintf(base_ip, "2001:1:%02x%02x::", GetNodeIndex()+1, inst);
	Ips6.SetBase (Ipv6Address (base_ip), Ipv6Prefix (64));
	std::cout << "connect to swtich\n";
	const int Mtu = 15000;
	
	ns3::NetDeviceContainer switch_devices;
	
	for(int i=0; i < (Api::m_use_p2p ? m_nWaveNet : 1); i++) {
		ns3::NodeContainer switch_nodes;
		if(!Api::m_use_p2p) {
			switch_nodes.Add(sw_node);
		}
		for(auto &r : m_reapers) {
			switch_nodes.Add(r.GetNode());
			r_idx++;
		}
		
		NetDeviceContainer devs;
		Ipv4InterfaceContainer wn_interfaces;
		Ipv6InterfaceContainer wn_interfaces_v6;
		if(Api::m_use_p2p) {
			for(uint j=0; j < switch_nodes.GetN(); j++) {
				sprintf(base_ip, "%d.%d.%d.0", GetNodeIndex()+1, inst, (int)m_nWaveNet*j + i);
				Ips.SetBase (base_ip, "255.255.255.0");
				std::cout << "adding ip: " << base_ip << "\n";
				NetDeviceContainer d = p2p.Install( switch_nodes.Get(j), sw_node );
				d.Get(0)->SetMtu(Mtu);
				d.Get(1)->SetMtu(Mtu);
				Ipv4InterfaceContainer iface = Ips.Assign(d);
				wn_interfaces.Add( iface.Get(0) );
				devs.Add(d.Get(0));
				if(Api::m_use_ipv6) {
					Ipv6InterfaceContainer iface_v6 = Ips6.Assign(d);
					wn_interfaces_v6.Add(iface_v6);
					Ips6.NewNetwork();
				}
				
				std::cout << wn_interfaces.GetAddress(j, 0) << "\n";
				// Ips.NewNetwork();
			}
		} else {
			devs  = csma.Install(switch_nodes);
			
			std::cout << "Devs " << devs.GetN() << "\n";
			wn_interfaces = Ips.Assign(devs);
			if(Api::m_use_ipv6) {
				wn_interfaces_v6 = Ips6.Assign(devs);
				for(uint j=0; j < wn_interfaces_v6.GetN(); j++) {
					wn_interfaces_v6.SetForwarding(j,true);
					wn_interfaces_v6.SetDefaultRouteInAllNodes(j);
					std::cout << wn_interfaces_v6.GetAddress(j, 1) << "\n";
				}
			}
			for(uint j=0; j < wn_interfaces.GetN(); j++) {
				devs.Get(j)->SetMtu(Mtu);
				// std::cout << wn_interfaces.GetAddress(j, 0) << "\n";
			}
			
		}
		
		if(Api::m_use_ipv6) {
			
			m_radvd_helper.AddAnnouncedPrefix(wn_interfaces_v6.GetInterfaceIndex (0), Ipv6Address(base_ip), 64);
			
			class IpAddressHelper {
			public:
			  /**
			   * \brief Print the node's IP addresses.
			   * \param n the node
			   */
			  inline void PrintIpAddresses (Ptr<ns3::Node>& n)
			  {
				Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();
				uint32_t nInterfaces = ipv6->GetNInterfaces();

				std::cout << "Node: " << ipv6->GetObject<ns3::Node> ()->GetId ()
					<< " Time: " << Simulator::Now ().GetSeconds () << "s "
					<< "IPv6 addresses" << std::endl;
				std::cout << "(Interface index, Address index)\t" << "IPv6 Address" << std::endl;

				for (uint32_t i = 0; i < nInterfaces; i++)
				  {
					for (uint32_t j = 0; j < ipv6->GetNAddresses(i); j++)
					  {
						std::cout << "(" << int(i) << "," << int(j) << ")\t" << ipv6->GetAddress(i,j) << std::endl;
					  }
				  }
				std::cout << std::endl;
			  }
			};

			IpAddressHelper ipAddressHelper;
			
			Simulator::Schedule (Seconds (2.0), &IpAddressHelper::PrintIpAddresses, &ipAddressHelper, sw_node);
		}
		
		uint dev=(uint)!Api::m_use_p2p;
		for(auto &r : m_reapers) {
			r.AddWNInterface(wn_interfaces.Begin() + dev);
			if(Api::m_use_ipv6) {
				r.AddWNInterfaceV6(wn_interfaces_v6.Begin() + dev);
			}
			r.AddWNDevice(devs.Get(dev));
			dev++;
		}
		
		Ips.NewNetwork();
	}
}

Ptr<MyApp>
Node::GetApp(int i) {
	if (i >= (int)m_reaper_apps.GetN()) {
		std::cout << "no such reaper\n";
	}
	return DynamicCast<MyApp>(m_reaper_apps.Get(i));
}

void
Node::InstallApiApps(int node_num, Api* api) {
	int i=0;
	for(auto& r : m_reapers) {
		Ptr<MyApp> app = CreateObject<MyApp> ();
		app->Setup(node_num, i, 2000, api);
		r.GetNode()->AddApplication (app);
		m_reaper_apps.Add(app);
		app->SetStartTime (Seconds (0));
		// app->SetStopTime (Seconds (20.));
		i++;
	}
	
	if(Api::m_use_ipv6) {
		for(uint i=0; i < m_switch_nodes.GetN(); i++) {
			auto app = m_radvd_helper.Install(m_switch_nodes.Get(i));
			app.Start(Seconds(0));
			app.Stop(Seconds(100));
		}
	}
}

}


