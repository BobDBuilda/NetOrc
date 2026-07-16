#ifndef OBSERVABILITY_HPP
#define OBSERVABILITY_HPP

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& instance();

    void setServiceName(const std::string& name);
    void setDebugEnabled(bool enabled);

    void log(LogLevel level,
             const std::string& message,
             const std::map<std::string, std::string>& fields = {});

private:
    Logger() = default;

    std::string serviceName_ = "netorc";
    bool debugEnabled_ = false;
    std::mutex logMutex_;
};

struct HttpMetricKey {
    std::string method;
    std::string route;
    int status = 0;

    bool operator<(const HttpMetricKey& other) const {
        if (method != other.method) return method < other.method;
        if (route != other.route) return route < other.route;
        return status < other.status;
    }
};

class MetricsRegistry {
public:
    static MetricsRegistry& instance();

    void observeHttpRequest(const std::string& method,
                            const std::string& route,
                            int statusCode);

    void incrementEventsQueued();
    void incrementEventsProcessed();
    void incrementEventsInvalid();
    void setEventQueueDepth(std::uint64_t depth);

    std::string renderPrometheus() const;

private:
    MetricsRegistry() = default;

    mutable std::mutex metricsMutex_;
    std::map<HttpMetricKey, std::uint64_t> httpRequestsTotal_;
    std::atomic<std::uint64_t> eventsQueued_{0};
    std::atomic<std::uint64_t> eventsProcessed_{0};
    std::atomic<std::uint64_t> eventsInvalid_{0};
    std::atomic<std::uint64_t> eventQueueDepth_{0};
};

std::string makeRequestId();
std::string sanitizeRouteLabel(const std::string& path);

#endif // OBSERVABILITY_HPP
