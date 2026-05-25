#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <iostream>
#include <functional>
#include <string.h>

#include "ServiceContainer.hpp"
#include "TaskQueue.hpp"
#include "FlowManager.hpp"
#include "DatabaseService.hpp"
#include "PacketHandlers.hpp"
#include "TopologyManager.hpp"
#include "api/SouthBoundInterface/Classes/SBI.hpp"
#include "ThreadEnvironment.hpp"

 //App: The main application class.
 //Now uses a ServiceContainer (similar to .NET WebBuilder) to handle
 //arbitrary dependencies without fixed constructor signatures.
 
class App {
private:
    ServiceContainer& services;

public:
    //because this is defined as taking a reference as an argument
    //when we pass in the service container, it will be passed by reference, 
    // and the app will hold a reference to the same container 
    // that was created in main.
    App(ServiceContainer& container) : services(container) {
        std::cout << "App initialized with Service Container" << std::endl;
    }

    // Example of how App would access its services internally
    void processEvent(const NetworkEvent& event, SouthBoundInterface& sbi, ITopologyManager& topo);
    void run();
};
   
#endif // APP_H
