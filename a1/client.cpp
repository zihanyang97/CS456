//
//  main.cpp
//  client
//
//  Created by Zihan Yang on 2018-09-28.
//  Copyright © 2018 Zihan Yang. All rights reserved.
//



 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

int main(int argc, const char * argv[]) {
    int sockfd_UDP, n_port;
    struct sockaddr_in saddr_UDP;
    struct hostent *server;
    char buffer[256];
    
    if (argc != 5) {
        perror("ERROR, incorrect num of arguments\n");
    }
    
    //printf("check argc done\n");
    
    // get n_port number
    n_port = atoi(argv[2]);
    
    //printf("get n_port done\n");
    
    // create a socket
    sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_UDP < 0) {
        perror("ERROR, can't open socket\n");
        return -1;
    }
    
    //printf("create socket done\n");
    
    // find server
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        perror("ERROR, no such host\n");
        return -1;
    }
    
    // clear & setup address structure
    memset(&saddr_UDP, '\0', sizeof(saddr_UDP));
    saddr_UDP.sin_family = AF_INET;
    memcpy((char *) &saddr_UDP.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    saddr_UDP.sin_port = htons(n_port);
    
    //printf("find server done\n");
    
    // client send req_code to server
    memset(buffer, 0, 256);
    memcpy(buffer, argv[3], sizeof(char*));
    if (sendto(sockfd_UDP, buffer, strlen(buffer), 0, (struct sockaddr*) &saddr_UDP, sizeof(struct sockaddr)) < 0) {
        perror("ERROR, send req_code failed\n");
        return -1;
    }
    
    //printf("send req_code done\n");
    
    // receive r_port from server
    memset(buffer, 0, 256);
    socklen_t slen = sizeof(saddr_UDP);
    if (recvfrom(sockfd_UDP, buffer, sizeof(buffer), 0, NULL, NULL) < 0) {
        perror("ERROR, receive r_port failed\n");
        return -1;
    }
    
    //printf("receive r_port done\n");
    
    std::string str(buffer);
    int r_port = atoi(str.c_str());
    
    // send back confirm r_port to server
    memset(buffer, 0, 256);
    buffer[0] = '1';
    if (sendto(sockfd_UDP, buffer, strlen(buffer), 0,(struct sockaddr*) &saddr_UDP, sizeof(struct sockaddr)) < 0) {
        perror("ERROR, send req_code failed\n");
        return -1;
    }
    
    //printf("send back confirm  r_port done\n");
    
    // receive acknowledge from server
    memset(buffer, 0, 256);
    if (recvfrom(sockfd_UDP, buffer, sizeof(buffer), 0, (struct sockaddr *) &saddr_UDP, &slen) < 0) {
        perror("ERROR, receive acknowledge failed\n");
        return -1;
    }
    
    //printf("receive ack done\n");
    
    // check acknowledge message
    if (buffer[0] == '0') {
        perror("ERROR, server doesn't acknowledge\n");
        return -1;
    }
    
    // close UDP socket with server
    close(sockfd_UDP);
    
    // create TCP connection
    int sockfd_TCP;
    struct sockaddr_in saddr_TCP;

    sockfd_TCP = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd_TCP < 0) {
        perror("ERROR, can't open socket\n");
        return -1;
    }
    
    // clear & setup new address structure
    memset(&saddr_TCP, '\0', sizeof(saddr_TCP));
    saddr_TCP.sin_family = AF_INET;
    memcpy((char *) &saddr_TCP.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    //std::cout << "r_port TCP: " << r_port << std::endl;
    saddr_TCP.sin_port = htons(r_port);
    
    //printf("create TCP done\n");
    
    // connect to server
    if (connect(sockfd_TCP, (struct sockaddr *) &saddr_TCP, sizeof(saddr_TCP)) < 0) {
        perror("ERROR, connecting failed\n");
        return -1;
    }
    
    //printf("connect to server done\n");
    
    // send message to server
    memset(buffer, 0, 256);
    if (write(sockfd_TCP,argv[4],strlen(argv[4])) < 0) {
        perror("ERROR, send message failed\n");
        return -1;
    }
    
    //printf("send message done\n");
    
    // receive reversed message from server
    memset(buffer, 0, 256);
    if (read(sockfd_TCP, buffer, sizeof(buffer)) < 0) {
        perror("ERROR, receive message failed\n");
        return -1;
    }
    
    //printf("receive reversed message done\n");
    printf("CLIENT_RCV_MSG='%s'\n", buffer);
    
    // close TCP connection
    close(sockfd_TCP);
    return 0;
}
