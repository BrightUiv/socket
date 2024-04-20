#include "message_struct.h"

int sendSocketPacket(int sockfd, Socket_Packet_t *packet)
{
    if (send(sockfd, packet, packet->header.packetLength, 0) == -1)
    {
        perror("Failed to send Socket_Packet_t *packet");
        return -1;
    }
    return 0;
}

int recvSocketPacket(int sockfd, Socket_Packet_t **packet)
{
    size_t packet_size;

    // recv packet
    return socket_receive_payload(sockfd, (void **)packet, &packet_size);
}