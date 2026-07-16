#ifndef PACKET_HANDLERS_HPP
#define PACKET_HANDLERS_HPP

#include <map>
#include <functional>
#include <vector>
#include <iostream>
#include "../../../core/Classes/FlowManager.hpp"
#include "../../../core/Classes/DatabaseService.hpp"
#include "../../../core/Classes/TopologyManager.hpp"
#include "../OpenFlowLib.h"
#include "../Utils/LLDPParser.hpp"

// Forward declaration to break circular dependency
class SouthBoundInterface;

/**
 * PacketHandler: A functional type for dispatching OpenFlow messages.
 * Matches the "Handler Map" pattern.
 * Now includes a reference to SouthBoundInterface (sbi) and ITopologyManager to enable reactive sending and topology discovery.
 */
using PacketHandler = std::function<void(int fd, const std::vector<uint8_t>& payload, IFlowManager& fm, IDatabaseService& db, SouthBoundInterface& sbi, ITopologyManager& topo)>;

/**
 * getPacketHandlers: Returns the mapping of oflib::PacketTypes to their respective logic.
 */
inline std::map<oflib::PacketType, PacketHandler> getPacketHandlers() {
    return {
        { oflib::PacketType::HELLO, [](int fd, const auto& payload, auto& fm, auto& db, auto& sbi, auto& topo) {
            std::cout << "  -> [Handler] HELLO: Initializing Flow Table for new connection." << std::endl;
            
            FlowEntry defaultFlow;
            defaultFlow.in_port = 1;
            defaultFlow.action_output_port = 2;
            fm.addFlow(fd, defaultFlow, db);
        }},
        { oflib::PacketType::PACKET_IN, [](int fd, const auto& payload, auto& fm, auto& db, auto& sbi, auto& topo) {
            std::cout << "  -> [Handler] PACKET_IN: Analyzing payload." << std::endl;
            
            if (LLDPParser::isLLDP(payload)) {
                std::cout << "     -> [Discovery] LLDP Packet Detected!" << std::endl;
                LLDP_Neighbor neighbor = LLDPParser::parse(payload);
                
                // In a real OpenFlow PACKET_IN, the ingress port is in the header, 
                // but here we'll mock it as "Port 1" for demonstration.
                topo.updateLink(fd, "1", neighbor.chassis_id, neighbor.port_id);
                topo.printTopology();
            } else {
                if (fm.hasMatch(fd, 1)) {
                    std::cout << "     -> Flow hit! Routing to port 2." << std::endl;
                } else {
                    std::cout << "     -> Flow miss! Sending to controller logic." << std::endl;
                }
            }
        }},
        { oflib::PacketType::FLOW_MOD, [](int fd, const auto& payload, auto& fm, auto& db, auto& sbi, auto& topo) {
            std::cout << "  -> [Handler] FLOW_MOD: Received request. Updating flow table..." << std::endl;

            FlowEntry newFlow;
            newFlow.in_port = 10; 
            newFlow.action_output_port = 20;

            fm.addFlow(fd, newFlow, db);
            std::cout << "     -> Flow added to FD: " << fd << std::endl;
        }}
    };
}

#endif // PACKET_HANDLERS_HPP
