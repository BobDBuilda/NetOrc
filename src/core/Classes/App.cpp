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

void App::processEvent(const NetworkEvent& event, SouthBoundInterface& sbi, ITopologyManager& topo) {
    auto& flowManager = services.getService<IFlowManager>();
    auto& db = services.getService<IDatabaseService>();
    auto& config = services.getService<Configuration>();
    auto handlers = getPacketHandlers();

    if (config.verbose) {
        std::cout << "[Event Loop] Processing " << static_cast<int>(event.type)
                  << " from FD: " << event.client_fd
                  << " (Payload Size: " << event.payload.size() << ")" << std::endl;
    }

    auto handlerFound = handlers.find(event.type);
    if (handlerFound != handlers.end()) {
        handlerFound->second(event.client_fd, event.payload, flowManager, db, sbi, topo);
    } else {
        if (config.debug) {
            std::cout << "  -> [Warning] No handler registered for packet type: "
                      << static_cast<int>(event.type) << std::endl;
        }
    }
}

void App::run() {
    auto& config = services.getService<Configuration>();
    
    if (config.verbose) {
        std::cout << "App is running with Service-Based Architecture..." << std::endl;
    }

    try {
        auto& sbi = services.getService<SouthBoundInterface>();
        //auto& nbi = services.getService<NorthBoundInterface>();
        auto& tm = services.getService<ThreadEnvironment>();
        auto& queue = services.getService<TaskQueue<NetworkEvent>>();
        auto& topo = services.getService<ITopologyManager>();

        sbi.init(tm, queue, config);
        //nbi.init(tm, queue);

        if (config.verbose) {
            std::cout << "Starting Main Event Loop..." << std::endl;
        }
        while (true) {
            NetworkEvent event{};//initialized on the stack.
            //pop off the TaskQueue
            queue.wait_and_pop(event);//wait and pop is defined to take a reference.
            //remember a 'pop' in itself does not take an argument, it just
            //removes from the top of a queue, but 'wait_and_pop' is defined to take 
            //a reference and populate it with the data from the front of the queue.
            //
            //thus event gets populated with 
            // the data from the queue, and we can process it.
            processEvent(event, sbi, topo);
            //should dispatching go here?
            //should sending logic go here?
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal App Error: " << e.what() << std::endl;
    }
}
