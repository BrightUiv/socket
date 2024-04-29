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
    char payload[1000];
} Socket_Packet_t;

#endif // MESSAGE_STRUCT
