#ifndef SOCKET_UTIL
#define SOCKET_UTIL
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

typedef struct
{
    int sockfd;                      // 套接字文件描述符，用于**唯一**标识和管理套接字，ip地址+端口号
    char server_ip[INET_ADDRSTRLEN]; // 服务器IP地址字符串，INET_ADDRSTRLEN 是IP地址字符串的最大长度。
    int server_port;                 // 服务器端口号
} Connection;

Connection connect_to_server(char *server_ip, int server_port);

void send_message(Connection conn, char *message);

void receive_message(Connection conn);

int socket_send_payload(int sockfd, const void *payload, size_t payload_size);

int socket_receive_payload(int sockfd, void **payload, size_t *payload_size);

#endif
