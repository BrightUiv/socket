#ifndef __MYHEAD_H__
#define __MYHEAD_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include<apr_queue.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include"swarm_ranging.h"
/**
 * Type by which queues are referenced.  For example, a call to xQueueCreate()
 * returns an QueueHandle_t variable that can then be used as a parameter to
 * xQueueSend(), xQueueReceive(), etc.
 */
// struct QueueDefinition; /* Using old naming convention so as not to break kernel aware debuggers. */


#define pdFALSE	0
#define pdTRUE	1

#define M2T(X) ((unsigned int)(X))
#define F2T(X) ((unsigned int)((configTICK_RATE_HZ/(X))))
#define T2M(X) ((unsigned int)(X))

#define ASSERT(e) assert(e)

#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

typedef apr_queue_t* QueueHandle_t;
// typedef pthread_mutex_t SemaphoreHandle_t;

// 



typedef uint32_t portTickType;

//不需要
// typedef QueueHandle_t SemaphoreHandle_t;


/**
 * Type by which software timers are referenced.  For example, a call to
 * xTimerCreate() returns an TimerHandle_t variable that can then be used to
 * reference the subject timer in calls to other software timer API functions
 * (for example, xTimerStart(), xTimerReset(), etc.).
 */
// struct tmrTimerControl; /* The old naming convention is used to prevent breaking kernel aware debuggers. */
// typedef struct tmrTimerControl * TimerHandle_t;

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
// typedef struct tskTaskControlBlock * TaskHandle_t;

typedef short BaseType_t;

//实现--自己修改
// typedef uint32_t TickType_t;
typedef long TickType_t;

// BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait );
// #define xSemaphoreTake( xSemaphore, xBlockTime )    xQueueSemaphoreTake( ( xSemaphore ), ( xBlockTime ) )


// BaseType_t xQueueGenericSend( QueueHandle_t xQueue);
// #define xSemaphoreGive( xSemaphore )    xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ))


//实现
SemaphoreHandle_t xSemaphoreCreateMutex();

//自己添加，实现
void xSemaphoreDestroyMutex(SemaphoreHandle_t mutex);

//实现
TickType_t xTaskGetTickCount( void );

//实现
void vTaskDelay( const TickType_t xTicksToDelay );

// BaseType_t  ( QueueHandle_t xQueue, void * const pvBuffer,TickType_t xTicksToWait );

//待优化：设置为从apr_queue队列之中获取队首的元素指针，目前为rangingMessage_with_timestamp类型
BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,
                          TickType_t xTicksToWait );

//待修改:传入一个队列的长度和每个节点元素的大小
QueueHandle_t xQueueCreate( const uint32_t uxQueueLength,//指针队列的大小
                                      const uint32_t uxItemSize);//uxItemSize没用

//自己添加，实现，销毁内存池，释放所需要的所有资源
void xQueueDestroy(apr_pool_t* pool);

//待优化：使用xQueueSend()往队列之中存放数据
BaseType_t xQueueSendFromISR(  QueueHandle_t xQueue,
                                     const void * const pvItemToQueue,
                                     BaseType_t * const pxHigherPriorityTaskWoken);
#endif
