#include "nodes.h"
#include "api.h"
#include "modified-default-simulator-impl.h"

#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

// #include "ns3/olsr-helper.h"
// #include "ns3/ipv4-static-routing-helper.h"
// #include "ns3/ipv4-list-routing-helper.h"
// #include "ns3/ipv4-nix-vector-helper.h"


#include "systemc.h"

#include <iostream>
using namespace std;

#include "common.h"

Sim::Api api;

// 0 1 2 3
// 4 5 6 7
void send_pkt() {
	Sim::SimPacket pkt;
	pkt.from_node = 0;
	pkt.from_reaper = 0;
	
	pkt.to_node = 0;
	pkt.to_reaper = 7;
	
	//pkt.to_port = 2000;
	pkt.content = "heyy";
	TRACE();
	api.SendPacket(pkt);//TODO This fail.
}

void on_recv(const Sim::SimPacket& pkt) {
	std::cout << "recv (" << pkt.to_node << "-" << pkt.to_reaper << ") " <<
		" msg from (" << pkt.from_node << "-" << pkt.from_reaper << 
		") msg: " << pkt.content << "\n";
}


// For 10 becons
sc_time sim_duration(10, SC_US);

struct BeaconModule: sc_module
{
   
   
	SC_CTOR(BeaconModule)
	{
		SC_THREAD(beacon_thread);
	}

	void beacon_thread()
	{
		cout << "Start of beacon_thread" << endl;
		while(true) {
			cout << "SystemC beacon @ " << sc_time_stamp() << endl;
			wait(5, SC_US);
		}
	}
};



class NS3SystemCCosimulator {

public:

	BeaconModule beacon;
	//ModifiedDefaultSimulatorImpl ns3_sim;
	
	NS3SystemCCosimulator()
	: beacon("beacon")
	{

	}


	void ns3_init() {
		Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
		
		GlobalValue::Bind ("SimulatorImplementationType", StringValue("ns3::ModifiedDefaultSimulatorImpl"));

		ns3::Time::SetResolution(ns3::Time::Unit::PS);
		

		// Reaper reaper_links
		PointToPointHelper reaper_link;
		reaper_link.SetDeviceAttribute  ("DataRate", StringValue ("100Mbps"));
		reaper_link.SetChannelAttribute ("Delay", StringValue ("1us"));
		

		InternetStackHelper internet;
		Sim::Node::internet = internet;
		// IP adresses
		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("10.0.0.0", "255.255.255.0");
		
		// node1 with matrix 4x2 reapers, 2 WaveNet ports per reaper
		Sim::Node *node0 = new Sim::Node("node0", 4, 2, 2, reaper_link);
		
		// node2 with matrix 4x2 reapers, 2 WaveNet ports per reaper
		Sim::Node *node1 = new Sim::Node("node1", 4, 2, 2, reaper_link);
		
		// switch
		auto sw = CreateObject<Node>();
		internet.Install(sw);
		
		// switch for switch
		PointToPointHelper sw_link;
		sw_link.SetDeviceAttribute  ("DataRate", StringValue ("100Mbps"));
		sw_link.SetChannelAttribute ("Delay", StringValue ("1us")); // propagation delay
		Names::Add("switch", sw);
		
		// IP addresses for node0 are 10.0.xxx.xxx
		node0->AssignIpv4Addresses(ipv4);
		// IP addresses for node1 are 10.1.xxx.xxx
		ipv4.SetBase ("10.1.0.0", "255.255.255.0");
		node1->AssignIpv4Addresses(ipv4);
		// ipv4.SetBase ("10.2.0.0", "255.255.255.0");
		ipv4.SetBase ("10.2.0.0", "255.255.255.0");
		node0->ConnectToSwitch(sw, sw_link, ipv4);
		node1->ConnectToSwitch(sw, sw_link, ipv4);
		
		api.AddNode(*node0);
		api.AddNode(*node1);
		api.InstallApiApps();
		api.SetRecvCallback(&on_recv);
		
		api.WriteIps("ips.txt");
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		
		
		
		

		
		//Simulator::Schedule(MicroSeconds(5), send_pkt);
		
		

		
		

		//reaper_link.EnablePcapAll("p");
		
		//api.WriteRouting("global-routing.routes");
		//api.WriteChannelStats("channels.csv");
		//Simulator::Run();
		


		cout << "NS3 resolution (8 is PS): " << ns3::Time::GetResolution() << endl;
		
		int i=0;
		while(api.GetNextEventTime() != (uint64_t)-1) {
			api.ProcessOneEvent();
		}

		for(int i = 0; i < 250; i++) {
			Simulator::Schedule(MicroSeconds(1+ i*2), send_pkt);
		}
	}



