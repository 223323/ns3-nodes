#include "nodes.h"
#include "app.h"



NS_LOG_COMPONENT_DEFINE ("Nodes");

namespace Sim {

void
Reaper::AssignIpv4Addresses(ns3::Ipv4AddressHelper& Ips) {
	for(uint i=0; i < m_devices.GetN(); i++) {
		m_interfaces.Add( Ips.Assign(m_devices.Get(i)) );
		Ips.NewNetwork();
	}
}

void
Reaper::PrintIps(std::ostream& o, int indent) {
	// for(uint i=0; i < m_interfaces.GetN(); i++) {
	for(uint i=0; i < (uint)GetAddressNum(); i++) {
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
	// std::cout << "GetAddress " << m_devices.GetN() << "\n";
	// InetSocketAddress addr2 = InetSocketAddress::ConvertFrom (m_devices.Get(i)->GetAddress());
	// Ipv4Address addr = addr2.GetIpv4();
	// return addr;
}

int
Reaper::GetAddressNum() {
	return m_devices.GetN();
}

ns3::Ipv4InterfaceContainer
Reaper::GetInterfaces() {
	return m_interfaces;
}

ns3::NetDeviceContainer
Reaper::GetDevices() {
	return m_devices;
}

void
Reaper::AddDevice(Ptr<ns3::NetDevice> dev, bool isLeft) {
	m_devices.Add(dev);
	if(isLeft) {
		m_left_devices.Add(dev);
	} else {
		m_right_devices.Add(dev);
	}
}

void
Reaper::AddSwitchDevice(Ptr<ns3::NetDevice> dev, bool isLeft) {
	m_switch_devices.Add(dev);
	if(isLeft) {
		m_left_devices.Add(dev);
	} else {
		m_right_devices.Add(dev);
	}
}

void
Reaper::MapChirplets() {
	for(uint32_t i=0; i < m_left_devices.GetN(); i++) {
		auto dev = m_left_devices.Get(i);
		InetSocketAddress addr = InetSocketAddress::ConvertFrom (dev->GetAddress());
		Ipv4Address ipv4 = addr.GetIpv4();
		m_map_ipv4_to_chirplet[ipv4.Get()] = 0;
	}
	
	for(uint32_t i=0; i < m_right_devices.GetN(); i++) {
		auto dev = m_right_devices.Get(i);
		InetSocketAddress addr = InetSocketAddress::ConvertFrom (dev->GetAddress());
		Ipv4Address ipv4 = addr.GetIpv4();
		m_map_ipv4_to_chirplet[ipv4.Get()] = 1;
	}
}

int
Reaper::GetChirplet(uint32_t ip) {
	return m_map_ipv4_to_chirplet[ip];
}


// NODE
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
Node::AssignIpv4Addresses (ns3::Ipv4AddressHelper& Ips) {
	
	for(auto &reap : m_reapers) {
		reap.m_interfaces = Ips.Assign(reap.m_devices);
		Ips.NewNetwork();
		// reap.AssignIpv4Addresses(Ips);
	}
	
	// for(auto devs : m_dev_pairs) {
		// std::cout << "dev " << devs.GetN() << "\n";
		// Ips.Assign(devs);
		// Ips.NewNetwork();
	// }
	
}

void
Node::ConnectReapers(ns3::PointToPointHelper link) {
	m_reapers_connected = true;
	std::cout << m_name << "\n";
	for(int x=0; x < m_cols-1; x++) {
		
		for(int y=0; y < m_rows; y++) {
			
			int n1 = m_cols * y + x;
			auto node1 = m_reapers[n1].GetNode();
			
			for(int y1=0; y1 < m_rows; y1++) {
				int n2 = m_cols * y1 + (x+1) % m_cols;
				auto node2 = m_reapers[n2].GetNode();
				
				std::cout << "\tLink: " << n1 << " : " << n2 << "\n";
				ns3::NetDeviceContainer devs = link.Install(node1, node2);
				
				m_dev_pairs.push_back(devs);
				
				m_reapers[n1].AddDevice( devs.Get(0), true );
				m_reapers[n2].AddDevice( devs.Get(1), false );
				
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
			ns3::Ipv4InterfaceContainer ifaces = Ips.Assign(devs);
			r.AddSwitchDevice( devs.Get(0), i < m_nWaveNet/2);
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
		app->SetStartTime (Seconds (0));
		i++;
	}
}

}


