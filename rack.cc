#include "rack.h"
#include "api.h"
namespace Sim {

Rack::Rack() {
	m_router_nodes.Create(2);
	Api::internet.Install(m_router_nodes);
	m_rack_idx = m_rack_count;
	m_rack_count++;
}

int Rack::m_rack_count = 0;

void 
Rack::AddNode(Node* node) {
	m_nodes.push_back(node);
}

int
Rack::GetNNodes() {
	return m_nodes.size();
}

ns3::NodeContainer
Rack::GetRackRouters() {
	return m_router_nodes;
}

void
Rack::ConnectNodesToRackRouters() {
	for(uint i = 0; i < (Api::m_use_p2p ? m_router_nodes.GetN() : 1); i++) {
		Ptr<ns3::Node> rnode = m_router_nodes.Get(i);
		for(Node* node : m_nodes) {
			// std::cout << m_rack_idx << " conn to switch " << i << "\n";
			node->ConnectToSwitch(rnode, i);
		}
	}
}

}
