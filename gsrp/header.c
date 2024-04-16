#include "header.h"

int uwbSendPacketBlock(UWB_Packet_t *packet)
{
  return xQueueSendFromISR(rxQueue, packet, portMAX_DELAY);
}