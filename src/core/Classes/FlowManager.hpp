#ifndef FLOW_MANAGER_HPP
#define FLOW_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <stdint.h>
#include "DatabaseService.hpp"

/**
 * FlowEntry: Represents a single rule in a switch's flow table.
 * Based on OpenFlow 1.0 Match fields.
 */
struct FlowEntry {
    uint32_t in_port;
    std::string dl_src; // Source MAC
    std::string dl_dst; // Dest MAC
    std::string nw_src; // Source IP
    std::string nw_dst; // Dest IP
    uint16_t tp_src;    // Source Port (TCP/UDP)
    uint16_t tp_dst;    // Dest Port (TCP/UDP)
    
    uint16_t action_output_port; // Simplified action: send to port
    uint64_t packet_count = 0;
    uint64_t byte_count = 0;
};

/**
 * FlowManager: The "State Service".
 * Manages the in-memory representation of flows across all switches.
 */
class IFlowManager {
public:
    virtual ~IFlowManager() = default;
    virtual void addFlow(uint64_t dpid, const FlowEntry& entry, IDatabaseService& db) = 0;
    virtual std::vector<FlowEntry> getFlows(uint64_t dpid) = 0;
    virtual bool hasMatch(uint64_t dpid, uint32_t in_port) = 0;
};

class FlowManager : public IFlowManager {
private:
    // Key: Datapath ID (Switch ID) -> Value: List of FlowEntries
    std::unordered_map<uint64_t, std::vector<FlowEntry>> tables;
    std::mutex mtx;

public:
    /**
     * Add or update a flow for a specific switch and persist to DB.
     */
    void addFlow(uint64_t dpid, const FlowEntry& entry, IDatabaseService& db) override {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tables[dpid].push_back(entry);
        }
        
        // Persist to SQLite
        db.saveFlow(dpid, entry.in_port, entry.action_output_port);
    }

    /**
     * Get all flows for a specific switch.
     */
    std::vector<FlowEntry> getFlows(uint64_t dpid) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (tables.find(dpid) != tables.end()) {
            return tables[dpid];
        }
        return {};
    }

    /**
     * A simple "Match" simulator for the controller logic.
     */
    bool hasMatch(uint64_t dpid, uint32_t in_port) override {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = tables.find(dpid);
        if (it == tables.end()) return false;

        for (const auto& flow : it->second) {
            if (flow.in_port == in_port) return true;
        }
        return false;
    }
};

#endif // FLOW_MANAGER_HPP
