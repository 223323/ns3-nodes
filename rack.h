#ifndef RACK_H
#define RACK_H

#include "node.h"

namespace Sim {

class Rack {
public:
	Rack();
	void AddNode(Node* node);
	int GetNNodes();
	ns3::NodeContainer GetRackRouters();
	void ConnectNodesToRackRouters();
private:
	std::vector<Node*> 		  	  m_nodes;
	ns3::NodeContainer  		  m_router_nodes;
	int 						  m_rack_idx;
	static int 					  m_rack_count;
	
	// std::string					m_rack_link
};

}

#endif