	void systemc_init(){


		sc_start( SC_ZERO_TIME );        // Run the initialization phase to create pending activity
	
		cout << "SC resolution (expect 1 ps): " << sc_get_time_resolution() << endl;
	}


	ModifiedDefaultSimulatorImpl* ns3_get_sim_imp() const {
		return dynamic_cast<ModifiedDefaultSimulatorImpl*>(
			&*Simulator::GetImplementation()
		);
	}

	static sc_time ns3_to_sc_time(const ns3::Time& t) {
		//return sc_time::from_value(t.GetPicoSeconds());
		return sc_time(t.GetPicoSeconds(), sc_core::SC_PS);
	}

	static sc_time uint64_t_to_sc_time(const uint64_t t) {
		//return sc_time::from_value(t);
		return sc_time(t, sc_core::SC_PS);
	}

	sc_time ns3_current_time() const {
		return ns3_to_sc_time(Simulator::Now());
	}
	sc_time ns3_time_to_next_event() const {
		return uint64_t_to_sc_time(api.GetNextEventTime() - Simulator::Now().GetPicoSeconds());
	}
	sc_time ns3_next_event_at_time() const {
		return uint64_t_to_sc_time(api.GetNextEventTime());
	}
	bool ns3_no_more_events(){
		return (api.GetNextEventTime() == (uint64_t)-1);
	}
	void ns3_process_one_event(){
		api.ProcessOneEvent();
	}



	sc_time sc_current_time() const {
		return sc_time_stamp();
	}
	sc_time sc_time_to_next_event() const {
		return sc_time_to_pending_activity();
	}
	sc_time sc_next_event_at_time() const {
		return sc_current_time() + sc_time_to_next_event();
	}
	bool sc_no_more_events(){
		return !sc_pending_activity();
	}
	void sc_process_one_event(){
		sc_start(sc_time_to_pending_activity());
	}


	void cosimulation() {
		cout << "cosimulation start" << endl;
		
		//while(sc_current_time() <= sim_duration) {
		for(int i = 0; i < 3000; i++){
			cout << "-----------------"<< endl;
			DEBUG(ns3_current_time());
			DEBUG(sc_current_time());

			if(sc_no_more_events() && ns3_no_more_events()){
				cout << "No more events!" << endl;
				break;
			}	

			if(sc_no_more_events()){
				cout << "No more SystemC events!" << endl;
				break;//TODO From now should skip calling of sc3 funs.
			}
			if(ns3_no_more_events()){
				cout << "No more NS3 events!" << endl;
				break;//TODO From now should skip calling of ns3 funs.
			}

#if 1
		
			
			DEBUG(ns3_next_event_at_time());
			DEBUG(sc_next_event_at_time());
			//ns3_process_one_event();

#endif

#if 1
			cout << "cosimulation loop beg ns3 @ " << ns3_current_time() << endl;
			cout << "cosimulation loop beg sc @ " << sc_current_time() << endl;

			if(ns3_next_event_at_time() <= sc_next_event_at_time()) {
//				cout << "ns3_no_more_events = " << ns3_no_more_events() << endl;
//				cout << "to next event = " << ns3_time_to_next_event() << endl;
				ns3_process_one_event();
			}else{
				sc_process_one_event();
			}
			
			cout << "cosimulation loop end ns3 @ " << ns3_current_time() << endl;
			cout << "cosimulation loop end sc @ " << sc_current_time() << endl;

			//break;
#endif
#if 0
			cout << "cosimulation loop begin @ " << sc_current_time() << endl;

			if(sc_no_more_events()){
				cout << "No more events!" << endl;
			}

			sc_process_one_event();
			cout << "cosimulation loop end @ " << sc_current_time() << endl;
		
#endif
		}
	}
};


int sc_main(int argc, char *argv[]) {


	NS3SystemCCosimulator c;
	c.ns3_init();
	c.systemc_init();

	c.cosimulation();


	//Simulator::Destroy ();
	return 0;
}
