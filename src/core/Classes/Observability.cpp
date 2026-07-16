#include "Observability.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace {
const char* toLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "debug";
        case LogLevel::INFO: return "info";
        case LogLevel::WARN: return "warn";
        case LogLevel::ERROR: return "error";
    }
    return "info";
}

std::string nowIso8601Utc() {
    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::string escapeJson(std::string value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (const char c : value) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}
} // namespace

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setServiceName(const std::string& name) {
    serviceName_ = name.empty() ? "netorc" : name;
}

void Logger::setDebugEnabled(const bool enabled) {
    debugEnabled_ = enabled;
}

void Logger::log(LogLevel level,
                 const std::string& message,
                 const std::map<std::string, std::string>& fields) {
    if (level == LogLevel::DEBUG && !debugEnabled_) {
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex_);
    std::ostringstream oss;
    oss << "{"
        << "\"ts\":\"" << nowIso8601Utc() << "\","
        << "\"service\":\"" << escapeJson(serviceName_) << "\","
        << "\"level\":\"" << toLevelString(level) << "\","
        << "\"message\":\"" << escapeJson(message) << "\"";

    for (const auto& kv : fields) {
        oss << ",\"" << escapeJson(kv.first) << "\":\"" << escapeJson(kv.second) << "\"";
    }
    oss << "}";
    std::cout << oss.str() << std::endl;
}

MetricsRegistry& MetricsRegistry::instance() {
    static MetricsRegistry registry;
    return registry;
}

void MetricsRegistry::observeHttpRequest(const std::string& method,
                                         const std::string& route,
                                         const int statusCode) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    httpRequestsTotal_[HttpMetricKey{method, route, statusCode}]++;
}

void MetricsRegistry::incrementEventsQueued() {
    eventsQueued_.fetch_add(1, std::memory_order_relaxed);
}

void MetricsRegistry::incrementEventsProcessed() {
    eventsProcessed_.fetch_add(1, std::memory_order_relaxed);
}

void MetricsRegistry::incrementEventsInvalid() {
    eventsInvalid_.fetch_add(1, std::memory_order_relaxed);
}

void MetricsRegistry::setEventQueueDepth(const std::uint64_t depth) {
    eventQueueDepth_.store(depth, std::memory_order_relaxed);
}

std::string MetricsRegistry::renderPrometheus() const {
    std::ostringstream oss;
    oss << "# HELP netorc_http_requests_total Total HTTP requests handled by route/method/status\n";
    oss << "# TYPE netorc_http_requests_total counter\n";
    {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        for (const auto& item : httpRequestsTotal_) {
            oss << "netorc_http_requests_total{method=\"" << item.first.method
                << "\",route=\"" << item.first.route
                << "\",status=\"" << item.first.status << "\"} "
                << item.second << "\n";
        }
    }

    oss << "# HELP netorc_events_queued_total Total events queued from southbound and northbound paths\n";
    oss << "# TYPE netorc_events_queued_total counter\n";
    oss << "netorc_events_queued_total " << eventsQueued_.load(std::memory_order_relaxed) << "\n";

    oss << "# HELP netorc_events_processed_total Total events processed by main event loop\n";
    oss << "# TYPE netorc_events_processed_total counter\n";
    oss << "netorc_events_processed_total " << eventsProcessed_.load(std::memory_order_relaxed) << "\n";

    oss << "# HELP netorc_events_invalid_total Total events rejected due to invalid input\n";
    oss << "# TYPE netorc_events_invalid_total counter\n";
    oss << "netorc_events_invalid_total " << eventsInvalid_.load(std::memory_order_relaxed) << "\n";

    oss << "# HELP netorc_event_queue_depth Current event queue depth\n";
    oss << "# TYPE netorc_event_queue_depth gauge\n";
    oss << "netorc_event_queue_depth " << eventQueueDepth_.load(std::memory_order_relaxed) << "\n";
    return oss.str();
}

std::string makeRequestId() {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<std::uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(rng);
    return oss.str();
}

std::string sanitizeRouteLabel(const std::string& path) {
    if (path.empty()) return "unknown";
    if (path == "/") return "root";
    if (path == "/health" || path == "/metrics" || path == "/config" || path == "/events") return path;
    return "other";
}
