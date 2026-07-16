#ifndef SOUTHBOUND_INTERFACE_HPP
#define SOUTHBOUND_INTERFACE_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "../OpenFlowLib.h"
#include "../../../core/Classes/ThreadEnvironment.hpp"
#include "../../../core/Classes/TaskQueue.hpp"
#include "../../../core/Classes/Config.hpp"

#include "PacketHandlers.hpp"

/**
 * OpenFlowMessage: The "Result Object".
 * This is what the worker thread produces and places into the Event Loop's queue.
 */
struct OpenFlowMessage {
    int client_fd;
    oflib::PacketType type;
    std::vector<uint8_t> payload; // The raw body of the OpenFlow message
};

// Alias for a function type that returns a boolean indicating if the packet is valid.
using ValidatorFn = std::function<bool(const uint8_t*, size_t)>;

extern std::unordered_map<oflib::PacketType, ValidatorFn> packetValidators;

/**
 * SouthBoundInterface: Wrapper around oflib::Server to integrate with the NetOrc core.
 */
class SouthBoundInterface {
private:
    Configuration* config = nullptr;
    std::unique_ptr<oflib::Server> server_;

public:
    void init(ThreadEnvironment& threadEnvironment, Configuration& cfg, 
              IFlowManager& fm, IDatabaseService& db, ITopologyManager& topo) {
        this->config = &cfg;
        
        // Initialize the new oflib::Server with 4 worker threads
        server_ = std::make_unique<oflib::Server>(4);

        // Organize handlers similar to NorthBoundInterface::registerRoutes
        setupProtocolEvents(fm, db, topo);

        // Register validators (if any)
        for (auto& pair : packetValidators) {
            server_->validate(pair.first, pair.second);
        }

        if (config->verbose) {
            std::cout << "Southbound Interface (OpenFlowLib) starting on port 20045..." << std::endl;
        }

        if (!server_->listen("0.0.0.0", 20045)) {
            std::cerr << "SBI failed to start on port 20045" << std::endl;
        }
    }

    /**
     * send_packet: Encapsulates sending OpenFlow-compliant messages to a switch.
     */
    void send_packet(int sock, oflib::PacketType type, uint32_t xid, const std::vector<uint8_t>& payload = {}) {
        if (server_) {
            server_->send_frame(sock, type, xid, payload);
        }
    }

private:
    /**
     * setupProtocolEvents: Defines the "routes" for incoming OpenFlow packets.
     */
    void setupProtocolEvents(IFlowManager& fm, IDatabaseService& db, ITopologyManager& topo) {
        auto handlers = getPacketHandlers();

        // Register handlers using the declarative API directly
        server_->hello([this, &fm, &db, &topo](auto& conn, auto& frame) {
            auto handler = getPacketHandlers()[oflib::PacketType::HELLO];
            handler(conn.fd, frame.body, fm, db, *this, topo);
        });

        server_->packet_in([this, &fm, &db, &topo](auto& conn, auto& frame) {
            auto handler = getPacketHandlers()[oflib::PacketType::PACKET_IN];
            handler(conn.fd, frame.body, fm, db, *this, topo);
        });

        server_->flow_removed([this, &fm, &db, &topo](auto& conn, auto& frame) {
            // Placeholder: Could add direct handler for FLOW_REMOVED if needed
        });
    }
};

#endif // SOUTHBOUND_INTERFACE_HPP
