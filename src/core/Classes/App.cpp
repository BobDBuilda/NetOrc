#include "App.h"
#include "Config.hpp"
#include "api/NorthBoundInterface/NorthBoundInterface.hpp"
#include "api/SouthBoundInterface/Classes/SBI.hpp"
#include "ThreadEnvironment.hpp"
#include "TaskQueue.hpp"
#include "FlowManager.hpp"
#include "DatabaseService.hpp"
#include "PacketHandlers.hpp"
#include "TopologyManager.hpp"

void App::run() {
    auto& config = services.getService<Configuration>();
    
    if (config.verbose) {
        Logger::instance().log(LogLevel::INFO, "App is running with service-based architecture (No Event Loop required)");
    }

    try {
        auto& sbi = services.getService<SouthBoundInterface>();
        auto& nbi = services.getService<NorthBoundInterface>();
        auto& sbi_tm = services.getService<ThreadEnvironment>("SBI");
        auto& nbi_tm = services.getService<ThreadEnvironment>("NBI");
        auto& flowManager = services.getService<FlowManager>();
        auto& dbService = services.getService<DatabaseService>();
        auto& topoManager = services.getService<TopologyManager>();

        sbi.init(sbi_tm, config, flowManager, dbService, topoManager);
        nbi.init(nbi_tm, config);

        // Keep the main thread alive/idle
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, "Fatal app error", {{"error", e.what()}});
    }
}
