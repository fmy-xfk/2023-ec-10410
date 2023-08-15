#ifndef __TCP_H__
#define __TCP_H__

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_common.h"

#define TCP_SUCCESS 0
#define TCP_ERR_BIND 1
#define TCP_ERR_LISTEN 2
#define TCP_ERR_ACCEPT 3
#define TCP_ERR_CONV 4
#define TCP_ERR_CONNECT 5

typedef struct{
    int backlog,sockfd,connfd;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen;
    struct sockaddr_in serverAddr;
}tcp_server;

void tcp_server_clean(tcp_server *sv);
void tcp_server_disconnect(tcp_server *sv);
unsigned int tcp_server_start(tcp_server *sv, unsigned short port);
int tcp_server_recv(tcp_server *sv,unsigned char* buf,int maxlen);
int tcp_server_send(tcp_server *sv,unsigned char* buf,int len);

typedef struct{
    int sockfd,connfd;
    volatile int connected;
    struct sockaddr_in serverAddr;
    void (*onConnectionChanged)(int connected);
}tcp_client;

void tcp_client_clean(tcp_client *sv);
unsigned int tcp_client_init(tcp_client *sv,const char* host,unsigned short port,void (*onConnectionChanged)(int));
unsigned int tcp_client_connect(tcp_client *sv);
int tcp_client_recv(tcp_client *sv,unsigned char* buf,int maxlen);
int tcp_client_send(tcp_client *sv,unsigned char* buf,int len);

#endif