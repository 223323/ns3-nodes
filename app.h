#ifndef APP_H
#define APP_H
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


using namespace ns3;

namespace Sim {
class Api;
class MyApp : public Application {
public:
    MyApp ();
    virtual ~MyApp ();

    static TypeId   GetTypeId (void);
    void 			Setup (int node_num, int reaper_num, uint16_t port, Api* api);
    
    void 			SendPacketTo(std::string packet, Address remote_address);
    
	uint16_t 		GetPort();
	int 			GetNodeNum();
	int 			GetReaperNum();
	
private:
	
    virtual void 	StartApplication (void);
    virtual void 	StopApplication (void);
    void 			HandleRead (Ptr<Socket> socket);
    void 			HandleAccept (Ptr<Socket> socket, const Address& from);
    void 			HandlePeerClose (Ptr<Socket> socket);
    void 			HandlePeerError (Ptr<Socket> socket);
    void 			ScheduleTx (void);
    void 			SendPacket (void);
    void 			Listen();

    Ptr<Socket>     m_socket;
    Address         m_peer;
    uint16_t		m_port;
    uint32_t        m_packetSize;
    uint32_t        m_nPackets;
    DataRate        m_dataRate;
    EventId         m_sendEvent;
    bool            m_running;
    uint32_t        m_packetsSent;
    
    Api*			m_api;
    int				m_node_num;
    int				m_reaper_num;
};
}

#endif
