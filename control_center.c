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
#include "task_queue_system.h"
#define SERVER_IP_ADDR "127.0.0.1"
#define SERVER_PORT_START 50627

static Connection connect_list[PROC_NUM];

/**
 * 参数为：ip地址+端口号
 * ip 地址为127.0.0.1,
 */
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

/**
 * 功能：control_center客户端，连接所有的服务器
 * 服务器：ip地址为127.0.0.1，端口号为for循环之中传入的参数
 */
int connectAllServer()
{
	int isFailed = 0;
	for (int i = 0; i < PROC_NUM; i++)
	{
		connect_list[i] = connectToServer(SERVER_PORT_START + i);
		if (connect_list->sockfd < 0)
			isFailed = 1;
	}
	return isFailed;
}

void disconnectAllServer()
{
	for (int i = 0; i < PROC_NUM; i++)
	{
		close(connect_list[i].sockfd);
		connect_list[i].sockfd = -1; // 标记为已关闭
	}
}

/**
 * 功能：将control_center的消息，封装成一个socket_packet
 */
int sendPayloadTo(int partyID, int rtxType, const void *payload, size_t payloadLength)
{
	Socket_Packet_t packet;
	memset(&packet, 0, sizeof(packet));
	packet.header.packetLength = sizeof(Socket_Packet_Header_t) + payloadLength;
	packet.header.type = rtxType; // todo use typedef enum to define meaningful type
	// 将payload封装在packet中的payload发送过去
	strncpy(packet.payload, payload, payloadLength);

	return send_packet(connect_list[partyID], &packet, packet.header.packetLength);
}

/**
 * 功能：从服务器swarm_ranging进程读取socekt_packet消息
 */
ssize_t recvPayloadFrom(int partyID, int *packetType, const void *payload, size_t payloadLength)
{
	Socket_Packet_t packet;
	int payloadLen = -1;
	ssize_t result = receive_packet(connect_list[partyID], &packet, sizeof(packet));
	if (result > 0)
	{
		payloadLen = result - sizeof(packet.header);
		*packetType = packet.header.type;
		strncpy(payload, packet.payload, payloadLen);
		printf("Received packet, with length: %ld, payloadLen=%d\n", result, payloadLen);
	}
	return payloadLen;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	printf("We have a totall of %d CF Proc.\n", PROC_NUM);
	// 打开配置文件
	fp = fopen("simulate_active.conf", "r");
	if (fp == NULL)
	{
		perror("Failed to open file");
		return -1;
	}
	int party_id;
	char rxtx_type[8];
	long long timestamp;
	uint8_t buffer[1024];

	int isFailed = connectAllServer(); // control_center客户端连接所有的服务器
	if (isFailed != 0)
	{
		printf("Connect to Servers failed\n");
		return -1;
	}
	printf("Connect to Servers success\n");

	while (1)
	{
		if (fscanf(fp, "%d\t%s\t0x%llx", &party_id, rxtx_type, &timestamp) == EOF) // 从配置文件之中读取一行数据
			break;
		assert(party_id >= 0);
		assert(party_id < PROC_NUM);
		assert(rxtx_type[0] == 'R' || rxtx_type[0] == 'T');
		assert(rxtx_type[1] == 'X');
		assert(rxtx_type[2] == 0);
		assert(timestamp <= 0xffffffffff);
		printf("----------------------\n");
		printf("%d\t%s\t%llx\n", party_id, rxtx_type, timestamp);

		// 一、control_center进程发给swarm_ranging进程
		UWB_Packet_t packet; // TODO:control_center收到UWB_Packet_t类型的消息
		if (rxtx_type[0] == 'T')
		{
			// 发送时间戳
			int sent_len = sendPayloadTo(party_id, TX_Command, &timestamp, sizeof(timestamp));
			if (sent_len >= 0)
			{
				printf("(Control_Center): Message sent to server %d, with length %d.\n", party_id, sent_len);
			}
			else
			{
				printf("(Control_Center): Failed to send message to server %d\n", party_id);
			}
		}
		else
		{ // rxtx_type[0] == 'R'

			// 1.发送时间戳
			int send_len;
			send_len = sendPayloadTo(party_id, RX_Command, &timestamp, sizeof(timestamp));
			if (send_len >= 0)
			{
				printf("(Control_Center):Timestamp sent to server %d, with length %d.\n", party_id, sent_len);
			}
			else
			{
				printf("(Control_Center):Failed to send timestamp to server %d\n", party_id);
			}

			// 2.发送UWB_Packet_t类型的数据
			send_len = sendPayloadTo(party_id, Send_RangingMessage, &packet, sizeof(packet));
			if (send_len >= 0)
			{
				printf("(Control_Center): UWB_Packet sent to server %d, with length %d.\n", party_id, sent_len);
			}
			else
			{
				printf("(Control_Center): Failed to send UWB_Packet to server %d\n", party_id);
			}
		}

		// 二、control_center进程收到swarm_ranging进程的消息
		int packetType;
		int recv_len = recvPayloadFrom(party_id, &packetType, buffer, sizeof(buffer));
		if (recv_len > 0)
		{
			memcpy(&packet, buffer, sizeof(packet));
			printf("(Swarm_Ranging Send): Received from %d, message: \"%s\"\n", party_id, (char *)buffer);
		}
		else
		{
			printf("(Swarm_Ranging Send): Failed to receive packet from server %d\n", party_id);
		}
	}

	printf("fclose\n");
	fclose(fp);
	printf("disconnectAllServer\n");
	disconnectAllServer();

	return 0;
}