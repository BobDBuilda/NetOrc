#include <stdint.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

int recv_all(int sock, char* buffer, int length){
    int total = 0;

    while(total < length){
        int n = recv(sock, buffer + total, length - total, 0);

        if(n == 0) return 0;
        if(n < 0) return -1;

        total += n;
    }

    return total;
}

class SouthBoundInterfaceManager{
private:
    //A SBI acts as the communication link enabling
    //the centralized SDN controller to manage, configure
    //and instruct physical or virtual network devices
    //(switches, routers). It translates high-level
    //control policies into actionable commands
    //for the data plane and facilitates bidirectional
    //communication, allowing the controller to receive
    //network status updates

    //what exactly would i need to manage in an API?
    //accounting to see what devices used this interface
    //
    SOCKET socket1;
    uint16_t SOUTHBOUND_PORT;
    bool isAlive;
    uint16_t TOTAL_CONNECTIONS;
public:
    void CheckInternalStatus(uint8_t port){
        //check to see if the port through which
        //packet_in messages come in is up and running
        //check to see if controller is ok???
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2), &wsa); //Init Winsock

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(SOUTHBOUND_PORT);

        try{
            bind(sock, (struct sockaddr*)&server, sizeof(server));
            std::cout << "Bind failed" << std::endl;
        }catch(std::string error){
            std::abort();
        }

        listen(sock, 3);
        std::cout << "Controller listening on port" << std::endl;
    }

    void static Init(){
        //bind a TCP socket and listen on it
        //can i send a braodcast from here to see
        //what devices are connected?
        //first craft the packet
        //
        //i can pass in a function here to check whether
        //the, 
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2), &wsa); //Init Winsock

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(6653); //standard SDN port

        if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR){
            std::cout << "Bind Failed!" << std::endl;
            return;
        }

        listen(sock,3);

        std::cout << "Southbound Interface listening on port 6653..." << std::endl;

        while(true){
            SOCKET new_socket = accept(sock, NULL, NULL);

            //could put a handle socket function here

            // std::thread([new_socket](){
            //     char buffer[1024];
            //     while(true){
            //         int n = recv(new_socket, buffer, sizeof(buffer), 0);
            //         if(n <= 0) break;
            //         std::cout << "Switch connected 1" << std::endl;
            //         send(new_socket, buffer, n, 0);
            //     }
            //     std::cout << "Switch connected 2" << std::endl;
            //     closesocket(new_socket);
            // std::cout << "Switch connected 3" << std::endl;
            // }).detach();
        }
        // std::cout << "A Switch connected!" << std::endl;

        //Now you would recv() the 8-byte header here
        //Apparently the first thing a switch will send is an OFPT_HELLO(type 0).
        //to stay connected, your controller must immediatley send an OFPT_HELLO
        //back. if you don't, the switch will timeout and disconnect within seconds.
    }

    bool IsAuthorizeConnection(OpenFlowHeader hdr, SOCKET sock){
        //should authorize connection be a method
        //or a standalone function, the connection should be authorized
        //based on certain criteria, similar to how http requests
        //on sites may auth using CSRF tokens or cookies.

        //first get header
        int total = recv_all(sock, (char*)&hdr, sizeof(OpenFlowHeader));
        if(total < 8) return; //if not correct length, may be corrupted

        //ensure correct endianness
        hdr.TYPE = ntohl(hdr.TYPE);
        hdr.LENGTH = ntohl(hdr.LENGTH);
        
        if()
        //

    }

    void ValidateRequest(){

    }

    void PACKET_IN(){
        
    }
};