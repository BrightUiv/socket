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

volatile sig_atomic_t stop; // process ctrl+c

void handle_sigint(int sig)
{
	stop = 1;
}

int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	char sendBuff[1025];
	time_t ticks;

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

	/* creates an UN-named socket inside the kernel and returns
	 * an integer known as socket descriptor
	 * This function takes domain/family as its first argument.
	 * For Internet family of IPv4 addresses we use AF_INET
	 */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(50627 + portnum);

	/* The call to the function "bind()" assigns the details specified
	 * in the structure 『serv_addr' to the socket created in the step above
	 */
	bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);
	// current need only 1 client to connect
	connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);

	signal(SIGINT, handle_sigint);
	while (!stop)
	{
		Socket_Packet_t *packet;
		int result = recvSocketPacket(connfd,&packet);
		
	}
	if (connfd >= 0)
	{
		close(connfd);
	}

	if (listenfd >= 0)
	{
		close(listenfd);
	}
}
