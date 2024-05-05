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
#include <poll.h>
#include "message_struct.h"
#include "task_queue_system.h"

#define TIMEOUT -1	   // Poll wait forever
#define MAX_CLIENTS 20 // 客户端：swarm_ranging进程最大数量

volatile sig_atomic_t stop; // process ctrl+c
int listenfd = -1;
int portnum = -1;
SemaphoreHandle_t readyToGenerateAndSend;

typedef struct
{
	struct pollfd fds[MAX_CLIENTS + 1]; // 存储网络连接的状态信息
	int count;
} ClientManager;

ClientManager manager;
/**
 * 一个服务器，它需要同时处理多个客户端连接。你可以使用 ClientManager 来跟踪每个客户端的连接状态以及服务器的监听套接字。
 */

/**
 * 功能：初始化ClientManager
 */
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
/**
 * 接收新的客户端端control_center的文件描述符
 */
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
	// 存放在manager的数组之中进行寄存
	manager.count++;
	manager.fds[manager.count].fd = connfd;
	manager.fds[manager.count].events = POLLIN;

	if (manager.count >= MAX_CLIENTS)
	{
		// Reached max clients, reject further connections
		close(connfd);
	}
}

/**
 * 功能：关闭所有的socket连接，通过socket描述符fd来实现
 */
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
/**
 * 功能：程序执行过程中处理特定事件（在这种情况下是信号）的方法
 * SIGINT:CTRL+C终止程序
 */
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

/**
 * 功能：设置一个用于接收客户端连接的套接字
 */
int init_server_socket(int port) // port为绑定的端口号
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0); // 一个文件描述符，用来唯一标识新创建的套接字
	if (listenfd < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr; // 一个结构体，用来保存互联网地址
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;				   // ipv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // host to network long 表示服务器套接字接受所有可用的网络接口上的连接
	// 表示"所有IP地址"或"任意IP地址"
	serv_addr.sin_port = htons(port); // host to network short 提供的端口号从主机字节顺序转换为网络字节顺序（对于短整型）

	// 函数将 serv_addr 中的地址和端口信息绑定到 listenfd 指定的套接字上
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind failed");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

	// 函数设置服务器套接字listenfd为监听状态，MAX_CLIENTS 定义了队列中最多可以有多少客户端等待连接。
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

/**
 * 功能：处理control_center客户端发送过来的socket_packet消息
 * 参数：idx表示swarm_ranging服务器管理的socket连接字
 */
void handle_client_data(int idx)
{
	Connection conn = {.sockfd = manager.fds[idx].fd};
	Socket_Packet_t packet;
	ssize_t result = receive_packet(conn, &packet, sizeof(packet)); // connf表示申请通信的客户端
	if (result >= 0)
	{
		// TODO: 调用rxCallback()
		// 成功接收到消息，打印消息内容
		printf("(%d) received: type=%d, timestamp=0x%llx.\n", portnum, packet.header.type, *(long long *)packet.payload);
		// xSemaphoreGive(readyToGenerateAndSend, portMAX_DELAY);

		// case 1:如果control_center要求发报文，完后才能如下步骤：
		// 0.保存tx_timestamp到临时变量
		// 1.xSemaphoreGive()
		// 	1.1.重写uwbSendBlock()函数，直接通过socket发送，最近版本
		// 2. Delay(1ms)函数
		// 3.调用TxCallback()函数
		//	3.1重写dwt_read_tx_timestamp()函数

		// case 2:如果control_center要求接收报文
		// 0. 保存rx_timestamp到临时变量
		// 1. 接受rangingMessage到临时变量
		// 2. 调用rxCallback()函数处理消息

		//  back message，在此处进行修改需要返回给control_center进程的测距消息
		const char *responseMessage = "Message received successfully";

		Socket_Packet_t responsePacket;
		memset(&responsePacket, 0, sizeof(responsePacket)); // 初始化responsePacket

		responsePacket.header.packetLength = sizeof(Socket_Packet_Header_t) + strlen(responseMessage);
		strncpy(responsePacket.payload, responseMessage, strlen(responseMessage)); // 复制响应消息到payload

		// send back一接收到Packet，立即send给control_center进程
		send_packet(conn, &responsePacket, responsePacket.header.packetLength);
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

/**
 * 功能：处理新的测距消息或者添加新的连接
 */
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
		// while循环之中不断地处理收到的测距消息
		for (int i = 1; i < manager.count + 1; i++)
		{
			if (manager.fds[i].fd != -1 && manager.fds[i].revents & POLLIN)
			{
				handle_client_data(i);
			}
		}
	}
}

//  连续三次启动   ./swarm_ranging_proc server$i $i &
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
	long arg = strtol(argv[2], &p, 10); //$i，10表示十进制，用于端口号
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

	initClientManager(&manager); // 初始化pollfd[]数组
	// initRangingInit();//开了两个线程TX,RX

	setup_signal_handler(); // ctrl+c实现关闭所有套接字的连接

	listenfd = init_server_socket(50627 + portnum); // 设置一个服务器进程swarm_ranging监听所有客户端程序的套接字listenfd

	run_poll_loop(); // 对于swarm_ranging维护的数据结构进行检查

	close(listenfd); // 依据文件描述符，socket连接
}
