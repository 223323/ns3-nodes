#include "nodes.h"
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
	pkt.from_node = 0;
	pkt.from_reaper = 0;
	
	pkt.to_node = 0;
	pkt.to_reaper = 6;
	
	pkt.content = "heyy";
	api.SendPacket(pkt);
}

void on_recv(const Sim::SimPacket& pkt) {
	std::cout << "recv (" << pkt.to_node << "-" << pkt.to_reaper << ") " <<
		" msg from (" << pkt.from_node << "-" << pkt.from_reaper << 
		") msg: " << pkt.content << "\n";
}



int main (int argc, char *argv[]) {
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
	
	GlobalValue::Bind ("SimulatorImplementationType", StringValue("ns3::ModifiedDefaultSimulatorImpl"));
	
	// Reaper reaper_links
	PointToPointHelper reaper_link;
	reaper_link.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
	reaper_link.SetChannelAttribute ("Delay", StringValue ("1ms"));
	

	InternetStackHelper internet;
	Sim::Node::internet = internet;
	// IP adresses
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.0.0.0", "255.255.255.0");
	
	// node1 with matrix 2x4 reapers, 2 WaveNet ports per reaper
	Sim::Node node0("node0", 2, 4, 2, reaper_link);
	
	// node2 with matrix 2x4 reapers, 2 WaveNet ports per reaper
	Sim::Node node1("node1", 2, 4, 2, reaper_link);
	
	// switch
	auto sw = CreateObject<Node>();
	internet.Install(sw);
	
	// switch for switch
	PointToPointHelper sw_link;
	sw_link.SetDeviceAttribute  ("DataRate", StringValue ("0.5Mbps"));
	sw_link.SetChannelAttribute ("Delay", StringValue ("1ms")); // propagation delay
	Names::Add("switch", sw);
	
	// IP addresses for node0 are 10.0.xxx.xxx
	node0.AssignIpv4Addresses(ipv4);
	// IP addresses for node1 are 10.1.xxx.xxx
	ipv4.SetBase ("10.1.0.0", "255.255.255.0");
	node1.AssignIpv4Addresses(ipv4);
	// ipv4.SetBase ("10.2.0.0", "255.255.255.0");
	ipv4.SetBase ("10.2.0.0", "255.255.255.0");
	node0.ConnectToSwitch(sw, sw_link, ipv4);
	node1.ConnectToSwitch(sw, sw_link, ipv4);
	
	api.AddNode(node0);
	api.AddNode(node1);
	api.InstallApiApps();
	api.SetRecvCallback(&on_recv);
	
	api.WriteIps("ips.txt");
	
	Simulator::Schedule(Seconds(1), send_pkt);
	Simulator::Schedule(Seconds(4), send_pkt);
	Simulator::Schedule(Seconds(7), send_pkt);
	
	/*
		OnOff application
		Parameters are (On/Off)Time, DataRate, PacketSize
	*/
	/*
	ApplicationContainer apps;
	OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
	clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
	clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
	clientHelper.SetAttribute ("DataRate", DataRateValue(DataRate("10Mbps")));
	clientHelper.SetAttribute ("PacketSize", UintegerValue(15000));
	
	apps = node0.AddOnOffApplication(node0.GetReaper(0), node0.GetReaper(6), clientHelper, 1000);
	apps.Stop(Seconds(0.0));
	apps.Start(Seconds(0.0));
	apps.Stop(Seconds(10.0));
	
	node0.PacketSinkApplication(1000);
	node1.PacketSinkApplication(1000);
	*/
	
	/*
	MobilityHelper mobility;

	mobility.SetPositionAllocator ("ns3::GridPositionAllocator", 
				"MinX", DoubleValue (0.0), "MinY", DoubleValue (0.0),"DeltaX", DoubleValue (5.0), "DeltaY", DoubleValue (10.0), 
				 "GridWidth", UintegerValue (5), "LayoutType", StringValue ("RowFirst"));

	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	
	mobility.Install (sw);
	mobility.Install (node0.GetNodes());
	mobility.Install (node1.GetNodes());

	AnimationInterface anim ("SimpleNS3Simulation_NetAnimationOutput.xml"); 
	
	anim.SetConstantPosition (nodes.Get(0), 0, 5);
	anim.SetConstantPosition (nodes.Get(1), 10, 5);
	
	AnimationInterface anim (animFile);
	
	AsciiTraceHelper ascii;
	reaper_link.EnableAsciiAll (ascii.CreateFileStream ("reaper_link.tr"));
	*/
	
	
	/* 
		Write PCAP files, which are named by this rule:
			<Prefix>-<NodePrefixName>-<netdevice>.pcap
		for example:
			p-node1-0-3.pcap
	*/
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	reaper_link.EnablePcapAll("p");
	
	api.WriteRouting("global-routing.routes");
	api.WriteChannelStats("channels.csv");
	
	
	
	
	
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
