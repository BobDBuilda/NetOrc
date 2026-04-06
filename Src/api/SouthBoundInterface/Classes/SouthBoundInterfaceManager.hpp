#include <vector>
#include <stdint.h>
#include <memory>
#include "ISouthBoundInterface.hpp"
#include "SouthBoundInterface.hpp"
#include "ThreadPool.hpp"

#if _WIN32_
    typedef SOCKET socket_t;
#else
    typedef uint32_t socket_t;
#endif

class SouthBoundInterfaceManager{\
    //a vector of pointers to ISBI named connections
    std::vector<std::unique_ptr<ISouthBoundInterface>> connections;
    ThreadPool thread_pool = ThreadPool(5);
    SOCKET sock;
public: 
    SouthBoundInterfaceManager(){
        try{
            //creates a socket endpoint
            //returns a socket descriptor
            this->sock = socket(AF_INET, SOCK_STREAM, 0);
        }catch(uint8_t e){

        }
        
    }

    void CreateThreadPool(){
        //why am i creating a thread pool?
        //is it because, I have to pass off a thread to every 
        //interface so that a socket can be created and not block
        //the main thread

    }


};