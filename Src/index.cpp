#ifdef _WIN32
    #include <iostream>
    #include <winsock2.h>
    #include <cstdint>
    #include "OpenFlowHeader.h"
    //#include <ws2_32.lib>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet.in/h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


void handle_switch(int client_socket){
    char buffer[1024];

    //receive openFlow Message (eg. HELLO or PACKET_IN)
    int bytes = recv(client_socket, buffer, sizeof(buffer), 0);

    //Identify Message Type (Basic OpenFlow Header parsing)
    uint8_t version = buffer[0];
    uint8_t type = buffer[1]; //Type 10 = Packet-In in OpenFlow 1.3

    if (type == 10){
        std::cout << "Switch sent a packet it doesn't recognize. deciding..." <<std::endl;
        
        //Create a FLOW_MOD message (Simplest: "Flood to all ports")
        //in a real MVP, construct a binary OpenFlow packet here.

        char flow_mod[1] = {/*Binary data for 'Output: ALL'*/};
        send(client_socket, flow_mod, sizeof(flow_mod), 0);
    }
}


void start_controller(){
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

    std::cout << "Controller listening on port 6653..." << std::endl;

    //Accept a switch connection
    //shouldnt the switch acceptance come
    //after it is initialized
    SOCKET new_socket = accept(sock, NULL, NULL);

    std::cout << "Switch connected!" << std::endl;

    //Now you would recv() the 8-byte header here
    //Apparently the first thing a switch will send is an OFPT_HELLO(type 0).
    //to stay connected, your controller must immediatley send an OFPT_HELLO
    //back. if you don't, the switch will timeout and disconnect within seconds.
}


int main(){
    start_controller();
    //initialize southbound interface and check
    //to see if 

    return 0;
}
//need to bind to sockets
//when packets come in
//see what they are and where they 
//are supposed to go
//when i say 'see what they are'
//what is emant by that?

//what the IP src/dst is? What about MAC?
//then 'switch' them along those routes

//#include <netinet/in.h>
