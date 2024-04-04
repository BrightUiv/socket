#include "task_queue_system.h"
#include<stdlib.h>
#include<stdio.h>

apr_pool_t *pool;

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
                                      const uint32_t uxItemSize){
    apr_queue_t *queue;

    // 初始化 APR 库
    apr_initialize();
    atexit(apr_terminate);

    // 创建内存池
    apr_pool_create(&pool, NULL);

    // 创建队列
    apr_queue_create(&queue, uxQueueLength, pool);

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
    //存入指向Ranging_Message_With_Timestamp_t类型的指针
    Ranging_Message_With_Timestamp_t* ptr=(Ranging_Message_With_Timestamp_t*)malloc(sizeof(Ranging_Message_With_Timestamp_t));
    memcpy(ptr,pvItemToQueue,sizeof(Ranging_Message_With_Timestamp_t));
    apr_queue_push(xQueue, ptr);
    return 1;
}

//从队列之中获取一个元素
BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,//pvBuffer指向一个Ranging_Message_With_Timestamp_t类型的数据
                          TickType_t xTicksToWait ){
    //实现free()函数的机制
    Ranging_Message_With_Timestamp_t* ptr;
    apr_queue_pop(xQueue,(void** )&ptr);//传出的是元素地址（指针）的地址

    //获取的ptr，可以*ptr找到对应的测距消息
    Ranging_Message_With_Timestamp_t* buffer = (Ranging_Message_With_Timestamp_t*)pvBuffer;
    *buffer = *ptr;

    //释放apr_queue队列之中的首地址
    free(ptr);

    return 1;
}


//返回一个互斥锁数据类型
SemaphoreHandle_t xSemaphoreCreateMutex(){
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);  // 使用默认属性初始化互斥锁
    return mutex;
}


//传入一个互斥锁的类型
void xSemaphoreDestroyMutex(SemaphoreHandle_t mutex){
    pthread_mutex_destroy(&mutex);
}



int main(){

    QueueHandle_t xQueue=xQueueCreate(5,0);
    BaseType_t xHigherPriorityTaskWokenValue = 1; // 创建并初始化变量
    BaseType_t * const pxHigherPriorityTaskWoken = &xHigherPriorityTaskWokenValue; // 将变量的地址赋给指针
    Ranging_Message_With_Timestamp_t rxMessageWithTimestamp;
    xQueueSendFromISR(xQueue,&rxMessageWithTimestamp,pxHigherPriorityTaskWoken);//向队列之中存入一个元素
    xQueueReceive(xQueue,&rxMessageWithTimestamp,1);
    xQueueDestroy(pool);
    return 0;
}