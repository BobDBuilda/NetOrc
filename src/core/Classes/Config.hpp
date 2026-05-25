#ifndef CONFIG_HPP
#define CONFIG_HPP

struct Configuration {
    bool debug = false;
    bool verbose = false;

    void printStatus() const {
        if (debug) {
            std::cout << "[Config] Debug mode: ENABLED" << std::endl;
        }
        if (verbose) {
            std::cout << "[Config] Verbose logging: ENABLED" << std::endl;
        }
    }
};

#endif // CONFIG_HPP