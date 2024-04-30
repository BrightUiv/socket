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

// 需要重新调整位置
int getIntByIndex(char buffer[], int count_num, const char *delimiter)
{
	char buffer_copy[1024];
	// 复制字符串到新的内存位置
	strcpy(buffer_copy, buffer);

	char *token;   // 分隔符之间的字符
	int count = 0; // 重置计数器

	// 使用strtok分割字符串
	token = strtok(buffer_copy, delimiter);
	while (token != NULL)
	{
		count++;
		if (count == count_num)
		{
			break;
		}
		token = strtok(NULL, delimiter); // 继续分割剩余的字符串
	}
	// printf("Location %d  return   %s\n", count_num, token);

	return atoi(token);
}

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
int sendToServer(int partyID, int rtxType, size_t payloadLength, const char *payload)
{
	Socket_Packet_t packet;
	memset(&packet, 0, sizeof(packet));
	packet.header.packetLength = sizeof(Socket_Packet_Header_t) + payloadLength;
	packet.header.type = rtxType; // todo use typedef enum to define meaningful type
	// 将payload封装在packet中的payload发送过去
	strncpy(packet.payload, payload, payloadLength);

	return sendSocketPacket(connect_list[partyID].sockfd, &packet);
}
 */

/**
 * 功能：从服务器swarm_ranging进程读取socekt_packet消息
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
*/
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

		/**
		 * 解析simulate.conf文件的一行
		 * 得到：{srcAddr---TX/RX---timestamp}---Packet
		 *
		 */
		// printf("before continue\n");
		// continue;
		// printf("after continue\n");
		//  向服务器发送消息，主要是payload的部分
		//  party_id对应src_addr,rxtx_type[0]传递的是ASCII值
		int sent_len = sendPayloadTo(party_id, rxtx_type[0], &timestamp, sizeof(timestamp));
		if (sent_len >= 0)
		{
			printf("Message sent to server %d, with length.\n", party_id, sent_len);
		}
		else
		{
			printf("Failed to send message to server %d\n", party_id);
		}

		// 一发送完就尝试从服务器接收消息
		int packetType;
		int recv_len = recvPayloadFrom(party_id, &packetType, buffer, sizeof(buffer));
		if (recv_len > 0)
		{
			printf("(Control): received from %d, message: \"%s\"\n", party_id, (char *)buffer);
		}
		else
		{
			printf("Failed to receive packet from server %d\n", party_id);
		}
	}

	printf("fclose\n");
	fclose(fp);
	printf("disconnectAllServer\n");
	disconnectAllServer();

	return 0;
}