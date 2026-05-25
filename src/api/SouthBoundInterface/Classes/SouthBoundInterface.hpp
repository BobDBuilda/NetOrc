// #ifndef SOUTHBOUND_INTERFACE_HPP
// #define SOUTHBOUND_INTERFACE_HPP

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <cstring>
// #include <iostream>
// #include "OpenFlowPackets.hpp"
// #include <chrono>
// #include <unordered_map>
// #include <functional>
// #include <memory>

// // the job of the SBI is strictly to translate. it takes raw binary from the wire
// // and deserializes it into a protocol specific object

// //the below creates an alias for a function type that takes a char pointer and an integer 
// //and returns a boolean indicating if the packet is valid.
// using ValidatorFn = std::function<bool(char*, int)>;

// extern std::unordered_map<OFPacketType, ValidatorFn> packetValidators;

// class SouthBoundInterface {
// public:
//     void init() {
//         int sock = socket(AF_INET, SOCK_STREAM, 0);
//         //values such as -1 and 0 are used to denote success or failure in system calls.
//         if (sock < 0) {
//             perror("socket failed");
//             return;
//         }
        
//         int opt = 1;
//         if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//             perror("setsockopt");
//         }

//         sockaddr_in server;
//         server.sin_family = AF_INET;
//         server.sin_addr.s_addr = INADDR_ANY;
//         server.sin_port = htons(20045);

//         if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
//             perror("bind failed");
//             close(sock);
//             return;
//         }

//         if (listen(sock, SOMAXCONN) < 0) {
//             perror("listen failed");
//             close(sock);
//             return;
//         }
//         std::cout << "Southbound Interface listening on port " << 20045 << std::endl;

//         accept_conn(sock);
//     }

//     void accept_conn(int sock) {
//         std::cout << "Transitioned to the accepting state ..." << std::endl;

//         //in network programming, fcntl is primarily used to switch a socket
//         //beteen bloking and non-blocking modes.

//         //by default sockets are blocking. if you call recv() and there is no data
//         //the thread stops and waits. Using fcntl, you can tell the OS to return
//         //immediately with an error(EWOULDBLOCK) INSTEAD OF WAITING.

//         //void make_non_blocking(int fd/sock){
//         //    //1.get current flags
//         //    int flags = fcntl(fd/sock, F_GETFL, 0);
//         //    //2.add non-blocking flag
//         //    fcntl(fd/sock, F_SETFL, flags | O_NONBLOCK);
//         //}

         
//         //this is synchronous.
//         while (true) {
//             fd_set readfds;
//             FD_ZERO(&readfds);
//             FD_SET(sock, &readfds);

//             timeval timeout;
//             timeout.tv_sec = 10;
//             timeout.tv_usec = 0;

//             //this is a timeout based select loop.  
//             int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

//             if (activity < 0) {
//                 perror("select error");
//                 break;
//             }

//             if (activity > 0 && FD_ISSET(sock, &readfds)) {
//                 //accept is synchronous. since the soket wasnt set to non-blocking
//                 //via fcntl.
//                 //can lock the entire thread if a client initiates a handshake but
//                 //fails to complete it.
//                 int client = accept(sock, nullptr, nullptr);
//                 if (client < 0) {
//                     perror("accept failed");
//                     continue;
//                 }
//                 std::cout << "Client connected" << std::endl;
//                 receive_conn(client);
//             } else {
//                 std::cout << "No incoming connection ... " << std::endl;
//             }
//         }
//     }

//     //thi is synchronous. since recv() is blocking.
//     void receive_conn(int sock) {
//         char buffer[4096];
//         ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

//         if (bytesReceived < 8) {
//             if (bytesReceived == 0) {
//                 std::cout << "Client disconnected" << std::endl;
//             } else if (bytesReceived < 0) {
//                 perror("recv failed");
//             } else {
//                 std::cerr << "Incomplete header received\n";
//             }
//             close(sock);
//             return;
//         }

//         uint8_t version = buffer[0];
//         uint8_t rawtype = buffer[1];

//         uint16_t length;
//         memcpy(&length, buffer + 2, 2);
//         length = ntohs(length);

//         uint32_t xid;
//         memcpy(&xid, buffer + 4, 4);
//         xid = ntohl(xid);

//         std::cout << "Version: " << (int)version << "\n";
//         std::cout << "Type: " << (int)rawtype << "\n";
//         std::cout << "Length: " << length << "\n";
//         std::cout << "XID: " << xid << "\n";

//         OFPacketType type;
//         switch (rawtype) {
//             case 0: type = OFPacketType::HELLO; break;
//             case 10: type = OFPacketType::PACKET_IN; break;
//             default:
//                 std::cerr << "Unknown type: " << (int)rawtype << std::endl;
//                 close(sock);
//                 return;
//         }

//         auto it = packetValidators.find(type);
//         if (it != packetValidators.end()) {
//             if (it->second(buffer + 8, bytesReceived - 8)) {
//                 std::cout << "Packet type " << (int)rawtype << " validated successfully" << std::endl;
//             } else {
//                 std::cerr << "Packet type " << (int)rawtype << " validation failed" << std::endl;
//             }
//         }
        
//         // In a real implementation, you might want to keep the socket open for further messages
//         // or pass it to a handler. For now, we'll keep it simple.
//     }

//     void send_res(int sock) {
//         // Implement response sending
//     }
// };

// #endif // SOUTHBOUND_INTERFACE_HPP
