#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdlib>
#include <iostream>
#include <string>

struct Configuration {
    bool debug = false;
    bool verbose = false;
    std::string serviceName = "netorc";
    int northboundPort = 8192;

    void loadFromEnv() {
        if (const char* val = std::getenv("NETORC_SERVICE_NAME")) {
            serviceName = val;
        }
        if (const char* val = std::getenv("NETORC_NBI_PORT")) {
            try {
                northboundPort = std::stoi(val);
            } catch (...) {
                northboundPort = 8192;
            }
        }
    }

    void printStatus() const {
        if (debug) {
            std::cout << "[Config] Debug mode: ENABLED" << std::endl;
        }
        if (verbose) {
            std::cout << "[Config] Verbose logging: ENABLED" << std::endl;
        }
        std::cout << "[Config] Service name: " << serviceName << std::endl;
        std::cout << "[Config] NBI port: " << northboundPort << std::endl;
    }
};

#endif // CONFIG_HPP
