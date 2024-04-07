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
#include <sys/epoll.h>

volatile sig_atomic_t stop; // process ctrl+c
int listenfd = -1;

#define MAX_CLIENTS 1024

typedef struct
{
	int fds[MAX_CLIENTS];
	int count;
} ClientManager;

ClientManager manager;

void initClientManager(ClientManager *manager)
{
	manager->count = 0;
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		manager->fds[i] = -1; // 初始化为无效的文件描述符
	}
}

void accept_new_connection(int epoll_fd, int listenfd, ClientManager *manager)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
	if (connfd == -1)
	{
		perror("accept");
		return;
	}

	if (manager->count < MAX_CLIENTS)
	{
		manager->fds[manager->count++] = connfd;
	}
	else
	{
		// 超出最大客户端连接数，关闭新的连接
		close(connfd);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = connfd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1)
	{
		perror("epoll_ctl: add connfd");
		close(connfd);
	}
}

void handle_sigint(int sig)
{
	stop = 1;

	// 遍历客户端连接并关闭它们
	for (int i = 0; i < manager.count; i++)
	{
		if (manager.fds[i] != -1)
		{
			close(manager.fds[i]);
		}
	}
}

void setup_signal_handler()
{
	signal(SIGINT, handle_sigint);
}

int init_server_socket(int port)
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
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

	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind failed");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

	if (listen(listenfd, 20) < 0)
	{
		perror("listen failed");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

	return listenfd;
}

void handle_client_data(int epoll_fd, int connfd)
{
	Socket_Packet_t *packet = NULL;
	int result = recvSocketPacket(connfd, &packet);
	if (result == 0 && packet != NULL)
	{
		// 成功接收到消息，打印消息内容
		printf("from %d Received message: %s\n", connfd, packet->payload);

		// back message
		const char *responseMessage = "Message received successfully";
		Socket_Packet_t responsePacket;
		memset(&responsePacket, 0, sizeof(responsePacket)); // 初始化responsePacket
		responsePacket.header.packetLength = sizeof(Socket_Packet_Header_t) + strlen(responseMessage);
		strncpy(responsePacket.payload, responseMessage, sizeof(responsePacket.payload) - 1); // 复制响应消息到payload

		// send back
		sendSocketPacket(connfd, &responsePacket);
		// free
		free(packet);
	}
	else
	{
		// 处理错误或关闭连接
		if (result == -1)
		{
			perror("client close connection");
		}

		// 从epoll的监听列表中移除并关闭连接
		if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connfd, NULL) == -1)
		{
			perror("epoll_ctl: remove connfd failed");
		}
		close(connfd);
	}
}

int init_epoll(int listenfd)
{
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1 failed");
		close(listenfd); // 确保在退出前关闭listenfd
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
	{
		perror("epoll_ctl failed");
		close(listenfd);
		close(epoll_fd);
		exit(EXIT_FAILURE);
	}

	return epoll_fd;
}

void run_epoll_loop(int epoll_fd, int listenfd, ClientManager *manager)
{
	const int MAX_EVENTS = 10;
	struct epoll_event events[MAX_EVENTS];

	while (!stop)
	{
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1)
		{
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == listenfd)
			{
				accept_new_connection(epoll_fd, listenfd, manager);
			}
			else
			{
				handle_client_data(epoll_fd, events[n].data.fd);
			}
		}
	}
}

int main(int argc, char *argv[])
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
	char *procname = argv[1];
	char *p;
	errno = 0; // not 'int errno', because the '#include' already defined it
	long arg = strtol(argv[2], &p, 10);
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
	int portnum = arg;

	// Everything went well, print it as a regular number plus a newline
	printf("The proc name: %s\nThe port num: %d\n", procname, portnum);

	initClientManager(&manager);

	setup_signal_handler();
	listenfd = init_server_socket(50627 + portnum);
	int epoll_fd = init_epoll(listenfd);

	run_epoll_loop(epoll_fd, listenfd, &manager);

	close(listenfd);
	close(epoll_fd);
}
