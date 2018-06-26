//
//  udp_server.h
//  SysDig
//
//  Created by Ranjit Borra on 6/24/18.
//  Copyright © 2018 Ranjit Borra. All rights reserved.
//

#ifndef udp_server_h
#define udp_server_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;
namespace sysdig {
    class UdpServer{
    public:
        explicit UdpServer(string ip_address = "127.0.1.1", int port = 8200);
        void recv(char * message);
        int closeConnection();
    private:
        int sock_fd_ = 0;
        struct sockaddr_in address_;
    };
    UdpServer::UdpServer(string ip_address, int port){
        sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock_fd_ < 0){
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in cliaddr;
        memset(&address_, 0, sizeof(address_));
        memset(&cliaddr, 0, sizeof(cliaddr));
        address_.sin_family = AF_INET;
        address_.sin_addr.s_addr = inet_addr(ip_address.c_str());
        address_.sin_port = htons(port);
        
        char buffer[1024];
        unsigned int len;
        if (::bind(sock_fd_, (struct sockaddr *)&address_, sizeof(address_)))
            perror("ERROR on binding");
        while(1){
            
            auto n = ::recvfrom(sock_fd_, buffer, 1024, 0,
                         (struct sockaddr *)&cliaddr, &len);
            if (n < 0)
                perror("ERROR in recvfrom");
            cout<<buffer<<endl;
        }
    }
}
#endif /* udp_server_h */
