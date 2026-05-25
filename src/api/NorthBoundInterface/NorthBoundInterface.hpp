#ifndef NORTHBOUND_INTERFACE_HPP
#define NORTHBOUND_INTERFACE_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

class NorthBoundInterface {
public:
     void init() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        //values such as -1 and 0 are used to denote success or failure in system calls.
        if (sock < 0) {
            perror("socket failed");
            return;
        }


        //what is opt1? 
        int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
        }

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(20045);

        if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            perror("bind failed");
            close(sock);
            return;
        }

        if (listen(sock, SOMAXCONN) < 0) {
            perror("listen failed");
            close(sock);
            return;
        }
        std::cout << "Southbound Interface listening on port " << 20045 << std::endl;

        accept_conn(sock);
    }

private:
    void accept_conn(int sock) {
        // Basic placeholder for accepting connections
        while (true) {
            int client = accept(sock, nullptr, nullptr);
            if (client >= 0) {
                std::cout << "NBI: Accepted connection" << std::endl;
                close(client);
            } else {
                perror("accept failed");
                break;
            }
        }
    }
};

#endif // NORTHBOUND_INTERFACE_HPP
