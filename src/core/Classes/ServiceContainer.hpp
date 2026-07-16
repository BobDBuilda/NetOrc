#ifndef SERVICE_CONTAINER_HPP
#define SERVICE_CONTAINER_HPP

#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <string>

/**
 * ServiceContainer: A generic registry for application services.
 * Mimics the behavior of .NET's IServiceProvider.
 * Supports named services to allow multiple instances of the same type.
 */
class ServiceContainer {
private:
    struct ServiceKey {
        std::type_index type;
        std::string name;

        bool operator==(const ServiceKey& other) const {
            return type == other.type && name == other.name;
        }
    };

    struct ServiceKeyHasher {
        std::size_t operator()(const ServiceKey& key) const {
            return std::hash<std::type_index>{}(key.type) ^ std::hash<std::string>{}(key.name);
        }
    };

    std::unordered_map<ServiceKey, void*, ServiceKeyHasher> services;

public:
    ServiceContainer() = default;

    // Register a service by its type and an optional name
    template <typename T>
    void addService(T* service, const std::string& name = "") {
        services[{std::type_index(typeid(T)), name}] = static_cast<void*>(service);
    }

    // Resolve a service by its type and an optional name
    template <typename T>
    T& getService(const std::string& name = "") {
        auto it = services.find({std::type_index(typeid(T)), name});
        if (it == services.end()) {
            throw std::runtime_error("Service not found: " + std::string(typeid(T).name()) + (name.empty() ? "" : " (" + name + ")"));
        }
        return *static_cast<T*>(it->second);
    }
};

#endif // SERVICE_CONTAINER_HPP
