#include "message_struct.h"

/**
 * 功能：通过调用send()函数，传递测距消息socket_packet
 */
int sendSocketPacket(int sockfd, Socket_Packet_t *packet)
{
    if (send(sockfd, packet, packet->header.packetLength, 0) == -1) // 文件描述符，packet包，头部，标志
    {
        perror("Failed to send Socket_Packet_t *packet");
        return -1;
    }
    return 0;
}

int recvSocketPacket(int sockfd, Socket_Packet_t *packet)
{
    size_t packet_size;

    // recv packet
    return socket_receive_payload(sockfd, (void *)packet, &packet_size);
}