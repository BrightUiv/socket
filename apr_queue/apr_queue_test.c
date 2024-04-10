// #include <apr_general.h>
#include <apr_queue.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    apr_pool_t *pool;
    apr_queue_t *queue;
    void *elem;
    
    // 初始化 APR
    apr_initialize();
    atexit(apr_terminate);
    apr_pool_create(&pool, NULL);
    
    // 创建队列，大小为 5
    apr_queue_create(&queue, 5, pool);
    
    // 向队列中添加元素
    const char *msg = "Hello APR Queue";
    apr_queue_push(queue, (void*)msg);
    
    // 从队列中取出元素
    if(apr_queue_pop(queue, &elem) == APR_SUCCESS) {
        printf("Popped from queue: %s\n", (char*)elem);
    }
    
    // 清理
    apr_pool_destroy(pool);
    printf("hello world");
    return 0;
}
