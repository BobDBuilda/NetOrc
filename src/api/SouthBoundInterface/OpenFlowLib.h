#ifndef NETORC_OPENFLOW_LIB_H
#define NETORC_OPENFLOW_LIB_H

#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>

/**
 * oflib: A lightweight library for OpenFlow 1.0 wire protocol management.
 * 
 * This library provides the foundational primitives for communicating with OpenFlow-enabled
 * network switches. It encapsulates the binary framing logic, type-safe packet definitions,
 * and a basic state machine for managing the OpenFlow handshake lifecycle.
 */
namespace oflib {

// OpenFlow 1.0 wire constants as defined in the official specification.
constexpr uint8_t kOFVersion10 = 0x01;      // Protocol version 1.0
constexpr size_t kOFHeaderLen = 8;          // Fixed header size (8 bytes)
constexpr uint16_t kOFMaxFrameLen = 65535;  // Max size allowed by the 16-bit length field

/**
 * PacketType: Maps to the native OpenFlow byte that is named 'type' in the header.
 */
enum class PacketType : uint8_t {
    /* --- Symmetric Messages (Either Direction) --- */
    HELLO = 0,          // Connection setup; exchange version support
    ERROR = 1,          // Notify of problematic requests or state
    ECHO_REQUEST = 2,   // Keep-alive and latency measurement
    ECHO_REPLY = 3,     // Response to Echo Request
    VENDOR = 4,         // Experimenter/Extension mechanism

    /* --- Controller-to-Switch Messages --- */
    FEATURES_REQUEST = 5,   // Ask switch for its capabilities
    FEATURES_REPLY = 6,     // Switch response with capabilities (DPID, ports, etc)
    GET_CONFIG_REQUEST = 7, // Ask switch for its current configuration
    GET_CONFIG_REPLY = 8,   // Switch response with config flags/miss-send length
    SET_CONFIG = 9,         // Set switch configuration parameters
    PACKET_OUT = 13,        // Controller sends packet to be emitted from switch port
    FLOW_MOD = 14,          // Controller adds/modifies/deletes flow table entries
    PORT_MOD = 15,          // Controller modifies port state (up/down)

    /* --- Asynchronous Messages (Switch-to-Controller) --- */
    PACKET_IN = 10,     // Switch sends packet it couldn't process to controller
    FLOW_REMOVED = 11,  // Switch notifies controller that a flow expired or was deleted
    PORT_STATUS = 12,   // Switch notifies controller of port changes (e.g., link up/down)

    /* --- Statistics & Barrier Messages --- */
    STATS_REQUEST = 16,     // Controller requests switch stats (flow, port, queue)
    STATS_REPLY = 17,       // Switch response with statistics
    BARRIER_REQUEST = 18,   // Controller ensures all previous messages were processed
    BARRIER_REPLY = 19,     // Switch confirmation of barrier completion
    QUEUE_GET_CONFIG_REQUEST = 20, // Request queue configuration
    QUEUE_GET_CONFIG_REPLY = 21    // Response with queue configuration
};

/**
 * Header: The fixed 8-byte preamble of every OpenFlow packet.
 */
struct Header {
    uint8_t version{};
    uint8_t type{};
    uint16_t length{};    // Total length including header
    uint32_t xid{};       // Transaction ID for request/reply matching
};

// Compatibility alias for legacy validators
using OpenFlowHeader = Header;

/**
 * Frame: A complete OpenFlow message (Header + Optional Payload).
 */
struct Frame {
    Header header{};
    std::vector<uint8_t> body{};
};

/**
 * SessionState: Tracks the progress of the OpenFlow handshake.
 * Switches start at CONNECTED and must exchange HELLO and FEATURES before
 * becoming ACTIVE for general traffic.
 */
enum class SessionState {
    CONNECTED,
    HELLO_DONE,
    FEATURES_DONE,
    ACTIVE,
    CLOSING
};

/**
 * Connection: Represents a per-client persistent state for a switch connection.
 */
struct Connection {
    int fd = -1;                                // File descriptor for the TCP socket
    SessionState state = SessionState::CONNECTED;
    std::chrono::steady_clock::time_point last_activity = std::chrono::steady_clock::now();
    std::vector<uint8_t> inbuf{};               // Accumulation buffer for partial TCP reads
};

/**
 * ParseStatus: Internal signals for the Framer loop.
 */
enum class ParseStatus {
    NEED_MORE,  // Not enough bytes in buffer to form a complete frame
    OK,         // Frame successfully extracted
    INVALID     // Protocol error (e.g., bad version or length)
};

// Callback signatures for extensibility.
using ValidatorFn = std::function<bool(const uint8_t*, size_t)>;
using HandlerFn = std::function<void(Connection&, const Frame&)>;
using FilterFn = std::function<bool(const Frame&)>;

/**
 * Filter: A collection of standard predicates for "routing" OpenFlow packets.
 */
struct filter {
    static FilterFn all() { return [](const Frame&) { return true; }; }
    
