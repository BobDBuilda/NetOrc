#include <cstdlib>
#include <iostream>
#include <vector>

#include "core/Classes/App.h"

namespace {

int g_failed = 0;

void expectTrue(bool value, const char* name) {
    if (!value) {
        std::cerr << "[FAIL] " << name << std::endl;
        ++g_failed;
    } else {
        std::cout << "[PASS] " << name << std::endl;
    }
}

struct FakeDatabaseService : IDatabaseService {
    int save_calls = 0;
    uint64_t last_dpid = 0;
    uint32_t last_in_port = 0;
    int last_action_port = 0;

    void saveFlow(uint64_t dpid, uint32_t in_port, int action_port) override {
        ++save_calls;
        last_dpid = dpid;
        last_in_port = in_port;
        last_action_port = action_port;
    }
};

struct FakeFlowManager : IFlowManager {
    int add_calls = 0;
    uint64_t last_dpid = 0;
    FlowEntry last_entry{};
    IDatabaseService* last_db = nullptr;

    void addFlow(uint64_t dpid, const FlowEntry& entry, IDatabaseService& db) override {
        ++add_calls;
        last_dpid = dpid;
        last_entry = entry;
        last_db = &db;
        db.saveFlow(dpid, entry.in_port, entry.action_output_port);
    }

    std::vector<FlowEntry> getFlows(uint64_t) override {
        return {};
    }

    bool hasMatch(uint64_t, uint32_t) override {
        return false;
    }
};

struct FakeTopologyManager : ITopologyManager {
    int update_calls = 0;
    void updateLink(int fd, const std::string& p1, const std::string& c, const std::string& p2) override {
        update_calls++;
    }
    void printTopology() override {}
};

} // namespace

int main() {
    ServiceContainer services;
    FakeFlowManager flowManager;
    FakeDatabaseService db;
    SouthBoundInterface sbi;
    Configuration config;
    FakeTopologyManager topo;
    config.verbose = false;
    config.debug = false;

    services.addService<IFlowManager>(&flowManager);
    services.addService<IDatabaseService>(&db);
    services.addService<Configuration>(&config);
    services.addService<ITopologyManager>(&topo);

    App app(services);

    NetworkEvent event{};
    event.client_fd = 42;
    event.type = OFPacketType::HELLO;
    event.payload = {};

    app.processEvent(event, sbi, topo);

    expectTrue(flowManager.add_calls == 1, "HELLO handler adds a flow once");
    expectTrue(db.save_calls == 1, "HELLO handler persists the flow once");
    expectTrue(flowManager.last_dpid == 42, "Handler uses the event fd as the switch key");
    expectTrue(flowManager.last_entry.in_port == 1, "HELLO handler sets in_port to 1");
    expectTrue(flowManager.last_entry.action_output_port == 2, "HELLO handler sets action port to 2");
    expectTrue(db.last_dpid == 42, "DB sees the same datapath id");
    expectTrue(db.last_in_port == 1, "DB receives the flow in_port");
    expectTrue(db.last_action_port == 2, "DB receives the flow action port");
    expectTrue(flowManager.last_db == &db, "Flow manager receives the injected DB double");

    if (g_failed != 0) {
        std::cerr << g_failed << " test(s) failed" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "App integration test passed" << std::endl;
    return EXIT_SUCCESS;
}
