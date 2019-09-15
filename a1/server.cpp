//
//  main.cpp
//  server
//
//  Created by Zihan Yang on 2018-09-28.
//  Copyright © 2018 Zihan Yang. All rights reserved.
//


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>

int main(int argc, const char * argv[]) {
    int sockfd_UDP,n_port, num;
    struct sockaddr_in saddr_UDP;
    
    // set a random port No.
    srand((unsigned)time(0));
    n_port = rand() % 30000 + 1025;
    
    // create a socket
    sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd_UDP < 0) {
        perror("ERROR, can't opening socket\n");
        return -1;
    }
    
    // clear address structure
    memset(&saddr_UDP, '\0', sizeof(saddr_UDP));
    
    // setup the saddr's structure
    saddr_UDP.sin_family = AF_INET;
    saddr_UDP.sin_addr.s_addr = htonl(INADDR_ANY);
    
    num = 10000;
    while(1) {
        --num;
        saddr_UDP.sin_port = htons(n_port);
        if (bind(sockfd_UDP, (struct sockaddr *) &saddr_UDP, sizeof(saddr_UDP)) == 0) {
            break;
        }
    
        n_port++;
        if (num <= 0) {
            perror("ERROR, can't bind\n");
            return -1;
        }
    }
    
    // print port No.
    printf("SERVER_PORT=%d\n", n_port);
    
    while (1) {
        
        struct sockaddr_in caddr_UDP;
        socklen_t clen = sizeof(caddr_UDP);
        char buffer[256];
        
        memset(buffer, 0, 256);
        
        // initiate negotiation
        if (recvfrom(sockfd_UDP, buffer, sizeof(buffer), 0, (struct sockaddr *) &caddr_UDP, &clen) < 0) {
            perror("ERROR, can't receive request from client\n");
            return -1;
        }
        
        //printf("receive req_code done\n");
        
        std::string req_code(buffer);
        if (req_code != argv[1]) {
            perror("ERROR, wrong req_code\n");
            return -1;
        }
        
        //printf("check req_code done\n");
        
        // req_code confirmed, create TCP socket
        int sockfd_TCP, r_port;
        struct sockaddr_in saddr_TCP;
        
        sockfd_TCP = socket(AF_INET, SOCK_STREAM, 0);
        
        if (sockfd_TCP < 0) {
            perror("ERROR, can't opening socket\n");
            return -1;
        }
        
        r_port = rand() % 30000 + 1025;
        
        // clear address structure
        memset(&saddr_TCP, '\0', sizeof(saddr_TCP));
        
        // setup the saddr's structure
        saddr_TCP.sin_family = AF_INET;
        saddr_TCP.sin_addr.s_addr = htonl(INADDR_ANY);
        
        num = 10000;
        while(1) {
            --num;
            saddr_TCP.sin_port = htons(r_port);
            if (bind(sockfd_TCP, (struct sockaddr *) &saddr_TCP, sizeof(saddr_TCP)) == 0) {
                break;
            }
            
            r_port++;
            if (num <= 0) {
                perror("ERROR, can't bind\n");
                return -1;
            }
        }
        
        // send back r_port number
        //memcpy(buffer, &r_port, sizeof(int));
        
        std::string str = std::to_string(r_port);
        char const *pchar = str.c_str();
        
        if (sendto(sockfd_UDP, pchar, sizeof(pchar), 0, (struct sockaddr*) &caddr_UDP, clen) < 0) {
            perror("ERROR, send r_port failed\n");
            return -1;
        };
        
        // receive confirm message
        memset(buffer, 0, 256);
        if (recvfrom(sockfd_UDP, buffer, sizeof(buffer), 0, (struct sockaddr *) &caddr_UDP, &clen) < 0) {
            perror("ERROR, can't receive request from client\n");
            return -1;
        }
        
        // check confirm message
        if (buffer[0] == '0') {
            perror("ERROR, r_port can't match\n");
            return -1;
        }
        
        // send back acknowledge of correctness to client
        memset(buffer, 0, 256);
        buffer[0] = '1';
        if (sendto(sockfd_UDP,buffer, sizeof(buffer), 0, (struct sockaddr*) &caddr_UDP, clen) < 0) {
            perror("ERROR, send acknowledge failed\n");
            return -1;
        }

        printf("SERVER_TCP_PORT=%d\n",r_port);
        
        // listen to the socket
        if ((listen(sockfd_TCP, 5) < 0)) {
            perror("ERROR, listen failed\n");
            return -1;
        }
        
        // accept
        int newsockfd;
        newsockfd = accept(sockfd_TCP, NULL, NULL);
        
        if (newsockfd < 0) {
            perror("ERROR, accept failed");
            return -1;
        }
        
        // server receive message
        memset(buffer, 0, 256);
        
        if (read(newsockfd,buffer,sizeof(buffer)) < 0) {
            perror("ERROR, can't read from socket\n");
            close(newsockfd);
            return -1;
        }
        
        printf("SERVER_RCV_MSG='%s'\n", buffer);

        // reverse message
        std::string message(buffer);
        std::reverse(message.begin(), message.end());
        // send back reversed message
        write(newsockfd, message.c_str(), message.size());
        
        close(newsockfd);
        close(sockfd_TCP);
    }
    
    return 0;
}
 
