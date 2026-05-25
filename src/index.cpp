#include <iostream>
#include <string>
#include <vector>
#include "core/Classes/App.h"
#include "core/Classes/Config.hpp"
#include "core/Classes/ServiceContainer.hpp"
#include "core/Classes/ThreadEnvironment.hpp"
#include "core/Classes/TaskQueue.hpp"
#include "core/Classes/FlowManager.hpp"
#include "core/Classes/DatabaseService.hpp"
#include "core/Classes/TopologyManager.hpp"
#include "api/NorthBoundInterface/NorthBoundInterface.hpp"
#include "api/SouthBoundInterface/Classes/SBI.hpp"

int main(int argc, char* argv[]) {
    Configuration config;

    // Simple command line argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            config.debug = true;
        } else if (arg == "--verbose") {
            config.verbose = true;
        }
    }

    config.printStatus();

    //Create the Service Container (The "Builder" LIKE IN .net)
    ServiceContainer services;

    //Register Configuration first so other services can access it if needed
    services.addService<Configuration>(&config);

    //Instantiate Services
    NorthBoundInterface nbi; //allocate space on the stack
    SouthBoundInterface sbi; //allocate space on the stack
    ThreadEnvironment thread_env(5); // Pool of 5 worker threads
    TaskQueue<NetworkEvent> eventQueue;
    FlowManager flowManager;
    DatabaseService dbService;
    TopologyManager topoManager;

    //Register Services (Similar to builder.Services in .NET)
    services.addService<NorthBoundInterface>(&nbi);
    services.addService<SouthBoundInterface>(&sbi);
    services.addService<ThreadEnvironment>(&thread_env);
    services.addService<TaskQueue<NetworkEvent>>(&eventQueue);
    services.addService<IFlowManager>(&flowManager);
    services.addService<FlowManager>(&flowManager);
    services.addService<IDatabaseService>(&dbService);
    services.addService<DatabaseService>(&dbService);
    services.addService<ITopologyManager>(&topoManager);

    //Create App and Inject the Container
    App app(services); //allocates on the stack.
    //these will be long-lived since the main scope
    //is the entire application lifetime, so s
    //stack allocation is fine here.

    app.run();

    return 0;
}
