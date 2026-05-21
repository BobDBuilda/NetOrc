#include <stdint.h>
#include <iostream>
#include <functional>
#include <string.h>

#include "api/NorthBoundInterface/NorthBoundInterface.hpp"
#include "api/SouthBoundInterface/Classes/SouthBoundInterface.hpp"

class App{
private:
//store as references since app is not responsible for their lifecycle. 
// they are created outside and passed in.
    NorthBoundInterface& nbi;
    SouthBoundInterface& sbi;

public:
    App(NorthBoundInterface& nbi, SouthBoundInterface& sbi)
        : nbi(nbi), sbi(sbi) {
        std::cout << "App created" << std::endl;
    }

    void run();
};
