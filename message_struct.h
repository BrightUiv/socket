#if !defined(MESSAGE_STRUCT)
#define MESSAGE_STRUCT
#include <stdint.h>
#include "socketUtil/SocketUtil.h"

typedef struct
{
    size_t packetLength;
    int type;
} Socket_Packet_Header_t;

typedef struct
{
    Socket_Packet_Header_t header;
    char payload[2048];
} Socket_Packet_t;

int sendSocketPacket(int sockfd, Socket_Packet_t *packet);

int recvSocketPacket(int sockfd, Socket_Packet_t **packet);

#endif // MESSAGE_STRUCT
