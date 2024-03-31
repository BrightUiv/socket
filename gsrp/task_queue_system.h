#ifndef __MYHEAD_H__
#define __MYHEAD_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/**
 * Type by which queues are referenced.  For example, a call to xQueueCreate()
 * returns an QueueHandle_t variable that can then be used as a parameter to
 * xQueueSend(), xQueueReceive(), etc.
 */
struct QueueDefinition; /* Using old naming convention so as not to break kernel aware debuggers. */
typedef struct QueueDefinition   * QueueHandle_t;

typedef union dwTime_u {
  uint8_t raw[5];
  uint64_t full;
  struct {
    uint32_t low32;
    uint8_t high8;
  } __attribute__((packed));
  struct {
    uint8_t low8;
    uint32_t high32;
  } __attribute__((packed));
} dwTime_t;

typedef uint32_t portTickType;

typedef QueueHandle_t SemaphoreHandle_t;

/**
 * Type by which software timers are referenced.  For example, a call to
 * xTimerCreate() returns an TimerHandle_t variable that can then be used to
 * reference the subject timer in calls to other software timer API functions
 * (for example, xTimerStart(), xTimerReset(), etc.).
 */
struct tmrTimerControl; /* The old naming convention is used to prevent breaking kernel aware debuggers. */
typedef struct tmrTimerControl * TimerHandle_t;

/**
 * task. h
 *
 * Type by which tasks are referenced.  For example, a call to xTaskCreate
 * returns (via a pointer parameter) an TaskHandle_t variable that can then
 * be used as a parameter to vTaskDelete to delete the task.
 *
 * \defgroup TaskHandle_t TaskHandle_t
 * \ingroup Tasks
 */
struct tskTaskControlBlock;     /* The old naming convention is used to prevent breaking kernel aware debuggers. */
typedef struct tskTaskControlBlock * TaskHandle_t;

#define M2T(X) ((unsigned int)(X))
#define F2T(X) ((unsigned int)((configTICK_RATE_HZ/(X))))
#define T2M(X) ((unsigned int)(X))

#define ASSERT(e) assert(e)

typedef short BaseType_t;
typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait );
#define xSemaphoreTake( xSemaphore, xBlockTime )    xQueueSemaphoreTake( ( xSemaphore ), ( xBlockTime ) )

BaseType_t xQueueGenericSend( QueueHandle_t xQueue);
#define xSemaphoreGive( xSemaphore )    xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ))

QueueHandle_t xQueueCreateMutex();
#define xSemaphoreCreateMutex()    xQueueCreateMutex()

TickType_t xTaskGetTickCount( void );


void vTaskDelay( const TickType_t xTicksToDelay );

BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,
                          TickType_t xTicksToWait );

#define pdFALSE	0
#define pdTRUE	1

BaseType_t xQueueGenericSendFromISR( QueueHandle_t xQueue,
                                     const void * const pvItemToQueue,
                                     BaseType_t * const pxHigherPriorityTaskWoken);
#define xQueueSendFromISR( xQueue, pvItemToQueue, pxHigherPriorityTaskWoken ) \
    xQueueGenericSendFromISR( ( xQueue ), ( pvItemToQueue ), ( pxHigherPriorityTaskWoken ))


QueueHandle_t xQueueCreate( const uint32_t uxQueueLength,
                                      const uint32_t uxItemSize);

#endif