    // Example: Match by EtherType (requires parsing body, but we provide the hook)
    static FilterFn ether_type(uint16_t type) {
        return [type](const Frame& f) {
            if (f.body.size() < 14) return false; // Basic Ethernet header check
            uint16_t et;
            std::memcpy(&et, f.body.data() + 12, 2);
            return ntohs(et) == type;
        };
    }
};

/**
 * Framer: Stateless logic for converting raw byte streams into discrete OpenFlow Frames.
 */
class Framer {
public:
    /**
     * Extracts fields from raw bytes, handling network-to-host byte order conversion.
     */
    static bool parse_header(const uint8_t* bytes, Header& out) {
        if (!bytes) {
            return false;
        }

        out.version = bytes[0];
        out.type = bytes[1];

        uint16_t n_len = 0;
        std::memcpy(&n_len, bytes + 2, sizeof(n_len));
        out.length = ntohs(n_len);

        uint32_t n_xid = 0;
        std::memcpy(&n_xid, bytes + 4, sizeof(n_xid));
        out.xid = ntohl(n_xid);

        return true;
    }

    /**
     * Attempts to "carve" the next complete frame out of the connection's input buffer.
     * Implements basic protocol validation (version checks, length bounds).
     */
    static ParseStatus next_frame(std::vector<uint8_t>& inbuf, Frame& out) {
        if (inbuf.size() < kOFHeaderLen) {
            return ParseStatus::NEED_MORE;
        }

        Header hdr{};
        if (!parse_header(inbuf.data(), hdr)) {
            return ParseStatus::INVALID;
        }

        // Enforce strict protocol adherence
        if (hdr.version != kOFVersion10 || hdr.length < kOFHeaderLen || hdr.length > kOFMaxFrameLen) {
            return ParseStatus::INVALID;
        }

        if (inbuf.size() < hdr.length) {
            return ParseStatus::NEED_MORE;
        }

        // Move data from buffer into Frame object
        out.header = hdr;
        const size_t body_len = static_cast<size_t>(hdr.length - kOFHeaderLen);
        out.body.assign(inbuf.begin() + static_cast<long>(kOFHeaderLen),
                        inbuf.begin() + static_cast<long>(hdr.length));

        // Consume the bytes from the head of the buffer
        inbuf.erase(inbuf.begin(), inbuf.begin() + static_cast<long>(hdr.length));
        if (out.body.size() != body_len) {
            return ParseStatus::INVALID;
        }

        return ParseStatus::OK;
    }
};

/**
 * OpenFlowEndpoint: The primary interface for sending and receiving OpenFlow messages.
 */
class OpenFlowEndpoint {
public:
    virtual ~OpenFlowEndpoint() = default;

