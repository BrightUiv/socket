#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include "message_struct.h"
#include <poll.h>

volatile sig_atomic_t stop; // process ctrl+c
int listenfd = -1;
int portnum = -1;

#define TIMEOUT -1	   // Poll wait forever
#define MAX_CLIENTS 20 // 客户端：swarm_ranging进程最大数量

typedef struct
{
	struct pollfd fds[MAX_CLIENTS + 1]; // 存储网络连接的状态信息
	int count;
} ClientManager;

ClientManager manager;

// 初始化ClientManager
void initClientManager(ClientManager *manager)
{
	manager->count = 0;
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		manager->fds[i].fd = -1;
		manager->fds[i].events = 0;
		manager->fds[i].revents = 0;
	}
}

// 接受来自客户端的连接
void accept_new_connection()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
	if (connfd == -1)
	{
		perror("accept");
		return;
	}
	manager.count++;
	manager.fds[manager.count].fd = connfd;
	manager.fds[manager.count].events = POLLIN;

	if (manager.count >= MAX_CLIENTS)
	{
		// Reached max clients, reject further connections
		close(connfd);
	}
}

void handle_sigint(int sig)
{
	stop = 1;

	// Closing all connections
	for (int i = 0; i < manager.count + 1; i++)
	{
		if (manager.fds[i].fd >= 0)
		{
			close(manager.fds[i].fd);
			manager.fds[i].fd = -1; // Mark as closed
		}
	}
}

void setup_signal_handler()
{
	signal(SIGINT, handle_sigint);
}

// 关闭连接
void close_and_clear_client(int idx)
{
	if (manager.fds[idx].fd >= 0)
	{
		close(manager.fds[idx].fd); // Close the connection
		manager.fds[idx].fd = -1;	// Mark as closed
	}
}

// 初始化
int init_server_socket(int port) // port为绑定的端口号
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	// 准备一个套接字，用于TCP网络通信，支持 IPv4 地址
	if (listenfd < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	// 绑定套接字到端口port
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind failed");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

	// 监听套接字
	if (listen(listenfd, MAX_CLIENTS) < 0)
	{
		perror("listen failed");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

	// Set listenfd to manager
	manager.fds[0].fd = listenfd;
	manager.fds[0].events = POLLIN;
	return listenfd;
}

// 处理client客户端发送的消息
void handle_client_data(int idx)
{
	int connfd = manager.fds[idx].fd;
	Socket_Packet_t *packet = NULL;
	int result = recvSocketPacket(connfd, &packet); // 接收消息是Socket_Packet_t类型
	if (result >= 0 && packet != NULL)
	{
		// 成功接收到消息，打印消息内容
		printf("ID %d received message.\n", portnum);

		// back message，在此处进行修改需要返回给control_center进程的测距消息
		const char *responseMessage = "Message received successfully";
		Socket_Packet_t responsePacket;
		memset(&responsePacket, 0, sizeof(responsePacket)); // 初始化responsePacket
		responsePacket.header.packetLength = sizeof(Socket_Packet_Header_t) + strlen(responseMessage);
		strncpy(responsePacket.payload, responseMessage, sizeof(responsePacket.payload) - 1); // 复制响应消息到payload

		// send back一接收到Packet，立即send给control_center进程
		sendSocketPacket(connfd, &responsePacket);
		// free
		free(packet);
	}
	else
	{
		if (result == 0 || errno == ECONNRESET)
		{
			printf("Client disconnected\n");
		}
		else
		{
			// perror("recv error");
		}
		close_and_clear_client(idx);
	}
}

void run_poll_loop()
{
	while (!stop)
	{
		int ret = poll(manager.fds, manager.count + 1, TIMEOUT); // +1 for listenfd
		if (ret < 0)
		{
			perror("poll");
			if (errno == EINTR)
			{
				continue; // Interrupted by signal
			}
			break;
		}

		if (manager.fds[0].revents & POLLIN)
		{
			accept_new_connection();
		}

		for (int i = 1; i < manager.count + 1; i++)
		{
			if (manager.fds[i].fd != -1 && manager.fds[i].revents & POLLIN)
			{
				handle_client_data(i);
			}
		}
	}
}

int main(int argc, char *argv[]) // argc表示参数的数量，argv记录对应的参数
{
	if (argc < 3)
	{
		printf("Error: not enough arg.\n");
		return 1; // not enough arg
	}
	if (strlen(argv[1]) == 0 || strlen(argv[2]) == 0)
	{
		printf("Error: empty string.\n");
		return 1; // empty string
	}
	char *procname = argv[1]; // server$i
	char *p;
	errno = 0;							// not 'int errno', because the '#include' already defined it
	long arg = strtol(argv[2], &p, 10); //$i
	if (*p != '\0' || errno != 0)
	{
		printf("Error: port num.\n");
		return 1; // In main(), returning non-zero means failure
	}

	if (arg < 0 || arg > 1000)
	{
		printf("Error: port num range.\n");
		return 1;
	}
	portnum = arg;

	// Everything went well, print it as a regular number plus a newline
	// 涉及到server$i $i
	printf("The proc name: %s\nThe port num: %d\n", procname, portnum);

	initClientManager(&manager);

	setup_signal_handler();

	listenfd = init_server_socket(50627 + portnum);

	run_poll_loop(&manager);

	close(listenfd);
}
