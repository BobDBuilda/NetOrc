#include <vector>
#include <stdint.h>
#include <memory>
#include "ISouthBoundInterface.hpp"
#include "SouthBoundInterface.hpp"

class SouthBoundInterfaceManager{\
    //a vector of pointers to ISBI named connections
    std::vector<std::unique_ptr<ISouthBoundInterface>> connections;

public: 
    void addConnection(std::string ip){
        connections.push_back(std::make_unique<SouthBoundInterface>());

        connections.back()->connect(ip, 300);
    }

    void pollAll(){
        char buffer[4096];
        for (auto& conn : connections){
            conn->receiveData(buffer, sizeof(buffer));
        }

    }
};