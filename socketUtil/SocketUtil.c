// communication.c
#include "SocketUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

Connection connect_to_server(char *server_ip, int server_port)
{
    Connection conn;
    conn.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    conn.server_port = server_port;
    strncpy(conn.server_ip, server_ip, INET_ADDRSTRLEN);

    if (conn.sockfd < 0)
    {
        perror("Socket creation failed");
        conn.sockfd = -1;
        return conn;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(conn.sockfd);
        conn.sockfd = -1;
        return conn;
    }

    if (connect(conn.sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        close(conn.sockfd);
        conn.sockfd = -1;
    }

    return conn;
}

void send_message(Connection conn, char *message)
{
    if (send(conn.sockfd, message, strlen(message), 0) < 0)
    {
        perror("Send failed");
    }
}

void receive_message(Connection conn)
{
    char buffer[1024] = {0};
    if (recv(conn.sockfd, buffer, sizeof(buffer), 0) < 0)
    {
        perror("Receive failed");
    }
    else
    {
        printf("Message from server %s:%d - %s\n", conn.server_ip, conn.server_port, buffer);
    }
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

int socket_receive_payload(int sockfd, void **packet, size_t *dataLength)
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
        perror("recv end");
        return -1;
    }

    // malloc memory to recv data
    *packet = malloc(size);
    if (*packet == NULL)
    {
        perror("Failed to allocate memory for payload");
        return -1;
    }

    // recv data
    if (recv(sockfd, *packet, size, 0) <= 0)
    {
        perror("Failed to receive payload data");
        free(*packet);
        return -1;
    }

    *dataLength = size;
    return 0;
}
