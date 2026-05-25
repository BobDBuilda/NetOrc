#ifndef SERVICE_CONTAINER_HPP
#define SERVICE_CONTAINER_HPP

#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <string>

/**
 * ServiceContainer: A generic registry for application services.
 * Mimics the behavior of .NET's IServiceProvider.
 */
class ServiceContainer {
private:
    std::unordered_map<std::type_index, void*> services;

public:
    ServiceContainer() = default;

    // Register a service by its type
    template <typename T>
    void addService(T* service) {
        services[std::type_index(typeid(T))] = static_cast<void*>(service);
    }

    // Resolve a service by its type
    template <typename T>
    T& getService() {
        auto it = services.find(std::type_index(typeid(T)));
        if (it == services.end()) {
            throw std::runtime_error("Service not found: " + std::string(typeid(T).name()));
        }
        return *static_cast<T*>(it->second);
    }
};

#endif // SERVICE_CONTAINER_HPP
