#include "nodes.h"
#include "app.h"



NS_LOG_COMPONENT_DEFINE ("Nodes");

namespace Sim {

void
Reaper::AssignIpv4Addresses(ns3::Ipv4AddressHelper& Ips) {
	// m_interfaces.Add(Ips.Assign(m_devices));
	m_interfaces = Ips.Assign(m_devices);
	Ips.NewNetwork();
}

void
Reaper::PrintIps(std::ostream& o, int indent) {
	for(uint i=0; i < m_interfaces.GetN(); i++) {
		o.write("\t\t\t\t", indent);
		o << GetAddress(i) << " channel: " << GetChannelId(i) << "\n";
	}
}

uint32_t
Reaper::GetChannelId(int i) {
	uint32_t idx = (uint32_t)i;
	if (idx < m_devices.GetN()) {
		return m_devices.Get(idx)->GetChannel()->GetId();
	} else if(idx-m_devices.GetN() < m_switch_devices.GetN()) {
		return m_switch_devices.Get(idx-m_devices.GetN())->GetChannel()->GetId();
	}
	return 0;
}

ns3::Ipv4Address
Reaper::GetAddress(int i) {
	return m_interfaces.GetAddress(i,0);
}

int
Reaper::GetAddressNum() {
	return m_interfaces.GetN();
}

ns3::Ipv4InterfaceContainer
Reaper::GetInterfaces() {
	return m_interfaces;
}

Node::Node(std::string name, int nRows, int nCols, int nWaveNet, ns3::PointToPointHelper link)
	: m_name(name),
	  m_cols(nCols),
	  m_rows(nRows),
	  m_nReapers(nCols*nRows),
	  m_nWaveNet(nWaveNet) 
{
		  
	m_reaper_nodes.Create(m_nReapers);
	for(int i=0; i < m_nReapers; i++) {
		m_reapers.emplace_back(m_reaper_nodes.Get(i));
		Names::Add(m_name + "-" + std::to_string(i), m_reaper_nodes.Get(i));
	}
	internet.Install(m_reaper_nodes);
	ConnectReapers(link);
}

ns3::InternetStackHelper Node::internet;


void
Node::AssignIpv4Addresses (ns3::Ipv4AddressHelper Ips) {
	for (auto &r : m_reapers) {
		r.AssignIpv4Addresses(Ips);
	}
}

void
Node::ConnectReapers(ns3::PointToPointHelper link) {
	m_reapers_connected = true;
	
	for(int x=0; x < m_cols-1; x++) {
		
		for(int y=0; y < m_rows; y++) {
			
			int n1 = m_cols * y + x;
			auto node1 = m_reapers[n1].GetNode();
			
			for(int y1=0; y1 < m_rows; y1++) {
				int n2 = m_cols * y1 + (x+1) % m_cols;
				auto node2 = m_reapers[n2].GetNode();
				
				std::cout << "Link: " << n1 << " : " << n2 << "\n";
				ns3::NetDeviceContainer devs = link.Install(node1, node2);
				
				m_reapers[n1].m_devices.Add( devs.Get(0) );
				m_reapers[n2].m_devices.Add( devs.Get(1) );
				
				devs.Get(0)->SetMtu(Mtu);
				devs.Get(1)->SetMtu(Mtu);
			}
		}
	}
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


ApplicationContainer
Node::AddOnOffApplication(Reaper& reap1, Reaper& reap2, OnOffHelper onoff, int port) {
	AddressValue remoteAddress (InetSocketAddress (reap2.GetAddress(0), port));
	onoff.SetAttribute("Remote", remoteAddress);
	ApplicationContainer apps;
	apps.Add( onoff.Install(reap1.GetNode()) );
	return apps;
}

ApplicationContainer
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

void
Node::ConnectToSwitch(Ptr<ns3::Node> sw_node, ns3::PointToPointHelper link, ns3::Ipv4AddressHelper& Ips) {
	NS_ASSERT_MSG( m_reapers_connected, "ConnectReapers first!" );
	static int cnt=0;
	
	for(auto &r : m_reapers) {
		for(int i=0; i < m_nWaveNet; i++)  {
			
			ns3::NetDeviceContainer devs = link.Install(r.GetNode(), sw_node);
			Names::Add("sw"+std::to_string(++cnt) + "-" + std::to_string(i), devs.Get(0));
			// r.m_devices.Add( devs );
			auto ifaces = Ips.Assign(devs);
			r.m_switch_devices.Add( devs.Get(0) );
			r.m_interfaces.Add(ifaces.Get(0));
			devs.Get(0)->SetMtu(Mtu);
			devs.Get(1)->SetMtu(Mtu);
		}
	}
	// Ips.NewNetwork();
}

Ptr<MyApp>
Node::GetApp(int i) {
	if (i >= (int)m_reaper_apps.GetN()) {
		std::cout << "no such reaper\n";
	}
	return StaticCast<MyApp>(m_reaper_apps.Get(i));
}

void
Node::InstallApiApps(int node_num, Api* api) {
	int i=0;
	for(auto& r : m_reapers) {
		Ptr<MyApp> app = CreateObject<MyApp> ();
		app->Setup(node_num, i, 2000, api);
		r.GetNode()->AddApplication (app);
		m_reaper_apps.Add(app);
		// app->SetStartTime (Seconds (1.));
		// app->SetStopTime (Seconds (20.));
		i++;
	}
}

}


