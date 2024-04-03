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

static Connection connect_list[PROC_NUM + 1];

void connectAllServer(char *ip_addr)
{
	for (int i = 0; i < PROC_NUM; i++)
	{
		connect_list[i] = connect_to_server(ip_addr, 50627 + i);
		if (connect_list[i].sockfd < 0)
		{
			printf("\n Error: Could not create or connect %d socket \n", i);
			continue;
		}else{
			printf("connect to %d server\n",i);
		}
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
	connectAllServer(argv[1]);
	return 0;
}
