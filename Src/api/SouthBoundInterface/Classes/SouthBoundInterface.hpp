#include <winsock.h>
//#include <thread>
#include "OpenFlowHeader.h"
#include "Message.hpp"
#include <chrono>
#include <unordered_map>
#include <functional>
#include <memory>
//#include "ThreadPool.hpp"

//the job of the SBI is strictly to translate. it takes raw binary from the wire
//and deserializes it into a protocol specific object
//

using FactoryFn = std::function<std::unique_ptr<OFPacket>(char*, int)>;

std::unordered_map<OFPacketType, FactoryFn> packetFactories = {
    {OFPacketType::HELLO, [](char*,int){ return std::make_unique<OFPacketHello>(); }},
    {OFPacketType::PACKET_IN, [](char*,int){ return std::make_unique<OFPacketIn>(); }},
    // ... add other types
};

class SouthBoundInterface{
public:
    void init(){
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sock == INVALID_SOCKET){
            std::cout << WSAGetLastError() << std::endl;
        }

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(20045);

        if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR){
            std::cout << "Bind Failed!" << std::endl;
        }

        listen(sock, SOMAXCONN);
        std::cout << "Southbound Interface listening on port " << 20045 << std::endl;

        accept_conn(sock);
    }

    void accept_conn(SOCKET sock){
        std::cout << "Transitioned to the accepting state ..." << std::endl;

        int tries = 0;
        while(true){
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);

            timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            int activity = select(0, &readfds, NULL, NULL, &timeout);

            if(activity > 0 && FD_ISSET(sock, &readfds)){
                SOCKET client = accept(sock, nullptr, nullptr);
                std::cout << "Client connected" << std::endl;
                tries = 0;
                receive_conn(client);
            }else{
                std::cout << "No incoming connection ... " << std::endl;
            } 
            tries++;
        }
        //by default the above is blocking, for non-blocking behavior
        //the socket must be configured using fcntl() or WSAEventSelect()   
    }

    void receive_conn(SOCKET sock){
        char buffer[4096];
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

        //OpenFlowHeader* hdr = reinterpret_cast<OpenFlowHeader*>(buffer);
        //char* body = buffer + 8;

        if (bytesReceived < 8) {
            std::cerr << "Incomplete header received\n";
            return;
        }

        uint8_t version = buffer[0];
        uint8_t rawtype    = buffer[1];

        uint16_t length;
        memcpy(&length, buffer + 2, 2);
        length = ntohs(length);

        uint32_t xid;
        memcpy(&xid, buffer + 4, 4);
        xid = ntohl(xid);

        // Payload starts immediately after the header
        char* body = buffer + 8;
        int bodySize = bytesReceived - 8;

        std::cout << "Version: " << (int)version << "\n";
        std::cout << "Type: " << (int)rawtype << "\n";
        std::cout << "Length: " << length << "\n";
        std::cout << "XID: " << xid << "\n";

        OFPacketType type;
        switch(rawtype){
            case 0: type = OFPacketType::HELLO; break;
            case 10: type = OFPacketType::PACKET_IN; break;
            default:
                std::cerr << "Unknown type: " << (int)rawtype << std::endl;
                return;
        } 

        auto it = packetFactories.find(type);
        if(it != packetFactories.end()){
            auto pkt = it->second(buffer + 8, bytesReceived - 8);
            pkt->process();
        }


    }

    void send_res(SOCKET sock){
    
    }
};
