// #ifndef CONNECTION_HPP
// #define CONNECTION_HPP

// #include <netinet/in.h>
// #include <chrono>
// #include <vector>
// #include <cstdint>

// struct Connection {
//     int fd;                         // Socket file descriptor
//     sockaddr_in address;            // IP and Port of the switch
//     uint64_t dpid;                  // Unique Data Path ID (identifies the switch)
    
//     // Lifecycle tracking
//     std::chrono::steady_clock::time_point connected_at;
//     std::chrono::steady_clock::time_point last_activity;

//     // Buffer for handling partial messages or stream data
//     std::vector<uint8_t> read_buffer;

//     Connection(int socket_fd, sockaddr_in addr) 
//         : fd(socket_fd), address(addr), dpid(0) {
//         connected_at = std::chrono::steady_clock::now();
//         last_activity = connected_at;
//     }

//     ~Connection() {
//         // We don't close(fd) here because the Manager might want 
//         // to control the exact timing of socket shutdown.
//     }
// };

// #endif // CONNECTION_HPP
