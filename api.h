#ifndef API_H
#define API_H
#include "nodes.h"
#include <functional>
#include "modified-default-simulator-impl.h"

namespace Sim {

class MyApp;
struct SimPacket {
	int from_node;
	int from_reaper;
	int to_node;
	int to_reaper;
	int to_chirplet;
	std::string content;
	Address from_address;
};

typedef std::function<void(const SimPacket&)> RecvCallbackType;

class Api {
friend class MyApp;

public:
	Api();
	void AddNode(Node& node);
	void SendPacket(const SimPacket& pkt);
	void SetRecvCallback(RecvCallbackType recv_callback);
	void AddSwitch();
	void InstallApiApps();
	void PrintIps(std::ostream& o=std::cout);
	void WriteIps(std::string filename);
	void WriteChannelStats(std::string filename);
	void WriteRouting(std::string filename);
	
	
	bool Finished();
	uint64_t GetNextEventTime();
	void ProcessOneEvent();
	
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
	RecvCallbackType m_recv_callback;
	std::vector<Node*> m_nodes;
	uint16_t m_port;
	std::map<uint32_t, std::pair<int,int>> m_ip_addr_map;
	
	Ptr<OutputStreamWrapper> m_channel_stats_output;
	std::vector<int> m_channels;
	int m_idle, m_max_idle;
	
	Ptr<ModifiedDefaultSimulatorImpl> m_modif;
};

}
#endif
