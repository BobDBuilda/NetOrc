#ifdef _WIN32
    #include <iostream>
    #include <winsock2.h>
    #include <cstdint>
    //#include "OpenFlowHeader.h"
    #include "SouthBoundInterface.hpp"
    //typedef SOCKET socket_t
    //#include <ws2_32.lib>
    #pragma comment(lib, "ws2_32.lib")
#else
    typedef int socket_t
    #include <sys/socket.h>
    #include <netinet.in/h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

 

//#include "SouthBoundInterfaceManager.h"
//#include "App.h"
    //Now you would recv() the 8-byte header here
    //Apparently the first thing a switch will send is an OFPT_HELLO(type 0).
    //to stay connected, your controller must immediatley send an OFPT_HELLO
    //back. if you don't, the switch will timeout and disconnect within seconds.

int main(){

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SouthBoundInterface SBI;

    SBI.init();
    
    
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
