#include "tcp.h"
#include "time.h"

void tcp_server_clean(tcp_server *sv){
    close(sv->sockfd);
}

void tcp_server_disconnect(tcp_server *sv){
    close(sv->connfd);
}

unsigned int tcp_server_start(tcp_server *sv, unsigned short port){
    ssize_t retval = 0;
    sv->backlog=1;
    sv->sockfd=socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    sv->connfd=-1;
    sv->clientAddrLen = sizeof(sv->clientAddr);
    sv->serverAddr.sin_family = AF_INET;
    sv->serverAddr.sin_port = htons(port);  // 端口号，从主机字节序转为网络字节序
    sv->serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 允许任意主机接入， 0.0.0.0
    retval = bind(sv->sockfd, (struct sockaddr *)&(sv->serverAddr), sizeof(sv->serverAddr)); // 绑定端口
    if (retval < 0) {
        printf("[TCP]bind failed, %ld!\r\n", retval);
        tcp_server_clean(sv);
        return TCP_ERR_BIND;
    }
    //printf("[TCP]bind to port %hu success!\r\n", port);

    retval = listen(sv->sockfd, sv->backlog); // 开始监听
    if (retval < 0) {
        printf("[TCP]listen failed!\r\n");
        tcp_server_clean(sv);
        return TCP_ERR_LISTEN;
    }
    //printf("[TCP]listen with %d backlog success!\r\n", sv->backlog);

    // 接受客户端连接，成功会返回一个表示连接的 socket ， clientAddr 参数将会携带客户端主机和端口信息 ；失败返回 -1
    // 此后的 收、发 都在 表示连接的 socket 上进行；之后 sockfd 依然可以继续接受其他客户端的连接，
    //  UNIX系统上经典的并发模型是“每个连接一个进程”——创建子进程处理连接，父进程继续接受其他客户端的连接
    //  鸿蒙liteos-a内核之上，可以使用UNIX的“每个连接一个进程”的并发模型
    //     liteos-m内核之上，可以使用“每个连接一个线程”的并发模型
    sv->connfd = accept(sv->sockfd, (struct sockaddr *)&(sv->clientAddr), &(sv->clientAddrLen));
    if (sv->connfd < 0) {
        printf("[TCP]accept failed, %d, %d\r\n", sv->connfd, errno);
        tcp_server_clean(sv);
        return TCP_ERR_ACCEPT;
    }
    //printf("[TCP]accept success, connfd = %d!\r\n", sv->connfd);
    printf("[TCP]client addr info: host = %s, port = %hu\r\n", inet_ntoa(sv->clientAddr.sin_addr), ntohs(sv->clientAddr.sin_port));
    return 0;
}

int tcp_server_recv(tcp_server *sv,unsigned char* buf,int maxlen){
    int retval = recv(sv->connfd, buf, maxlen, 0);
    if (retval < 0) {
        tcp_server_disconnect(sv);
        usleep(100000);
        tcp_server_clean(sv);
    }
    return retval;
}

int tcp_server_send(tcp_server *sv,unsigned char* buf,int len){
    int retval = send(sv->connfd, buf, len, 0);
    if (retval <= 0) {
        tcp_server_disconnect(sv);
        tcp_server_clean(sv);
    }
    return retval;    
}

void tcp_client_clean(tcp_client *sv){
    close(sv->sockfd);
    if(sv->connected!=0){
        sv->connected=0;
        if(sv->onConnectionChanged!=NULL)(*sv->onConnectionChanged)(0);
    }
}

unsigned int tcp_client_init(tcp_client *sv,const char* host,unsigned short port,void (*onConnectionChanged)(int)){
    sv->connected=0;
    sv->onConnectionChanged=onConnectionChanged;
    sv->serverAddr.sin_family = AF_INET;    // AF_INET表示IPv4协议
    sv->serverAddr.sin_port = htons(port);  // 端口号，从主机字节序转为网络字节序
    // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
    if (inet_pton(AF_INET, host, &(sv->serverAddr.sin_addr)) <= 0) {
        printf("[TCP]inet_pton failed!\r\n");
        return TCP_ERR_CONV;
    }
    return TCP_SUCCESS;
}

unsigned int tcp_client_connect(tcp_client *sv){
    // 尝试和目标主机建立连接，连接成功会返回0 ，失败返回-1
    ssize_t retval = 0;
    sv->sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    //struct tm tv;
    //tv.tm_sec = 1;
    //setsockopt(sv->sockfd,SOL_SOCKET,SO_RCVTIMEO,(void*)&tv,sizeof(tv));
    if (connect(sv->sockfd, (struct sockaddr *)&(sv->serverAddr), sizeof(sv->serverAddr)) < 0) {
        tcp_client_clean(sv);
        return TCP_ERR_CONNECT;
    }
    if(sv->connected!=1){
        sv->connected=1;
        if(sv->onConnectionChanged!=NULL)(*sv->onConnectionChanged)(1);
    }
    return TCP_SUCCESS;
}

int tcp_client_recv(tcp_client *sv,unsigned char* buf,int maxlen){
    if(!sv->connected) return 0;
    int retval = recv(sv->sockfd, buf, maxlen, 0);
    if (retval == 0) {//-1=Fail,0=Exit
        tcp_client_clean(sv);
    }
    return retval;
}

int tcp_client_send(tcp_client *sv,unsigned char* buf,int len){
    if(!sv->connected) return -1;
    int retval = send(sv->sockfd, buf, len, 0);
    if (retval < 0) {//Fail
        tcp_client_clean(sv);
    }
    return retval;
}