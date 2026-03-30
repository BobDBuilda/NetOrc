#include <string>
#include "ISouthBoundInterface.hpp"
#include <winsock.h>
#include <thread>

class SouthBoundInterface:public ISouthBoundInterface{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    WSADATA wsa;
    uint8_t PORT_NUMBER;

    SOCKET listening_sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;

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
            }
        }
    }

    // int receiveData(char* buffer, int len) override {
    //     return recv(sock, buffer, len, 0); 
    // }

    void dispatch(SOCKET client_sock){
        char buffer[4096];
        recv(client_sock, buffer, sizeof(buffer), 0);

        //give buffer to respective function
    }
};