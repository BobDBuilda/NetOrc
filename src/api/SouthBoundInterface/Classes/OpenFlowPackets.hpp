#ifndef OPENFLOW_PACKETS_HPP
#define OPENFLOW_PACKETS_HPP

#include <stdint.h>
#include <iostream>


//as a validator, should it validate incoming data that every packet is going to have
//or should there be a validator for each packet type?

struct OpenFlowHeader {
    uint8_t VERSION;
    uint8_t TYPE;
    uint16_t LENGTH;
    uint32_t XID;
};

enum class OFPacketType : uint8_t {
    /* --- Symmetric Messages (Either Direction) --- */
    HELLO = 0,          // Connection setup; exchange version support
    ERROR = 1,          // Notify of problematic requests or state
    ECHO_REQUEST = 2,   // Keep-alive and latency measurement
    ECHO_REPLY = 3,     // Response to Echo Request
    VENDOR = 4,         // Experimenter/Extension mechanism

    /* --- Controller-to-Switch Messages --- */
    FEATURES_REQUEST = 5,   // Ask switch for its capabilities
    FEATURES_REPLY = 6,     // Switch response with capabilities (DPID, ports, etc)
    GET_CONFIG_REQUEST = 7, // Ask switch for its current configuration
    GET_CONFIG_REPLY = 8,   // Switch response with config flags/miss-send length
    SET_CONFIG = 9,         // Set switch configuration parameters
    PACKET_OUT = 13,        // Controller sends packet to be emitted from switch port
    FLOW_MOD = 14,          // Controller adds/modifies/deletes flow table entries
    PORT_MOD = 15,          // Controller modifies port state (up/down)

    /* --- Asynchronous Messages (Switch-to-Controller) --- */
    PACKET_IN = 10,     // Switch sends packet it couldn't process to controller
    FLOW_REMOVED = 11,  // Switch notifies controller that a flow expired or was deleted
    PORT_STATUS = 12,   // Switch notifies controller of port changes (e.g., link up/down)

    /* --- Statistics & Barrier Messages --- */
    STATS_REQUEST = 16,     // Controller requests switch stats (flow, port, queue)
    STATS_REPLY = 17,       // Switch response with statistics
    BARRIER_REQUEST = 18,   // Controller ensures all previous messages were processed
    BARRIER_REPLY = 19,     // Switch confirmation of barrier completion
    QUEUE_GET_CONFIG_REQUEST = 20, // Request queue configuration
    QUEUE_GET_CONFIG_REPLY = 21    // Response with queue configuration
};

// struct OFPacket {
//     virtual void process() = 0;
//     virtual ~OFPacket() = default;
// };

// struct OFPacketHello : OFPacket {
//     void process() override {
//         std::cout << "Processing HELLO packet" << std::endl;
//     }
// };

// struct OFPacketIn : OFPacket {
//     void process() override {
//         // look up in the flow table?
//         // int bytesSent = send();
//         std::cout << "Processing PACKET_IN packet" << std::endl;
//     }
// };

// OpenFlowHeader hdr;
// hdr.type = static_cast<uint8_t>(OFPacketType::HELLO);

#endif // OPENFLOW_PACKETS_HPP
