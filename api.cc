#include "api.h"
#include "app.h"
#include "modified-default-simulator-impl.h"
namespace Sim {

Api::Api() : m_idle(0) {
	m_max_idle = 10;
	m_port = 2000;
}

void
Api::AddNode(Node& node) {
	m_nodes.push_back(&node);
	
	auto &reapers = node.GetReapers();
	for(size_t r=0; r < reapers.size(); r++) {
		auto ifaces = reapers[r].GetInterfaces();
		for(int i=0; i < (int)ifaces.GetN(); i++) {
			Ipv4Address addr = ifaces.GetAddress(i,0);
			m_ip_addr_map[addr.Get()] = std::make_pair((int)m_nodes.size()-1, (int)r);
		}
	}
}

void
Api::SendPacket(const SimPacket& pkt) {
	if(pkt.from_node >= (int)m_nodes.size() || pkt.to_node >= (int)m_nodes.size()) {
		return;
	}
	auto &node1 = *m_nodes[pkt.from_node];
	auto &node2 = *m_nodes[pkt.to_node];
	// auto &reap1 = node1.GetReaper(pkt.from_reaper);
	auto &reap2 = node2.GetReaper(pkt.to_reaper);
	
	
	Address remoteAddress (InetSocketAddress (reap2.GetAddress(rand() % reap2.GetAddressNum()), m_port));
	// Address remoteAddress (InetSocketAddress (reap2.GetAddress(0), pkt.to_port));
	auto app = node1.GetApp(pkt.from_reaper);
	if (app) {
		app->SendPacketTo(pkt.content, remoteAddress);
	}
}

void
Api::SetRecvCallback(RecvCallbackType recv_callback) {
	m_recv_callback = recv_callback;
}

void
Api::InstallApiApps() {
	m_modif = StaticCast<ModifiedDefaultSimulatorImpl>( Simulator::GetImplementation() );
	for(uint32_t i=0; i < m_nodes.size(); i++) {
		m_nodes[i]->InstallApiApps(i, this);
	}
}

void
Api::PrintIps(std::ostream& o) {
	int i=0;
	for(auto n : m_nodes) {
		o << "node " << i << "\n";
		n->PrintIps(o, 1);
		i++;
	}
}

void
Api::OnRecvMsg(Ptr<Packet> packet, Address address, MyApp* app) {
	
	SimPacket pkt;
	
	// std::cout << "on recv msg: " << m_modif->GetTimeToNextEvent() << "\n";
	
	// get from address
	pkt.from_address = address;
	InetSocketAddress addr = InetSocketAddress::ConvertFrom (address);
	Ipv4Address ipv4 = addr.GetIpv4();
	auto m = m_ip_addr_map[ipv4.Get()];
	pkt.from_node = m.first;
	pkt.from_reaper = m.second;
	
	// ports if needed
	// pkt.from_port = addr.GetPort();
	// pkt.to_port = app->GetPort();
	
	// get to address
	pkt.to_node = app->GetNodeNum();
	pkt.to_reaper = app->GetReaperNum();
	
	
	// auto &node2 = *m_nodes[pkt.to_node];
	// auto &reap1 = node1.GetReaper(pkt.from_reaper);
	// auto &reap2 = node2.GetReaper(pkt.to_reaper);
	
	
	// addr = InetSocketAddress::ConvertFrom (packet->GetAddress());
	// ipv4 = addr.GetIpv4();
	// reap2.GetChirplet(ipv4.Get());
	pkt.to_chirplet = 0;
	
	// content
	pkt.content.resize(packet->GetSize());
	packet->CopyData((uint8_t*)&pkt.content[0], packet->GetSize());
	if(m_recv_callback) {
		m_recv_callback(pkt);
	}
}



void 
Api::DevTxTrace (std::string context, 
			 Ptr<const Packet> p,
			 Ptr<NetDevice> tx, 
			 Ptr<NetDevice> rx,
			 Time txTime, 
			 Time rxTime)
{

	Time now = Simulator::Now ();
	/*
	double fbTx = now.GetSeconds ();
	double lbTx = (now + txTime).GetSeconds ();
	double fbRx = (now + rxTime - txTime).GetSeconds ();
	double lbRx = (now + rxTime).GetSeconds ();
	*/
	int ch_id = tx->GetChannel()->GetId();
	// int ch_id2 = rx->GetChannel()->GetId();
	// if (ch_id >= (int)m_channels.size()) {
		// m_channels.resize(ch_id+1);
	// }
	m_channels[ch_id] += p->GetSize();
	m_idle = 0;
}

const std::vector<std::string> 
Api::GetElementsFromContext (const std::string& context)
{
  std::vector <std::string> elements;
  size_t pos1=0, pos2;
  while (pos1 != context.npos) {
    pos1 = context.find ("/",pos1);
    pos2 = context.find ("/",pos1+1);
    elements.push_back (context.substr (pos1+1,pos2-(pos1+1)));
    pos1 = pos2; 
    pos2 = context.npos;
  }
  return elements;
}


Ptr <ns3::Node>
Api::GetNodeFromContext (const std::string& context) {
  std::vector <std::string> elements = GetElementsFromContext (context);
  Ptr <ns3::Node> n = NodeList::GetNode (atoi (elements.at (1).c_str ()));
  NS_ASSERT (n);

  return n;
}

void
Api::Ipv4TxTrace (std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interfaceIndex) {
  const Ptr <const ns3::Node> node = GetNodeFromContext (context);
  // ++m_nodeIpv4Tx[node->GetId ()];
}
 
void
Api::Ipv4RxTrace (std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interfaceIndex) {
  const Ptr <const ns3::Node> node = GetNodeFromContext (context);
  // ++m_nodeIpv4Rx[node->GetId ()];
}

void
Api::Ipv4DropTrace (std::string context, 
			   const Ipv4Header & ipv4Header, 
			   Ptr<const Packet> p, 
			   Ipv4L3Protocol::DropReason dropReason, 
			   Ptr<Ipv4> ipv4, 
			   uint32_t)
{
  const Ptr <const ns3::Node> node = GetNodeFromContext (context);
  // ++m_nodeIpv4Drop[node->GetId ()];
}



void
Api::ProcessChannelStats() {
	auto &s = *m_channel_stats_output->GetStream();
	std::cout << "sim time: " << Simulator::Now().GetSeconds() << "\n";
	int i=0;
	for(int &c : m_channels) {
		s << c;
		if (i < (int)m_channels.size()-1) {
			s << ", ";
		}
		c = 0;
		i++;
	}
	if (m_idle++ < m_max_idle) {
		Simulator::Schedule(Seconds(1), &Api::ProcessChannelStats, this);
	}
	s << "\n";
}



void
Api::WriteIps(std::string filename) {
	Ptr<OutputStreamWrapper> ips = Create<OutputStreamWrapper> (filename, std::ios::out);
	PrintIps(*ips->GetStream());
}


void
Api::WriteChannelStats(std::string filename) {
	m_channel_stats_output = Create<OutputStreamWrapper> (filename, std::ios::out);
	Config::Connect ("/ChannelList/*/TxRxPointToPoint", MakeCallback (&Api::DevTxTrace, this));
    Config::Connect ("/NodeList/*/$ns3::Ipv4L3Protocol/Tx", MakeCallback (&Api::Ipv4TxTrace, this));
	Config::Connect ("/NodeList/*/$ns3::Ipv4L3Protocol/Rx", MakeCallback (&Api::Ipv4RxTrace, this));
	Config::Connect ("/NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback (&Api::Ipv4DropTrace, this));
	Simulator::Schedule(Seconds(1), &Api::ProcessChannelStats, this);
	m_channels.resize(ChannelList::GetNChannels());
}


void
Api::WriteRouting(std::string filename) {
	Ipv4GlobalRoutingHelper g;
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (filename, std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (12), routingStream);
}

uint64_t
Api::GetNextEventTime() {
	return m_modif->IsFinished() ? -1 : m_modif->GetNextEventTime().NanoSeconds();
}

void
Api::ProcessOneEvent();
	if (!m_modif->IsFinished()) {
		m_modif->ProcessOneEvent();
	}
}
