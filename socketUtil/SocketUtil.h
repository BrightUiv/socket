#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 10

#include <stddef.h> // For size_t

typedef struct {
    int sockfd;
    char server_ip[INET_ADDRSTRLEN];
    int server_port;
} Connection;

// 创建与服务器的连接并返回一个Connection结构
Connection connect_to_server(char *server_ip, int server_port);

// 发送消息到指定的连接
void send_message(Connection conn, char *message);

// 接收来自指定连接的消息
void receive_message(Connection conn);

// 通用的发送函数
int send_payload(int sockfd, const void *payload, size_t payload_size);

// 通用的接收函数
int receive_payload(int sockfd, void **payload, size_t *payload_size);