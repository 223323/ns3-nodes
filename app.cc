#include "app.h"
#include "api.h"

namespace Sim {
MyApp::MyApp ()
    : m_socket (0),
      m_peer (),
      m_packetSize (0),
      m_nPackets (0),
      m_dataRate (0),
      m_sendEvent (),
      m_running (false),
      m_packetsSent (0) {
}

NS_LOG_COMPONENT_DEFINE ("MyApp");

MyApp::~MyApp () {
    m_socket = 0;
}

TypeId
MyApp::GetTypeId (void) {
    static TypeId tid = TypeId ("MyApp")
                        .SetParent<Application> ()
                        .SetGroupName ("Nodes")
                        .AddConstructor<MyApp> ()
                        ;
    return tid;
}

void
MyApp::Setup (int node_num, int reaper_num, uint16_t port, Api* api) {
    m_socket = 0;
	m_reaper_num = reaper_num;
    m_port = port;
    m_api = api;
	m_node_num = node_num;
}

void
MyApp::StartApplication (void) {
    m_running = true;
    m_packetsSent = 0;
    // m_socket->Bind ();
    // m_socket->Connect (m_peer);
    Listen();
    // SendPacket ();
}

void
MyApp::Listen () {
    NS_LOG_FUNCTION (this);
    m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress ipaddr(Ipv4Address::GetAny(), m_port);
    Address local(ipaddr);
    // std::cout << "Listening at ipaddr: " << ipaddr.GetIpv4() << " : " << ipaddr.GetPort() << "\n";
    m_socket->Bind (local);
    m_socket->Listen ();
    m_socket->SetRecvCallback (MakeCallback (&MyApp::HandleRead, this));
    m_socket->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeCallback (&MyApp::HandleAccept, this));
    m_socket->SetCloseCallbacks (
        MakeCallback (&MyApp::HandlePeerClose, this),
        MakeCallback (&MyApp::HandlePeerError, this));
}

void
MyApp::StopApplication (void) {
    m_running = false;

    if (m_sendEvent.IsRunning ()) {
        Simulator::Cancel (m_sendEvent);
    }

    if (m_socket) {
        m_socket->Close ();
    }
}

void
MyApp::SendPacketTo(std::string content, int virtual_size, Address remote_address) {
    if(m_socket) {
		Ptr<Packet> packet = Create<Packet> ((const uint8_t*)&content[0], std::min<size_t>(virtual_size, content.size()));
		int padding = std::max<int>(0, virtual_size - content.size());
		if(padding > 0) {
			packet->AddPaddingAtEnd(padding);
		}
		m_socket->SendTo (packet, 0, remote_address);
	}
}

void
MyApp::HandlePeerClose (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
MyApp::HandlePeerError (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}


void
MyApp::HandleAccept (Ptr<Socket> socket, const Address& from) {
    NS_LOG_FUNCTION (this << socket);
}

void
MyApp::HandleRead (Ptr<Socket> socket) {
    Ptr<Packet> packet;
    Address from;
	
    while ((packet = socket->RecvFrom (from))) {
        if (packet->GetSize () == 0) {
            //EOF
            break;
        }
		m_api->OnRecvMsg(packet, from, this);
    }
}

uint16_t
MyApp::GetPort() {
	return m_port;
}

int
MyApp::GetNodeNum() {
	return m_node_num;
}

int
MyApp::GetReaperNum() {
	return m_reaper_num;
}

}
