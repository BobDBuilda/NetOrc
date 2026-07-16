#ifndef NORTHBOUND_INTERFACE_HPP
#define NORTHBOUND_INTERFACE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include "httplib.h"
#include "../../core/Classes/ThreadEnvironment.hpp"
#include "../../core/Classes/TaskQueue.hpp"
#include "../../core/Classes/Config.hpp"
#include "../../core/Classes/Observability.hpp"
#include "../SouthBoundInterface/Classes/SBI.hpp" // For OpenFlowMessage

class NorthBoundInterface {
public:
    void init(ThreadEnvironment& threadEnvironment, Configuration& config) {
        config_ = &config;

        std::thread([this, &threadEnvironment]() {
            // httplib manages the underlying TCP socket lifecycle (bind/listen/accept)
            // and gives us route handlers similar to Express.
            httplib::Server server;
            registerRoutes(server, threadEnvironment);
            const int port = (config_ && config_->northboundPort > 0) ? config_->northboundPort : kDefaultPort;

            if (config_ && config_->verbose) {
                Logger::instance().log(LogLevel::INFO, "Northbound interface starting",
                                       {{"port", std::to_string(port)}});
            }

            if (!server.listen("0.0.0.0", port)) {
                Logger::instance().log(LogLevel::ERROR, "NBI failed to bind/listen",
                                       {{"port", std::to_string(port)}});
            }
        }).detach();
    }

private:
    static constexpr int kDefaultPort = 8192;
    Configuration* config_ = nullptr;

    void registerRoutes(httplib::Server& server,
                        ThreadEnvironment& threadEnvironment);
};

#endif // NORTHBOUND_INTERFACE_HPP
