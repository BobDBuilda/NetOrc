// #ifndef SOUTHBOUND_INTERFACE_MANAGER_HPP
// #define SOUTHBOUND_INTERFACE_MANAGER_HPP

// #include <unordered_map>
// #include <stdint.h>
// #include <memory>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include "ISouthBoundInterface.hpp"
// #include "SouthBoundInterface.hpp"
// #include "ThreadEnvironment.hpp"
// #include "Connection.hpp"

// class SouthBoundInterfaceManager {
//     // Map socket file descriptors to their Connection state
//     // This allows O(1) lookup when data arrives on a specific socket
//     std::unordered_map<int, std::unique_ptr<Connection>> active_connections;

//     ThreadEnvironment thread_env = ThreadEnvironment(5);
//     int server_fd;

// public: 
//     SouthBoundInterfaceManager() : server_fd(-1) {
//         this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
//         if (this->server_fd < 0) {
//             perror("socket creation failed");
//         }
//     }

//     ~SouthBoundInterfaceManager() {
//         if (this->server_fd >= 0) {
//             close(this->server_fd);
//         }
//     }

//     void add_connection(int fd, sockaddr_in addr) {
//         active_connections[fd] = std::make_unique<Connection>(fd, addr);
//     }

//     void remove_connection(int fd) {
//         active_connections.erase(fd);
//         close(fd);
//     }
// };

// #endif // SOUTHBOUND_INTERFACE_MANAGER_HPP