    /**
     * Registers a callback for a specific packet type with an optional filter.
     */
    void on(PacketType type, HandlerFn handler, FilterFn f = filter::all()) {
        handlers_[type].push_back({std::move(f), std::move(handler)});
    }

    /**
     * Registers a security validator for a packet type to filter malicious or malformed payloads.
     */
    void validate(PacketType type, ValidatorFn validator) {
        validators_[type] = std::move(validator);
    }

    /**
     * Serializes and transmits an OpenFlow frame over a raw socket.
     * Performs atomic writes (looping on send) to ensure the frame isn't fragmented by the kernel.
     */
    bool send_frame(int fd, PacketType type, uint32_t xid, const std::vector<uint8_t>& body = {}) {
        if (fd < 0) {
            return false;
        }

        const size_t total_len = kOFHeaderLen + body.size();
        if (total_len > kOFMaxFrameLen) {
            return false;
        }

        std::vector<uint8_t> wire(total_len);
        wire[0] = kOFVersion10;
        wire[1] = static_cast<uint8_t>(type);

        const uint16_t n_len = htons(static_cast<uint16_t>(total_len));
        std::memcpy(wire.data() + 2, &n_len, sizeof(n_len));

        const uint32_t n_xid = htonl(xid);
        std::memcpy(wire.data() + 4, &n_xid, sizeof(n_xid));

        if (!body.empty()) {
            std::memcpy(wire.data() + kOFHeaderLen, body.data(), body.size());
        }

        size_t sent = 0;
        while (sent < wire.size()) {
            const ssize_t n = ::send(fd, wire.data() + sent, wire.size() - sent, MSG_NOSIGNAL);
            if (n <= 0) {
                return false;
            }
            sent += static_cast<size_t>(n);
        }

        return true;
    }

    /**
     * Ingests raw bytes from the network, runs the framing loop, and dispatches to handlers.
     * This is the "pump" of the Southbound Interface.
     */
    virtual bool ingest(Connection& conn, const uint8_t* data, size_t len) {
        if (!data || len == 0 || conn.fd < 0) {
            return false;
        }

        conn.inbuf.insert(conn.inbuf.end(), data, data + len);
        conn.last_activity = std::chrono::steady_clock::now();

        while (true) {
            Frame frame{};
            const ParseStatus status = Framer::next_frame(conn.inbuf, frame);
            if (status == ParseStatus::NEED_MORE) {
                return true; // Wait for more data
            }
            if (status == ParseStatus::INVALID) {
                return false; // Close connection on protocol violation
            }

            const PacketType type = static_cast<PacketType>(frame.header.type);
            if (!run_validator(type, frame)) {
                return false; // Security check failed
            }

            advance_lifecycle(conn, type);
            dispatch(conn, type, frame);
        }
    }

    /**
     * Helper to perform raw socket read into a provided scratch buffer.
     */
    static bool recv_into_buffer(Connection& conn, std::vector<uint8_t>& scratch, ssize_t& out_read) {
        if (conn.fd < 0) {
            out_read = -1;
            return false;
        }

        if (scratch.empty()) {
            scratch.resize(8192);
        }

        out_read = ::recv(conn.fd, scratch.data(), scratch.size(), 0);
        if (out_read <= 0) {
            return false;
        }

        return true;
    }

protected:
    /**
     * Routes the complete frame to its designated logic handler.
     */
    virtual void dispatch(Connection& conn, PacketType type, const Frame& frame) const {
        const auto it = handlers_.find(type);
        if (it == handlers_.end()) {
            return; // Ignore unhandled packet types
        }

        for (const auto& entry : it->second) {
            if (entry.filter(frame)) {
                entry.handler(conn, frame);
                break; // Execute only the first matching "route"
            }
        }
    }

    /**
     * Runs registered security checks before a packet is dispatched.
     */
    bool run_validator(PacketType type, const Frame& frame) const {
        const auto it = validators_.find(type);
        if (it == validators_.end()) {
            return true; // No validator registered, allow by default
        }

        return it->second(frame.body.data(), frame.body.size());
    }

