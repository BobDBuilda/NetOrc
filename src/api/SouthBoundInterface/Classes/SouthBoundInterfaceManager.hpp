#ifndef SOUTHBOUND_INTERFACE_MANAGER_HPP
#define SOUTHBOUND_INTERFACE_MANAGER_HPP

#include <vector>
#include <stdint.h>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "ISouthBoundInterface.hpp"
#include "SouthBoundInterface.hpp"
#include "ThreadPool.hpp"

class SouthBoundInterfaceManager {
    // a vector of pointers to ISBI named connections
    std::vector<std::unique_ptr<ISouthBoundInterface>> connections;
    ThreadPool thread_pool = ThreadPool(5);
    int sock;

public: 
    SouthBoundInterfaceManager() : sock(-1) {
        // creates a socket endpoint
        // returns a socket descriptor
        this->sock = socket(AF_INET, SOCK_STREAM, 0);
        if (this->sock < 0) {
            perror("socket creation failed");
        }
    }

    ~SouthBoundInterfaceManager() {
        if (this->sock >= 0) {
            close(this->sock);
        }
    }

    void CreateThreadPool() {
        // why am i creating a thread pool?
        // is it because, I have to pass off a thread to every 
        // interface so that a socket can be created and not block
        // the main thread
    }
};

#endif // SOUTHBOUND_INTERFACE_MANAGER_HPP