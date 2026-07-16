#include "NorthBoundInterface.hpp"

void NorthBoundInterface::registerRoutes(httplib::Server& server,
                                        ThreadEnvironment& threadEnvironment) {
    // Serve static files from the SPA directory.
    if (!server.set_mount_point("/", "./web/dist")) {
        Logger::instance().log(LogLevel::WARN, "Failed to mount static files", {{"path", "./web/dist"}});
    }

    server.set_post_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        MetricsRegistry::instance().observeHttpRequest(req.method, sanitizeRouteLabel(req.path), res.status);
    });

    // Liveness endpoint.
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    server.Get("/metrics", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(MetricsRegistry::instance().renderPrometheus(), "text/plain; version=0.0.4");
    });

    // Introspection endpoint.
    server.Get("/config", [this](const httplib::Request&, httplib::Response& res) {
        const bool debug = config_ ? config_->debug : false;
        const bool verbose = config_ ? config_->verbose : false;
        res.set_content(
            "{\"debug\":" + std::string(debug ? "true" : "false") +
            ",\"verbose\":" + std::string(verbose ? "true" : "false") + "}",
            "application/json");
    });

    // Event ingest endpoint:
    server.Post("/events", [this, &threadEnvironment](const httplib::Request& req, httplib::Response& res) {
        oflib::PacketType type = oflib::PacketType::VENDOR;
        std::string requestId = req.get_header_value("X-Request-Id");
        if (requestId.empty()) {
            requestId = makeRequestId();
        }
        res.set_header("X-Request-Id", requestId);
        
        if (req.has_param("type")) {
            const auto param = req.get_param_value("type");
            try {
                int typeNum = std::stoi(param);
                if (typeNum < 0 || typeNum > 255) throw std::out_of_range("type out of range");
                type = static_cast<oflib::PacketType>(typeNum);
            } catch (...) {
                res.status = 400;
                res.set_content("{\"error\":\"invalid type query parameter\"}", "application/json");
                return;
            }
        }

        // Logic now handled directly or via service calls
        threadEnvironment.enqueue([type, body = req.body]() {
            // Processing logic goes here
        });

        res.status = 202;
        res.set_content("{\"queued\":true}", "application/json");
    });
}
