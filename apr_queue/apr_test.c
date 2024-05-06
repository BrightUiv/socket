/* 
    这个文件是用来测试apr_queue队列机制的
    1.是不是线程阻塞，队列为空，线程读他是否会阻塞？
    ---apr_queue指针队列为空，会自动阻塞

    2.队列为空，线程真的阻塞，真阻塞之后，读线程在队列之中被放东西后，是否真的解除阻塞真的读取我们存放的东西
    ----apr_queue队列为空的时候，读线程真实阻塞，写线程写进队列之后，<自动>唤醒一个线程，（满足我们模型的需求），读线程可以正确读取入队元素

    基于假设：队列之中存放的是int类型的数据
 */
 #include<apr_queue.h>
 #include<pthread.h>
 #include<stdlib.h>
 #include<stdio.h>
#include<unistd.h>

 void* consumer(void* args){
    apr_queue_t * queue=(apr_queue_t *)args;

/*     unsigned int size=apr_queue_size(queue);
    printf("队列之中还剩元素 %u 个\n",size); */

    apr_status_t rv;

    int * ptr;
    rv=apr_queue_pop(queue,(void *)&ptr);
    if(rv!=APR_SUCCESS){
        printf("队列之中元素个数为0，安全阻塞\n");
    }

    pthread_t thread_id = pthread_self();
    printf("消费者线程id为：%lu ,出队元素为 %d \n",(unsigned long)thread_id,*ptr);
    
 }
 void * producer(void* args){
    apr_queue_t * queue=(apr_queue_t *)args;
    usleep(5*1000000);//休眠5s保证消费者阻塞

    
    int data=10;
    //------------------------------------------------------------------------------
    //唤醒第一个消费者线程
    apr_status_t rv;
    rv=apr_queue_push(queue,&data);

    unsigned int size2=apr_queue_size(queue);
    pthread_t thread_id = pthread_self();
    printf("生产者线程，线程id为 %lu 插入之后：队列之中还剩元素 %u 个\n",(unsigned long)thread_id,size2);

    //------------------------------------------------------------------------------
    //唤醒第二个消费者线程
    usleep(3*1000000);//休眠5s保证消费者阻塞
    data=11;
    rv=apr_queue_push(queue,&data);
    size2=apr_queue_size(queue);
    thread_id = pthread_self();
    printf("生产者线程，线程id为 %lu 插入之后：队列之中还剩元素 %u 个\n",(unsigned long)thread_id,size2);
    
 }
 int main(){
    apr_initialize();
    
    apr_pool_t* pool;
    apr_pool_create(&pool,NULL);//NULL的位置代表的是父内存池

    apr_queue_t* queue;
    apr_queue_create(&queue,10,4,pool);//可以存放十个指针
    // queue->item_size=10;

    pthread_t th1,th2,th3;

    pthread_create(&th1,NULL,consumer,queue);
    pthread_create(&th2,NULL,consumer,queue);
    pthread_create(&th3,NULL,producer,queue);

    pthread_join(th1,NULL);
    pthread_join(th2,NULL);
    pthread_join(th3,NULL);

    //销毁队列和释放内存池
    apr_queue_term(queue);
    apr_pool_destroy(pool);
    apr_terminate();



    return 0;
 }