#include "task_queue_system.h"
#include<stdlib.h>
#include<stdio.h>

apr_pool_t *pool;
static QueueHandle_t rxQueue;

//--------------------------------------------------------------------------------------
void rangingRxCallback(void *parameters)
{
  // DEBUG_PRINT("rangingRxCallback \n");

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  UWB_Packet_t *packet = (UWB_Packet_t *)parameters;

  dwTime_t rxTime;
//   dwt_readrxtimestamp((uint8_t *)&rxTime.raw);
  Ranging_Message_With_Timestamp_t rxMessageWithTimestamp;
  rxMessageWithTimestamp.rxTime = rxTime;
  Ranging_Message_t *rangingMessage = (Ranging_Message_t *)packet->payload; // 将UWB_Packet类型的packet转换为Ranging_Message类型的
  rxMessageWithTimestamp.rangingMessage = *rangingMessage;

  xQueueSendFromISR(rxQueue, &rxMessageWithTimestamp, &xHigherPriorityTaskWoken);
}

void rangingTxCallback(void *parameters)
{
  UWB_Packet_t *packet = (UWB_Packet_t *)parameters;
  Ranging_Message_t *rangingMessage = (Ranging_Message_t *)packet->payload;

  dwTime_t txTime;
//   dwt_readtxtimestamp((uint8_t *)&txTime.raw);

  Timestamp_Tuple_t timestamp = {.timestamp = txTime, .seqNumber = rangingMessage->header.msgSequence};
//   updateTfBuffer(timestamp);
}
//--------------------------------------------------------------------------------------

//休眠对应的秒数
void vTaskDelay( const TickType_t xTicksToDelay ){
    usleep(1000000 * (xTicksToDelay));
}

//获取当前系统对应的时间，返回的是秒数
TickType_t xTaskGetTickCount( void ){
    struct timespec ts;
    // 获取当前系统时间
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime");
        return 1;
    }
    // 打印当前系统时间
    //时间为--秒+纳秒--
    // printf("Current system time: %ld seconds, %ld nanoseconds\n", ts.tv_sec, ts.tv_nsec);
    //返回对应的秒数
    return ts.tv_sec;
}


/**
    Queue队列为一个指针队列，里面存放的都是指针类型的数据
*/
QueueHandle_t xQueueCreate( const uint32_t uxQueueLength,
                                      const uint32_t uxItemSize){//uxItemSize每个数据类型的长度
    apr_queue_t *queue;

    // 初始化 APR 库
    apr_initialize();
    atexit(apr_terminate);

    // 创建内存池
    apr_pool_create(&pool, NULL);

    // 创建队列
    apr_queue_create(&queue, uxQueueLength, uxItemSize,pool);//修改后源码后的apr_queue_create()函数

    // queue->item_size=uxItemSize;封装直接修改会报错

    return queue;
}


//销毁进程对应的内存池，直接释放内存池中的所有空间
void xQueueDestroy(apr_pool_t* pool){

    apr_pool_destroy(pool);
    apr_terminate();
}


//实现往apr_queue_t队列之中插入一个指针--实现
BaseType_t xQueueSendFromISR( 
    QueueHandle_t xQueue,  
    const void * const pvItemToQueue,//存放的是一级指针
    BaseType_t * const pxHigherPriorityTaskWoken)
{      
    unsigned int item_size=apr_queue_item_size(xQueue);

    //存入指向Ranging_Message_With_Timestamp_t类型的指针
    // Ranging_Message_With_Timestamp_t* ptr=(Ranging_Message_With_Timestamp_t*)malloc(sizeof(Ranging_Message_With_Timestamp_t));

    void * ptr=malloc(item_size);

    // memcpy(ptr,pvItemToQueue,sizeof(Ranging_Message_With_Timestamp_t));
    memcpy(ptr,pvItemToQueue,item_size);

    apr_queue_push(xQueue, ptr);
    return 1;
}

