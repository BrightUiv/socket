#include "message_struct.h"

int sendSocketPacket(int socketfd, Socket_Packet_t *packet)
{
    int result = socket_send_payload(socketfd, packet, packet->header.length);
    return result;
}

int recvSocketPacket(int socketfd, Socket_Packet_t **packet)
{
    int result = socket_receive_payload(socketfd,(void **)packet,&((*packet)->header.length));
    return result;
}