#include "message_struct.h"

int sendSocketPacket(int sockfd, Socket_Packet_t *packet)
{
    int result = socket_send_payload(sockfd, packet,
                                     packet->header.packetLength);
    return result;
}

int recvSocketPacket(int sockfd, Socket_Packet_t **packet)
{
    size_t packet_size;

    // recv packet
    return socket_receive_payload(sockfd, (void **)packet, &packet_size);
}