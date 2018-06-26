//
//  udp_client.h
//  SysDig
//
//  Created by Ranjit Borra on 6/24/18.
//  Copyright © 2018 Ranjit Borra. All rights reserved.
//

#ifndef udp_client_h
#define udp_client_h

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

using namespace std;
namespace sysdig {
    class UdpClient{
    public:
        explicit UdpClient(string ip_address = "127.0.0.1", int port = 8000);
        void send(char * message);
        int closeConnection();
    private:
        int sock_fd_ = 0;
        struct sockaddr_in address_;
        
    };
    UdpClient::UdpClient(string ip_address, int port){
        sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock_fd_ < 0){
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        address_.sin_family = AF_INET;
        address_.sin_addr.s_addr = inet_addr(ip_address.c_str());
        address_.sin_port = htons(port);
    }
    int UdpClient::closeConnection(){
        return close(sock_fd_);
    }
    void UdpClient::send(char *message){
        sendto(sock_fd_, (const char *)message, strlen(message),
               0, (const struct sockaddr *) &address_,
               sizeof(address_));
        delete message;
    }
    
}

#endif /* udp_client_h */
