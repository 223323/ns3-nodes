#ifndef API_H
#define API_H
#include "rack.h"
#include <functional>
#include "modified-default-simulator-impl.h"

namespace Sim {

class MyApp;
struct SimPacket {
	int from_node;
	int from_reaper;
	int to_node;
	int to_reaper;
	std::string content;
	int virtual_size;
	Address from_address;
};

typedef std::function<void(const SimPacket&)> RecvCallbackType;

struct Link {
	std::string speed;
	std::string delay;
};

class Api {
friend class MyApp;

public:
	Api();
	void AddNode(int rack, Node* node);
	void SendPacket(const SimPacket& pkt);
	void SetRecvCallback(RecvCallbackType recv_callback);
	void AddSwitch();
	void InstallApiApps();
	void PrintIps(std::ostream& o=std::cout);
	void WriteIps(std::string filename);
	void WriteChannelStats(std::string filename);
	void WriteRouting(std::string filename);
	void CreateRacks(int racks);
	
	void SetReaperLink(std::string speed, std::string delay);
	void SetRackLink(std::string speed, std::string delay);
	void SetSpineSwitchLink(std::string speed, std::string delay);
	void ConnectToSpineSwitch();

	bool Finished();
	uint64_t GetNextEventTime();
	void ProcessOneEvent();
	void ProcessAllEvents();
	
	static void						UseIPv6(bool value);
	static void						UseP2P(bool value);
	
	static bool						m_use_ipv6;
	static bool						m_use_p2p;
	static bool						m_reapers_circular;
	static Link 					m_reaper_link;
	static Link 					m_rack_link;
	static Link 					m_spine_switch_link;
	static ns3::InternetStackHelper internet;
private:
	
	void ProcessChannelStats();
	
	const std::vector<std::string> GetElementsFromContext (const std::string& context);
	Ptr <ns3::Node> GetNodeFromContext (const std::string& context);
	
	void Ipv4TxTrace (std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interfaceIndex);
	void Ipv4RxTrace (std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interfaceIndex);
	void Ipv4DropTrace (std::string context, 
			   const Ipv4Header & ipv4Header, 
			   Ptr<const Packet> p, 
			   Ipv4L3Protocol::DropReason dropReason, 
			   Ptr<Ipv4> ipv4, 
			   uint32_t);
	
	void DevTxTrace (std::string context, 
			 Ptr<const Packet> p,
			 Ptr<NetDevice> tx, 
			 Ptr<NetDevice> rx,
			 Time txTime, 
			 Time rxTime);
	
	void OnRecvMsg(Ptr<Packet> packet, Address address, MyApp* app);
	
	RecvCallbackType   m_recv_callback;
	std::vector<Node*> m_nodes;
	uint16_t 		   m_port;
	std::map<uint64_t, std::pair<int,int>> m_ip_addr_map;
	
	Ptr<OutputStreamWrapper> m_channel_stats_output;
	std::vector<int> 		 m_channels;
	int 					 m_idle, m_max_idle;
	
	Ptr<ModifiedDefaultSimulatorImpl> m_modif;
	
	std::vector<Rack> 		m_racks;
	Ptr<ns3::Node>		    m_spine_switch;
	
	
};

}
#endif
