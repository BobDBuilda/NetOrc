#include <stdint.h>
#include <iostream>
#include <functional>
#include <string.h>

class App{
private:
    std::string SouthboundInterface;
    std::string NorthboundInterface;
public:
    void get(std::string type, std::function<int(int)> callback){}
};

// void App::get(std::string type, std::function<int(int)> callback){
//     std::cout << "Hello" << std::endl;

//     if(type == "PACKET_IN"){
//         std::cout << "PACKET_IN event recorded" << std::endl;
//     }
// }