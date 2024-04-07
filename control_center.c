#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "socketUtil/SocketUtil.h"
#include "message_struct.h"
#define SERVER_PORT_START 50627

static Connection connect_list[PROC_NUM + 1];

Connection connectToServer(char *ip_addr, int port)
{
	Connection conn = connect_to_server(ip_addr, port);
	if (conn.sockfd < 0)
	{
		printf("\n Error: Could not create or connect socket to port %d\n", port);
	}
	else
	{
		printf("Connected to server on port %d\n", port);
	}
	return conn;
}

void connectAllServer(char *ip_addr)
{
	for (int i = 0; i < PROC_NUM; i++)
	{
		connect_list[i] = connectToServer(ip_addr, SERVER_PORT_START + i);
	}
}

int sendToServer(int sockfd, const char *message)
{
	Socket_Packet_t packet;
	memset(&packet, 0, sizeof(packet));
	packet.header.packetLength = sizeof(Socket_Packet_Header_t) + strlen(message);
	strncpy(packet.payload, message, sizeof(packet.payload) - 1);

	return sendSocketPacket(sockfd, &packet);
}

int recvFromServer(int sockfd)
{
	Socket_Packet_t *packet = NULL;
	int result = recvSocketPacket(sockfd, &packet);
	if (result == 0 && packet != NULL)
	{
		// 成功接收到消息，处理消息...
		printf("Received message: %s\n", packet->payload);
		free(packet); // 记得释放分配的内存
		return 0;
	}
	else
	{
		// 接收失败
		return -1;
	}
}

void communicateWithAllServers(char *ip_addr, const char *message)
{
	for (int i = 0; i < PROC_NUM; i++)
	{
		connect_list[i] = connectToServer(ip_addr, 50627 + i);
		if (connect_list[i].sockfd < 0)
		{
			printf("\n Error: Could not create or connect to server %d \n", i);
			continue;
		}
		printf("Connected to server %d\n", i);

		// 向服务器发送消息
		if (sendToServer(connect_list[i].sockfd, message) == 0)
		{
			printf("Message sent to server %d\n", i);
		}
		else
		{
			printf("Failed to send message to server %d\n", i);
		}

		// 尝试从服务器接收消息
		if (recvFromServer(connect_list[i].sockfd) == 0)
		{
			printf("Message received from server %d\n", i);
		}
		else
		{
			printf("Failed to receive message from server %d\n", i);
		}

		// 确保连接被关闭
		close(connect_list[i].sockfd);
		connect_list[i].sockfd = -1; // 标记为已关闭
	}
}

int main(int argc, char *argv[])
{
	printf("We have a totall of %d CF Proc.\n", PROC_NUM);
	if (argc != 2)
	{
		printf("\n Usage: %s <ip of server> \n", argv[0]);
		return 1;
	}
	char message[10] = "hello";
	communicateWithAllServers(argv[1], message);
	return 0;
}
