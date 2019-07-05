#include "reaper.h"

NS_LOG_COMPONENT_DEFINE ("Reaper");

namespace Sim {

void
Reaper::AssignIpv4Addresses(ns3::Ipv4AddressHelper& Ips) {
	// m_interfaces.Add(Ips.Assign(m_devices));
	m_interfaces = Ips.Assign(m_devices);
	Ips.NewNetwork();
}

void
Reaper::AssignIpv6Addresses(ns3::Ipv6AddressHelper& Ips) {
	// m_interfaces.Add(Ips.Assign(m_devices));
	m_interfaces_v6 = Ips.Assign(m_devices);
	Ips.NewNetwork();
}

void
Reaper::PrintIps(std::ostream& o, int indent) {
	for(uint i=0; i < m_interfaces.GetN(); i++) {
		o.write("\t\t\t\t", indent);
		o << GetAddress(i) << " channel: " << GetChannelId(i) << "\n";
	}
	o.write("\t\t\t\t", indent);
	o << "WN Ipv4\n";
	for(uint i=0; i < m_wn_interfaces.GetN(); i++) {
		o.write("\t\t\t\t", indent);
		o << GetWNAddress(i) << " channel: " << GetChannelId(i) << "\n";
	}
	o.write("\t\t\t\t", indent);
	o << "WN Ipv6\n";
	for(uint i=0; i < m_wn_interfaces_v6.GetN(); i++) {
		o.write("\t\t\t\t", indent);
		o << GetWNAddressV6(i) << " channel: " << GetChannelId(i) << "\n";
	}
}

uint32_t
Reaper::GetChannelId(int i) {
	return m_devices.Get(i)->GetChannel()->GetId();
}

uint32_t
Reaper::GetWNChannelId(int i) {
	return m_wn_devices.Get(i)->GetChannel()->GetId();
}

ns3::Ipv4Address
Reaper::GetAddress(int i) {
	return m_interfaces.GetAddress(i,0);
}

ns3::Ipv6Address
Reaper::GetAddressV6(int i) {
	return m_interfaces_v6.GetAddress(i,0);
}

ns3::Ipv4Address
Reaper::GetWNAddress(int i) {
	return m_wn_interfaces.GetAddress(i,0);
}

ns3::Ipv6Address
Reaper::GetWNAddressV6(int i) {
	return m_wn_interfaces_v6.GetAddress(i,1);
}

int
Reaper::GetAddressNum() {
	return m_interfaces.GetN();
}

int
Reaper::GetWNAddressNum() {
	return m_wn_interfaces.GetN();
}

int
Reaper::GetWNAddressNumV6() {
	return m_wn_interfaces_v6.GetN();
}

ns3::Ipv4InterfaceContainer
Reaper::GetInterfaces() {
	return m_interfaces;
}

ns3::Ipv4InterfaceContainer
Reaper::GetWNInterfaces() {
	return m_wn_interfaces;
}

ns3::Ipv6InterfaceContainer
Reaper::GetInterfacesV6() {
	return m_interfaces_v6;
}

ns3::Ipv6InterfaceContainer
Reaper::GetWNInterfacesV6() {
	return m_wn_interfaces_v6;
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
Reaper::AddSwitchDevice(Ptr<ns3::NetDevice> dev) {
	m_switch_devices.Add(dev);
}

void
Reaper::AddWNDevice(Ptr<ns3::NetDevice> dev) {
	m_wn_devices.Add(dev);
}

void
Reaper::AddInterface(ns3::Ipv4InterfaceContainer::Iterator it) {
	m_interfaces.Add(*it);
}

void
Reaper::AddInterfaceV6(ns3::Ipv6InterfaceContainer::Iterator it) {
	m_interfaces_v6.Add(it->first, it->second);
}

void
Reaper::AddWNInterface(ns3::Ipv4InterfaceContainer::Iterator it) {
	m_wn_interfaces.Add(*it);
}

void
Reaper::AddWNInterfaceV6(ns3::Ipv6InterfaceContainer::Iterator it) {
	m_wn_interfaces_v6.Add(it->first, it->second);
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

}
