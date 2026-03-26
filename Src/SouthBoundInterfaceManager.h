#include <stdint.h>
#include <winsock2.h>
#include <iostream>
#include <string>

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

    uint16_t SOUTHBOUND_PORT;
    bool isAlive;
    uint16_t TOTAL_DEVICES[];

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

    void Init(__callback){
        //bind a TCP socket and listen on it
        //can i send a braodcast from here to see
        //what devices are connected?
        //first craft the packet
        //
        //i can pass in a function here to check whether
        //the, 
    }

    void AuthorizeConnection(){

    }

    void ValidateRequest(){

    }

    void PACKET_IN(){
        
    }
};