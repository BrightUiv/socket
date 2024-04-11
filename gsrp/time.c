#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void timer_handler(int sig, siginfo_t *si, void *uc) {
    printf("Timer fired!\n");
}

timer_t xTimerCreate(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;

    sa.sa_sigaction = timer_handler;
    //sa_sigaction字段设置为timer_handler函数的地址。timer_handler是当接收到SIGRTMIN信号时应该被调用的信号处理函数。通过这种方式，你可以自定义信号的处理行为，而不是使用默认的行为（如终止进程）。

    sigaction(SIGRTMIN, &sa, NULL);//将之前设置的信号处理行为应用于SIGRTMIN信号。SIGRTMIN表示实时信号的最小值，它是实时信号范围内的第一个信号。

    struct sigevent se;
    memset(&se, 0, sizeof(se));
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGRTMIN;
    timer_t timer_id;
    timer_create(CLOCK_REALTIME, &se, &timer_id);

    return timer_id;
}

long xTimerStart(timer_t timer_id,int expire_time,int repetition){
    struct itimerspec its;
    its.it_value.tv_sec = expire_time; // 定时器第一次到期的时间,是按照秒来算
    its.it_interval.tv_sec = repetition; // 定时器重复触发的间隔时间,时间周期为2s    
    timer_settime(timer_id, 0, &its, NULL);//设置为0，定时器的到期时间是从现在开始计算的
    return 0;
}
int main(){
    timer_t timer_id=xTimerCreate();
    xTimerStart(timer_id,5,2);
    while(1){
        pause();//用于使调用它的进程挂起（暂停执行），直到该进程捕获到一个信号。一旦捕获到信号，如果该信号的默认行为是终止进程，或者该信号的处理函数返回，则pause()函数调用结束
    }
    return 0;   
}
