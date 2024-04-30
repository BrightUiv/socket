// communication.c
#include "SocketUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/**
 * 功能：客户端尝试向服务器建立连接
 * 1.生成客户端的socket
 * 2.初始化服务器的sev_addr
 * 3.客户端通过connect()连接服务器
 */
Connection connect_to_server(char *server_ip, int server_port)
{
    Connection conn;
    conn.sockfd = socket(AF_INET, SOCK_STREAM, 0); // 客户端的socket套接字
    conn.server_port = server_port;
    strncpy(conn.server_ip, server_ip, INET_ADDRSTRLEN); // 复制16位的字符串

    if (conn.sockfd < 0)
    {
        perror("Socket creation failed");
        conn.sockfd = -1;
        return conn;
    }

    struct sockaddr_in serv_addr; // 服务器地址的结构体
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;          // 表示IPV4，sin:socket internet
    serv_addr.sin_port = htons(server_port); // 将端口号的格式由主机字节顺序，转换为网络字节顺序

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) // 将点分十进制的IP地址字符串转换为用于网络传输的数值格式
    {
        perror("Invalid address/ Address not supported");
        close(conn.sockfd);
        conn.sockfd = -1;
        return conn;
    }

    if (connect(conn.sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // 客户端向服务器发出请求，尝试通信
    {
        perror("Connection Failed");
        close(conn.sockfd);
        conn.sockfd = -1;
    }

    return conn;
}

void send_message(Connection conn, char *message)
{
    if (send(conn.sockfd, message, strlen(message), 0) < 0) // send()函数成功，返回值为实际发送的字节数，失败，返回值为-1
    {
        perror("Send failed");
    }
}

void receive_message(Connection conn)
{
    char buffer[1024] = {0};
    if (recv(conn.sockfd, buffer, sizeof(buffer), 0) < 0) //-1表示接受失败，0表示对端正常关闭，返回实际接收到的字节数。如果这个数字小于 len，可能意味着数据流已经完成
    {
        perror("Receive failed");
    }
    else
    {
        printf("Message from server %s:%d - %s\n", conn.server_ip, conn.server_port, buffer);
    }
}

ssize_t send_packet(Connection conn, const void *packet, size_t packet_size)
{
    ssize_t sent_size = send(conn.sockfd, packet, packet_size, 0);
    printf("sent_size=%ld: send(conn.sockfd=%d, packet[0]=%d, packet_size=%ld, 0)\n", sent_size, conn.sockfd, *(int *)packet, packet_size);
    if (sent_size < 0)
    {
        perror("Send failed");
    }
    return sent_size;
}

ssize_t receive_packet(Connection conn, const void *packet, size_t packet_size)
{
    ssize_t recved_size = recv(conn.sockfd, packet, packet_size, 0);
    printf("recved_size=%ld: recv(conn.sockfd=%d, packet[0]=%d, packet_size=%ld, 0)\n", recved_size, conn.sockfd, *(int *)packet, packet_size);
    if (recved_size < 0)
    {
        perror("Packet Receive failed");
    }
    return recved_size;
}

int socket_send_payload(int sockfd, const void *payload, size_t dataLength)
{
    if (send(sockfd, &dataLength, sizeof(dataLength), 0) == -1)
    {
        perror("Failed to send payload size");
        return -1;
    }

    if (send(sockfd, payload, dataLength, 0) == -1)
    {
        perror("Failed to send payload data");
        return -1;
    }

    return 0;
}

/**
 * 功能：从特定的socket之中读取packet和size，control_center进程调用
 */
int socket_receive_payload(int sockfd, void *packet, size_t *dataLength)
{
    // recv packet length
    size_t size;
    int n = recv(sockfd, &size, sizeof(size), 0);
    if (n < 0)
    {
        perror("Failed to receive payload size");
        return -1;
    }
    else if (n == 0)
    {
        printf("recv end");
        return -1;
    }

    // recv data
    if (recv(sockfd, packet, size, 0) <= 0)
    {
        perror("Failed to receive payload data");
        return -1;
    }

    *dataLength = size;
    return 0;
}