    struct FilteredHandler {
        FilterFn filter;
        HandlerFn handler;
    };

    std::unordered_map<PacketType, ValidatorFn> validators_;
    std::unordered_map<PacketType, std::vector<FilteredHandler>> handlers_;

    /**
     * Internal state machine update to track the switch handshake.
     */
    static void advance_lifecycle(Connection& conn, PacketType type) {
        switch (conn.state) {
            case SessionState::CONNECTED:
                if (type == PacketType::HELLO) {
                    conn.state = SessionState::HELLO_DONE;
                }
                break;
            case SessionState::HELLO_DONE:
                if (type == PacketType::FEATURES_REPLY || type == PacketType::FEATURES_REQUEST) {
                    conn.state = SessionState::FEATURES_DONE;
                }
                break;
            case SessionState::FEATURES_DONE:
                conn.state = SessionState::ACTIVE;
                break;
            case SessionState::ACTIVE:
            case SessionState::CLOSING:
                break;
        }
    }
};

/**
 * Closes a switch connection and resets its state.
 */
inline void close_connection(Connection& conn) {
    if (conn.fd >= 0) {
        ::close(conn.fd);
        conn.fd = -1;
        conn.state = SessionState::CLOSING;
    }
}

} // namespace oflib

// Include external headers AFTER the main namespace to avoid any parsing/shadowing issues
#include "../../core/Classes/ThreadEnvironment.hpp"
#include "../../core/Classes/TaskQueue.hpp"

namespace oflib {

/**
 * Server: A declarative, thread-safe OpenFlow server that manages multiple switches.
 */
class Server : public OpenFlowEndpoint {
public:
    Server(size_t worker_threads = 4);
    ~Server() override;

    // Declarative API with optional filters (similar to NBI routes)
    Server& packet_in(HandlerFn h, FilterFn f = filter::all()) { on(PacketType::PACKET_IN, std::move(h), std::move(f)); return *this; }
    Server& flow_removed(HandlerFn h, FilterFn f = filter::all()) { on(PacketType::FLOW_REMOVED, std::move(h), std::move(f)); return *this; }
    Server& port_status(HandlerFn h, FilterFn f = filter::all()) { on(PacketType::PORT_STATUS, std::move(h), std::move(f)); return *this; }
    Server& error(HandlerFn h, FilterFn f = filter::all()) { on(PacketType::ERROR, std::move(h), std::move(f)); return *this; }
    Server& hello(HandlerFn h, FilterFn f = filter::all()) { on(PacketType::HELLO, std::move(h), std::move(f)); return *this; }
    Server& features_reply(HandlerFn h, FilterFn f = filter::all()) { on(PacketType::FEATURES_REPLY, std::move(h), std::move(f)); return *this; }

    /**
     * Starts the server on the specified host and port.
     */
    bool listen(const std::string& host, int port);

    /**
     * Stops the server and closes all active connections.
     */
    void stop();

    /**
     * Returns the next frame from the internal event queue.
     */
    void wait_for_event(Connection& out_conn, Frame& out_frame);

protected:
    /**
     * Override to push events to the internal queue after calling handlers.
     */
    void dispatch(Connection& conn, PacketType type, const Frame& frame) const override;

private:
    void accept_loop(int listen_sock);
    void handle_client(int client_fd);

    std::atomic<bool> running_{false};
    std::thread acceptor_thread_;
    std::unique_ptr<ThreadEnvironment> pool_;
    
    struct Event {
        Connection conn;
        Frame frame;
    };
    std::unique_ptr<TaskQueue<Event>> event_queue_;

    std::mutex connections_mutex_;
    std::unordered_map<int, Connection> connections_;
};

} // namespace oflib

#endif // NETORC_OPENFLOW_LIB_H
