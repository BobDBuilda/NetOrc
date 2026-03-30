#include <string>
#include "ISouthBoundInterface.hpp"
#include <winsock.h>
#include <thread>
#include "OpenFlowHeader.h"

class SouthBoundInterface:public ISouthBoundInterface{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    WSADATA wsa;
    uint8_t PORT_NUMBER;

    SOCKET listening_sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server;

    //should this actually be in the manager
    SOCKET connection_thread_pool[];
    //the listening sock is not part of this
    //the receving sock will be pulled from this pool

public:
    bool connect(uint32_t ip, uint8_t port) override{
        WSAStartup(MAKEWORD(2,2), &wsa); //Init Winsock

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY; //GOING TO alter this to accept IPs later
        //Host to network short so that it can be understood by net devices.
        server.sin_port = htons(PORT_NUMBER);

        if (bind(listening_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR){
            std::cout << "Bind Failed!" << std::endl;
            return;
        }


        //listens on the socket created above and sets backlog limit to 5
        //to limit number of active connections for the time being
        listen(listening_sock, 5);

        std::cout << "Southbound Interface listening on port " << PORT_NUMBER << std::endl;

        while(true){
            SOCKET new_sock = accept(listening_sock, NULL, NULL);

            if(new_sock != INVALID_SOCKET){
                std::thread(dispatch, new_sock).detach();
                this->dispatch(new_sock);
            }
        }
    }

    // int receiveData(char* buffer, int len) override {
    //     return recv(sock, buffer, len, 0); 
    // }

    void dispatch(SOCKET client_sock){
        char buffer[4096];
        int bytesReceived = recv(client_sock, buffer, sizeof(buffer), 0);

        OpenFlowHeader* hdr = reinterpret_cast<OpenFlowHeader*>(buffer);
        char* body = buffer + 8;

        //bcuz yk simple math
        int bodySize = bytesReceived - 8;
            //create a function that accepts the socket data etc
            //
        if(hdr->TYPE == 10){
            //HANDLE_PACKET_IN(body, bodySize);
            //should actually be able to handle this via polymorphism
            //message.process()
            //message was an interface and every header and body
            //is based off it, thus has the process method
            //and each has its unique implementation
            //so just call which it is to handle?
        }
    }


};