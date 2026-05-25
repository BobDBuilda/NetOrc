#ifndef TOPOLOGY_MANAGER_HPP
#define TOPOLOGY_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>

struct Link {
    int src_fd;
    std::string src_port;
    std::string dst_chassis_id;
    std::string dst_port;
};

class ITopologyManager {
public:
    virtual ~ITopologyManager() = default;
    virtual void updateLink(int src_fd, const std::string& src_port, const std::string& dst_chassis, const std::string& dst_port) = 0;
    virtual void printTopology() = 0;
};

class TopologyManager : public ITopologyManager {
private:
    std::vector<Link> links;
    std::mutex mtx;

public:
    void updateLink(int src_fd, const std::string& src_port, const std::string& dst_chassis, const std::string& dst_port) override {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Check if link already exists
        for (auto& link : links) {
            if (link.src_fd == src_fd && link.src_port == src_port) {
                link.dst_chassis_id = dst_chassis;
                link.dst_port = dst_port;
                return;
            }
        }

        links.push_back({src_fd, src_port, dst_chassis, dst_port});
        std::cout << "[Topology] New Link Discovered: Switch (FD:" << src_fd << ", Port:" << src_port 
                  << ") -> Remote Chassis:" << dst_chassis << " Port:" << dst_port << std::endl;
    }

    void printTopology() override {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "\n--- Network Topology ---" << std::endl;
        for (const auto& link : links) {
            std::cout << "Switch FD:" << link.src_fd << " [Port " << link.src_port << "] <---> "
                      << "Chassis:" << link.dst_chassis_id << " [Port " << link.dst_port << "]" << std::endl;
        }
        std::cout << "------------------------\n" << std::endl;
    }
};

#endif // TOPOLOGY_MANAGER_HPP