//从队列之中获取一个元素
BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,//pvBuffer指向一个Ranging_Message_With_Timestamp_t类型的数据
                          TickType_t xTicksToWait ){
    //实现free()函数的机制
    // Ranging_Message_With_Timestamp_t* ptr;
    void * ptr;     
    apr_queue_pop(xQueue,(void** )&ptr);//传出的是元素地址（指针）的地址

    //获取的ptr，可以*ptr找到对应的测距消息
    // Ranging_Message_With_Timestamp_t* buffer = (Ranging_Message_With_Timestamp_t*)pvBuffer;
    // *buffer = *ptr; 

    unsigned int item_size=apr_queue_item_size(xQueue);

    memcpy(pvBuffer,ptr,item_size);

    //释放apr_queue队列之中的首地址
    free(ptr);

    return 1;
}


//传出一个互斥锁数据类型
SemaphoreHandle_t xSemaphoreCreateMutex(){
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);  // 使用默认属性初始化互斥锁
    return mutex;
}

//传入一个互斥锁的类型
void xSemaphoreDestroyMutex(SemaphoreHandle_t mutex){
    pthread_mutex_destroy(&mutex);
}

void uwbRegisterListener(UWB_Message_Listener_t *listener) {
//   ASSERT(listener->type < UWB_MESSAGE_TYPE_COUNT);
  queues[listener->type] = listener->rxQueue;
//   listeners[listener->type] = *listener;感觉存在些问题
}

/* apr_queue是不是线程阻塞，队列为空，线程读他是否会阻塞？队列为空，线程真的阻塞，真阻塞之后，读线程在队列之中被放东西后，是否真的解除阻塞真的读取我们存放的东西 */

//实现rangingInit()的功能，
int main(){


    // MY_UWB_ADDRESS = uwbGetAddress();
    rxQueue = xQueueCreate(RANGING_RX_QUEUE_SIZE, RANGING_RX_QUEUE_ITEM_SIZE);
    neighborSetInit(&neighborSet);
    // neighborSetEvictionTimer = xTimerCreate("neighborSetEvictionTimer",
    //                                       M2T(NEIGHBOR_SET_HOLD_TIME / 2),
    //                                       pdTRUE,
    //                                       (void *) 0,
    //                                       neighborSetClearExpireTimerCallback);
    // xTimerStart(neighborSetEvictionTimer, M2T(0));
    rangingTableSetInit(&rangingTableSet);
    // rangingTableSetEvictionTimer = xTimerCreate("rangingTableSetEvictionTimer",
    //                                           M2T(RANGING_TABLE_HOLD_TIME / 2),
    //                                           pdTRUE,
    //                                           (void *) 0,
    //                                           rangingTableSetClearExpireTimerCallback);
    // xTimerStart(rangingTableSetEvictionTimer, M2T(0));
    TfBufferMutex = xSemaphoreCreateMutex();

    listener.type = UWB_RANGING_MESSAGE;
    listener.rxQueue = NULL; // handle rxQueue in swarm_ranging.c instead of adhocdeck.c
    listener.rxCb = rangingRxCallback;
    listener.txCb = rangingTxCallback;
    uwbRegisterListener(&listener);

    idVelocityX = 0;
    idVelocityY = 0;
    idVelocityZ = 0;


    // xTaskCreate(uwbRangingTxTask, ADHOC_DECK_RANGING_TX_TASK_NAME, UWB_TASK_STACK_SIZE, NULL,
    //           ADHOC_DECK_TASK_PRI, &uwbRangingTxTaskHandle);
    // xTaskCreate(uwbRangingRxTask, ADHOC_DECK_RANGING_RX_TASK_NAME, UWB_TASK_STACK_SIZE, NULL,
    //           ADHOC_DECK_TASK_PRI, &uwbRangingRxTaskHandle);




/*     QueueHandle_t xQueue=xQueueCreate(5,0);
    BaseType_t xHigherPriorityTaskWokenValue = 1; // 创建并初始化变量
    BaseType_t * const pxHigherPriorityTaskWoken = &xHigherPriorityTaskWokenValue; // 将变量的地址赋给指针
    Ranging_Message_With_Timestamp_t rxMessageWithTimestamp;
    xQueueSendFromISR(xQueue,&rxMessageWithTimestamp,pxHigherPriorityTaskWoken);//向队列之中存入一个元素
    xQueueReceive(xQueue,&rxMessageWithTimestamp,1);
    xQueueDestroy(pool); */
    return 0;
}

