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

#define TIMEOUT -1 // Poll wait forever
#define MAX_CLIENTS 1024

typedef struct
{
	struct pollfd fds[MAX_CLIENTS + 1];
	int count;
} ClientManager;

ClientManager manager;

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

	// Find an empty slot in fds
	for (int i = 1; i < MAX_CLIENTS + 1; i++)
	{
		if (manager.fds[i].fd == -1)
		{
			manager.fds[i].fd = connfd;
			manager.fds[i].events = POLLIN;
			manager.count++;
			break;
		}
	}

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
	for (int i = 0; i < MAX_CLIENTS + 1; i++)
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

void close_and_clear_client(int idx)
{
	if (manager.fds[idx].fd >= 0)
	{
		close(manager.fds[idx].fd); // Close the connection
		manager.fds[idx].fd = -1;	// Mark as closed
		manager.count--;
	}
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
	// Set listenfd to manager
	manager.fds[0].fd = listenfd;
	manager.fds[0].events = POLLIN;
	return listenfd;
}

void handle_client_data(int idx)
{
	int connfd = manager.fds[idx].fd;
	Socket_Packet_t *packet = NULL;
	int result = recvSocketPacket(connfd, &packet);
	if (result >= 0 && packet != NULL)
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
				continue; // Interrupted by signal
			break;
		}

		if (manager.fds[0].revents & POLLIN)
		{
			accept_new_connection();
		}

		for (int i = 1; i < MAX_CLIENTS + 1; i++)
		{
			if (manager.fds[i].fd != -1 && manager.fds[i].revents & POLLIN)
			{
				handle_client_data(i);
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

	run_poll_loop(&manager);

	close(listenfd);
}
