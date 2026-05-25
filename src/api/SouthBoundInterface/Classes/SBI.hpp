#ifndef SOUTHBOUND_INTERFACE_HPP
#define SOUTHBOUND_INTERFACE_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "OpenFlowPackets.hpp"
#include "../../../core/Classes/ThreadEnvironment.hpp"
#include "../../../core/Classes/TaskQueue.hpp"
#include "../../../core/Classes/Config.hpp"
#include "../Utils/validators/HeaderValidator.hpp"
#include <vector>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <memory>
#include <thread>
#include <array>

/**
 * NetworkEvent: The "Result Object".
 * This is what the worker thread produces and places into the Event Loop's queue.
 * Now points to an OFPacketType (enum) instead of an OFPacket object.
 * Updated to carry the 'payload' (message body) to support the Handler Map pattern.
 */
struct NetworkEvent {
    int client_fd;
    OFPacketType type;
    std::vector<uint8_t> payload; // The raw body of the OpenFlow message
};

// Alias for a function type that returns a boolean indicating if the packet is valid.
using ValidatorFn = std::function<bool(char*, int)>;

extern std::unordered_map<OFPacketType, ValidatorFn> packetValidators;

class SouthBoundInterface {
private:
    Configuration* config = nullptr;

public:
    void init(ThreadEnvironment& threadEnvironment, TaskQueue<NetworkEvent>& eventQueue, Configuration& cfg) {
        this->config = &cfg;
        [&]() {
            try {
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0) {
                    throw std::runtime_error("socket creation failed");
                }

                int opt = 1;
                if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                    perror("setsockopt warning");
                }

                sockaddr_in server{};
                server.sin_family = AF_INET;
                server.sin_addr.s_addr = INADDR_ANY;
                server.sin_port = htons(20045);

                if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
                    close(sock);
                    throw std::runtime_error("bind failed");
                }

                if (listen(sock, SOMAXCONN) < 0) {
                    close(sock);
                    throw std::runtime_error("listen failed");
                }

                if (config->verbose) {
                    std::cout << "Southbound Interface listening on port " << 20045 << " (Event-Loop Model)" << std::endl;
                }

                std::thread acceptor_thread([sock, &threadEnvironment, &eventQueue, this]() {
                    while (true) {
                        int client = accept(sock, nullptr, nullptr);
                        if(client >= 0){
                            threadEnvironment.enqueue([client, &eventQueue, this]() {
                                // Why is 'this' passed in the lambda capture list?
                                // To access handle_data and config within the worker thread.
                                handle_data(client, eventQueue);
                            });
                        }else{
                            if (config->debug) {
                                perror("accept failed");
                            }
                        }
                    }
                });
                //runs in the background.
                acceptor_thread.detach();
            } catch (const std::exception& err) {
                std::cerr << "SBI Initialization Error: " << err.what() << std::endl;
            }
        }();
    }

    /**
     * send_packet: Encapsulates sending OpenFlow-compliant messages to a switch.
     */
    void send_packet(int sock, OFPacketType type, uint32_t xid, const std::vector<uint8_t>& payload = {}) {
        uint16_t totalLen = 8 + static_cast<uint16_t>(payload.size());
        std::vector<uint8_t> buffer(totalLen);

        buffer[0] = 0x01; // OpenFlow Version 1.0
        buffer[1] = static_cast<uint8_t>(type);
        
        uint16_t n_len = htons(totalLen);
        memcpy(&buffer[2], &n_len, 2);
        
        uint32_t n_xid = htonl(xid);
        memcpy(&buffer[4], &n_xid, 4);
        
        if (!payload.empty()) {
            std::copy(payload.begin(), payload.end(), buffer.begin() + 8);
        }

        if (send(sock, buffer.data(), buffer.size(), 0) < 0) {
            if (config->debug) perror("send failed");
        }
    }

    /**
     * handle_data: Entry point for socket data management (Receive -> Parse -> Dispatch).
     */
    void handle_data(int sock, TaskQueue<NetworkEvent>& eventQueue) {
        //why is the buffer size 8192?
        std::array<char, 8192> buffer{};
        ssize_t bytesReceived = recv(sock, buffer.data(), buffer.size(), 0);

        if (!handleReceiveResult(sock, bytesReceived)) {
            return;
        }

        const OpenFlowHeader ofHdr = parseHeader(buffer.data());
        if (!Validator::validateHeader(ofHdr, static_cast<size_t>(bytesReceived))) {
            if (config->debug) {
                std::cerr << "Header validation failed for FD: " << sock << std::endl;
            }
            close(sock);
            return;
        }

        const OFPacketType type = static_cast<OFPacketType>(ofHdr.TYPE);
        const char* body = buffer.data() + 8;
        const int bodyLength = static_cast<int>(bytesReceived - 8);

        if (!validateBody(type, body, bodyLength)) {
            close(sock);
            return;
        }

        pushEvent(sock, type, body, bodyLength, eventQueue);
    }

private:
    bool handleReceiveResult(int sock, ssize_t bytesReceived) {
        if (bytesReceived >= 8) {
            return true;
        }

        if (bytesReceived == 0) {
            if (config->verbose) {
                std::cout << "Client disconnected" << std::endl;
            }
        } else if (bytesReceived < 0) {
            if (config->debug) {
                perror("recv failed");
            }
        } else {
            if (config->debug) {
                std::cerr << "Incomplete header received from FD: " << sock << std::endl;
            }
        }

        close(sock);
        return false;
    }

    static OpenFlowHeader parseHeader(const char* buffer) {
        OpenFlowHeader hdr{};
        hdr.VERSION = static_cast<uint8_t>(buffer[0]);
        hdr.TYPE = static_cast<uint8_t>(buffer[1]);

        uint16_t rawLength;
        memcpy(&rawLength, buffer + 2, sizeof(rawLength));
        hdr.LENGTH = ntohs(rawLength);

        uint32_t rawXid;
        memcpy(&rawXid, buffer + 4, sizeof(rawXid));
        hdr.XID = ntohl(rawXid);

        return hdr;
    }

    bool validateBody(OFPacketType type, const char* body, int bodyLength) {
        auto it = packetValidators.find(type);
        if (it == packetValidators.end()) {
            return true;
        }

        if (!it->second(const_cast<char*>(body), bodyLength)) {
            if (config->debug) {
                std::cerr << "Security Alert: Type-specific validation failed for packet type "
                          << static_cast<int>(type) << ". Dropping connection." << std::endl;
            }
            return false;
        }

        return true;
    }

    void pushEvent(int sock,
                   OFPacketType type,
                   const char* body,
                   int bodyLength,
                   TaskQueue<NetworkEvent>& eventQueue) {
        if (config->verbose) {
            std::cout << "[Worker] Task complete. Pushing event type " << static_cast<int>(type)
                      << " for Client FD: " << sock << std::endl;
        }

        //creates a payload vector from the body data and length, 
        //and moves it into the event queue.
        std::vector<uint8_t> payload(body, body + bodyLength);
        eventQueue.push({sock, type, std::move(payload)});
    }
};

#endif // SOUTHBOUND_INTERFACE_HPP
