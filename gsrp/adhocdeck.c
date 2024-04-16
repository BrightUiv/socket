#include <stdint.h>
#include <string.h>

#include "adhocdeck.h"
#include "dwTypes.h"
#include "libdw3000.h"
#include "swarm_ranging.h"

#define DEFAULT_RX_TIMEOUT 0xFFFFF

static uint16_t MY_UWB_ADDRESS;
static bool isInit = false;
static TaskHandle_t uwbTaskHandle = 0;
static TaskHandle_t uwbTxTaskHandle = 0;
static SemaphoreHandle_t irqSemaphore;

static QueueHandle_t txQueue;
static xQueueHandle queues[UWB_MESSAGE_TYPE_COUNT];
static UWB_Message_Listener_t listeners[UWB_MESSAGE_TYPE_COUNT];

/* rx buffer used in rx_callback */
static uint8_t rxBuffer[UWB_FRAME_LEN_MAX];

uint16_t uwbGetAddress()
{
	return MY_UWB_ADDRESS;
}

int uwbSendPacket(UWB_Packet_t *packet)
{
	ASSERT(packet);
	return xQueueSend(txQueue, packet, 0);
}

int uwbSendPacketBlock(UWB_Packet_t *packet)
{
	ASSERT(packet);
	return xQueueSend(txQueue, packet, portMAX_DELAY);
}

int uwbSendPacketWait(UWB_Packet_t *packet, int wait)
{
	ASSERT(packet);
	return xQueueSend(txQueue, packet, M2T(wait));
}

int uwbReceivePacket(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet)
{
	ASSERT(packet);
	ASSERT(type < UWB_MESSAGE_TYPE_COUNT);
	return xQueueReceive(queues[type], packet, 0);
}

int uwbReceivePacketBlock(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet)
{
	ASSERT(packet);
	ASSERT(type < UWB_MESSAGE_TYPE_COUNT);
	return xQueueReceive(queues[type], packet, portMAX_DELAY);
}

int uwbReceivePacketWait(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet, int wait)
{
	ASSERT(packet);
	ASSERT(type < UWB_MESSAGE_TYPE_COUNT);
	return xQueueReceive(queues[type], packet, M2T(wait));
}

void uwbRegisterListener(UWB_Message_Listener_t *listener)
{
	ASSERT(listener->type < UWB_MESSAGE_TYPE_COUNT);
	queues[listener->type] = listener->rxQueue;
	listeners[listener->type] = *listener;
}