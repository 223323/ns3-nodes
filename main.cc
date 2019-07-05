#include "api.h"

#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

// #include "ns3/olsr-helper.h"
// #include "ns3/ipv4-static-routing-helper.h"
// #include "ns3/ipv4-list-routing-helper.h"
// #include "ns3/ipv4-nix-vector-helper.h"

Sim::Api api;

// 0 1 2 3
// 4 5 6 7
void send_pkt() {
	Sim::SimPacket pkt;
	
	pkt.from_node = 1;
	pkt.from_reaper = 3;
	
	pkt.to_node = 0;
	pkt.to_reaper = 6;
	
	pkt.virtual_size = 3;
	pkt.content = "heyy";
	api.SendPacket(pkt);
}

void on_recv(const Sim::SimPacket& pkt) {
	std::cout << "[" << ns3::Simulator::Now().GetSeconds() << "] recv (" << pkt.to_node << "-" << pkt.to_reaper << ") " <<
		" msg from (" << pkt.from_node << "-" << pkt.from_reaper << 
		") msg: " << pkt.content << "\n";
}

int main (int argc, char *argv[]) {
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
	GlobalValue::Bind ("SimulatorImplementationType", StringValue("ns3::ModifiedDefaultSimulatorImpl"));
	
	api.SetReaperLink("10Mbps", "1ms");
	api.SetRackLink("10Mbps", "1ms");
	api.SetSpineSwitchLink("10Mbps", "1ms");
	
	Sim::Api::UseIPv6(1);
	Sim::Api::UseP2P(1);
	
	// matrix 2x4 reapers, 2 WaveNet ports per reaper
	Sim::Node *node0 = new Sim::Node(2, 4, 2);
	Sim::Node *node1 = new Sim::Node(2, 4, 2);
	Sim::Node *node2 = new Sim::Node(2, 4, 2);
	Sim::Node *node3 = new Sim::Node(2, 4, 2);
	
	api.CreateRacks(3);
	api.AddNode(0, node0); // add node0 to rack 0
	api.AddNode(0, node1); // add node0 to rack 0
	api.AddNode(1, node2); // add node0 to rack 1
	api.AddNode(2, node3); // add node0 to rack 2
	
	api.SetRecvCallback(&on_recv);
	api.InstallApiApps();
	api.WriteIps("ips.txt");
	
	api.ProcessAllEvents();
	for(int i=0; i < 20; i++) {
		Simulator::Schedule(Seconds(i*2), send_pkt);
	}
	
	/*
		Write PCAP files, which are named by this rule:
			<Prefix>-<NodePrefixName>-<netdevice>.pcap
		for example:
			p-node1-0-3.pcap
	*/
	PointToPointHelper ptph;
	ptph.EnablePcapAll("p");
	CsmaHelper ch;
	ch.EnablePcapAll("c");
	
	api.WriteRouting("global-routing.routes");
	api.WriteChannelStats("channels.csv");
	api.ProcessAllEvents();
	Simulator::Destroy ();
	return 0;
}
