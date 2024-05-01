#if !defined(MESSAGE_STRUCT)
#define MESSAGE_STRUCT
#include <stdint.h>
#include "socketUtil/SocketUtil.h"

enum
{
    TX_Command,           // 隐式枚举，0 swarm_ranging_proc的发送指令
    RX_Command,           // 隐式枚举，1 swarm_ranging_proc的接收指令
    Return_RangingMessage // 隐式枚举，2 无人机返回的rangingMessage测距消息
};

typedef struct
{
    size_t packetLength;
    int type; // 枚举类型,
    // RX指令(control_center发给swarm_ranging_proc)、
    // TX指令(control_center发给swarm_ranging_proc)、
    // 返回的数据(swarm_ranging_proc返回给control_center数据)
} Socket_Packet_Header_t;

typedef struct
{
    Socket_Packet_Header_t header;
    char payload[1000];
} Socket_Packet_t;

#endif // MESSAGE_STRUCT
