#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

void alarm_handler(int sig)
{
    printf("Timer expired可以替换\n");
}

struct itimerval xTimerCreate()
{
    struct itimerval timer;
    return timer;
}

int xTimerStart(struct itimerval timer, int expire_time, int repetition, void *func)
{
    // struct itimerval timer=xTimerCreate();
    struct sigaction sa;

    // 清空并设置信号处理函数
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = func;
    sigaction(SIGALRM, &sa, NULL);

    // 设置定时器的时间间隔
    timer.it_value.tv_sec = expire_time; // 定时器首次超时时间
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = repetition; // 定时器周期性超时时间，设置为0表示单次定时器
    timer.it_interval.tv_usec = 0;

    return setitimer(ITIMER_REAL, &timer, NULL);
}

int main()
{
    printf("Timer is set for 5 seconds.\n");
    struct itimerval timer = xTimerCreate();
    xTimerStart(timer, 5, 2, alarm_handler);

    // 循环以便程序继续运行并等待信号
    while (1)
    {
        usleep(1000000);
    }

    return 0;
}