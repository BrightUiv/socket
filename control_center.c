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
#include <assert.h>
#include "socketUtil/SocketUtil.h"
#include "message_struct.h"
#define SERVER_IP_ADDR "127.0.0.1"
#define SERVER_PORT_START 50627

static Connection connect_list[PROC_NUM];

Connection connectToServer(int port)
{
	Connection conn = connect_to_server(SERVER_IP_ADDR, port);
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

void connectAllServer()
{
	for (int i = 0; i < PROC_NUM; i++)
	{
		connect_list[i] = connectToServer(SERVER_PORT_START + i);
	}
}

void disconnectAllServer()
{
	for (int i = 0; i < PROC_NUM; i++)
	{
		close(connect_list[i].sockfd);
		connect_list[i].sockfd = -1; // 标记为已关闭
	}
}

int sendToServer(int partyID, int rtxType, size_t payloadLength, const char *payload)
{
	Socket_Packet_t packet;
	memset(&packet, 0, sizeof(packet));
	packet.header.packetLength = sizeof(Socket_Packet_Header_t) + payloadLength;
	packet.header.type = rtxType; // todo use typedef enum to define meaningful type
	strncpy(packet.payload, payload, payloadLength);

	return sendSocketPacket(connect_list[partyID].sockfd, &packet);
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

int main(int argc, char *argv[])
{
	FILE *fp;
	printf("We have a totall of %d CF Proc.\n", PROC_NUM);

	fp = fopen("simulate_active.conf", "r");
	int party_id;
	char rxtx_type[8];
	long long timestamp;

	connectAllServer();

	while (1)
	{
		if (fscanf(fp, "%d\t%s\t%llx", &party_id, rxtx_type, &timestamp) == EOF)
			break;
		assert(party_id >= 0);
		assert(party_id < PROC_NUM);
		assert(rxtx_type[0] == 'R' || rxtx_type[0] == 'T');
		assert(rxtx_type[1] == 'X');
		assert(rxtx_type[2] == 0);
		assert(timestamp <= 0xffffffffff);
		printf("--------\n");
		printf("%d\t%s\t%llx\n", party_id, rxtx_type, timestamp);
		// 向服务器发送消息
		if (sendToServer(party_id, rxtx_type[0], sizeof(timestamp), (char *)&timestamp) == 0)
		{
			printf("Message sent to server %d\n", party_id);
		}
		else
		{
			printf("Failed to send message to server %d\n", party_id);
		}

		// 尝试从服务器接收消息
		if (recvFromServer(connect_list[party_id].sockfd) == 0)
		{
			printf("Message received from server %d\n", party_id);
		}
		else
		{
			printf("Failed to receive message from server %d\n", party_id);
		}
	}

	fclose(fp);
	disconnectAllServer();

	return 0;
}
