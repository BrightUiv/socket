#if !defined(MESSAGE_STRUCT)
#define MESSAGE_STRUCT
#include <stdint.h>
#include "socketUtil/SocketUtil.h"

enum
{
    Command_TX,           // 隐式枚举，0 control_center对swarm_ranging_proc的发送指令
    Command_RX,           // 隐式枚举，1 control_center对swarm_ranging_proc的接收指令
    RangingMessage_Send,  // 隐式枚举，2 control_center进程发送给swarm_ranging进程的packet
    RangingMessage_Return // 隐式枚举，3 swarm_ranging进程返回给control_center进程的packet
};

typedef struct
{
    size_t packetLength;
    int type; // 枚举类型,

} Socket_Packet_Header_t;

typedef struct
{
    Socket_Packet_Header_t header;
    char payload[1000];
} Socket_Packet_t;

#endif // MESSAGE_STRUCT
