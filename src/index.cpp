#include <iostream>
#include <string>
#include <vector>
#include "core/Classes/App.h"
#include "core/Classes/Config.hpp"
#include "core/Classes/ServiceContainer.hpp"
#include "core/Classes/ThreadEnvironment.hpp"
#include "core/Classes/FlowManager.hpp"
#include "core/Classes/DatabaseService.hpp"
#include "core/Classes/TopologyManager.hpp"
#include "core/Classes/Observability.hpp"
#include "api/NorthBoundInterface/NorthBoundInterface.hpp"
#include "api/SouthBoundInterface/Classes/SBI.hpp"

int main(int argc, char* argv[]) {
    Configuration config;
    config.loadFromEnv();

    // Simple command line argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            config.debug = true;
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else if (arg.rfind("--service-name=", 0) == 0) {
            config.serviceName = arg.substr(std::string("--service-name=").size());
        } else if (arg.rfind("--nbi-port=", 0) == 0) {
            try {
                config.northboundPort = std::stoi(arg.substr(std::string("--nbi-port=").size()));
            } catch (...) {
                config.northboundPort = 8192;
            }
        }
    }

    Logger::instance().setServiceName(config.serviceName);
    Logger::instance().setDebugEnabled(config.debug || config.verbose);
    config.printStatus();
    Logger::instance().log(LogLevel::INFO, "NetOrc startup complete",
                           {{"service", config.serviceName},
                            {"northbound_port", std::to_string(config.northboundPort)}});

    //Create the Service Container (The "Builder" LIKE IN .net)
    ServiceContainer services;

    //Register Configuration first so other services can access it if needed
    services.addService<Configuration>(&config);

    //Instantiate Services
    NorthBoundInterface nbi; //allocate space on the stack
    SouthBoundInterface sbi; //allocate space on the stack
    ThreadEnvironment sbi_thread_env(5); // Pool for Southbound traffic
    ThreadEnvironment nbi_thread_env(2); // Dedicated pool for Northbound API
    FlowManager flowManager;
    DatabaseService dbService;
    TopologyManager topoManager;

    //Register Services (Similar to builder.Services in .NET)
    services.addService<NorthBoundInterface>(&nbi);
    services.addService<SouthBoundInterface>(&sbi);
    services.addService<ThreadEnvironment>(&sbi_thread_env, "SBI");
    services.addService<ThreadEnvironment>(&nbi_thread_env, "NBI");
    services.addService<FlowManager>(&flowManager);
    services.addService<DatabaseService>(&dbService);
    services.addService<TopologyManager>(&topoManager);
    //Create App and Inject the Container
    App app(services); //allocates on the stack.
    //these will be long-lived since the main scope
    //is the entire application lifetime, so s
    //stack allocation is fine here.

    app.run();

    return 0;
}
