#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
long long generateTxTimestamp(long long last_timestamp);
int generateRecvFlies(int *ptr_recv, int count_fly, int src_address);
void generateConfActive(FILE *fp, int count_fly);

int main()
{
    FILE *fp_conf;
    fp_conf = fopen("simulate_active.conf", "w");
    if (fp_conf == NULL)
    {
        perror("Unable to write the file\n");
    }

    srand(time(NULL));
    generateConfActive(fp_conf, 3);
    printf("generate success !\n");
    return 0;
}
/**
 * 功能：递增地生成一条TX时间戳,无人机发送的时间时间会略长
 */
long long generateTxTimestamp(long long last_timestamp)
{

    int increment = (int)rand() % 100 + 1;
    return last_timestamp + increment;
}

/**
 * 功能：递增地生成一条RX时间戳,无人机收到测距消息的时间时间
 */
long long generateRxTimestamp(long long last_timestamp)
{

    int increment = (int)rand() % 10 + 1;
    return last_timestamp + increment;
}

/**
 * 功能：随机生成一条测距消息接收的无人机,返回接收测距消息的无人机的数量
 * 参数：接收无人机的数组,无人机的总数，发送无人的src_address
 * 返回值：接收到此条测距消息的无人机数量，并且通过形参传入接收消息无人机的地址数组
 */
int generateRecvFlies(int *ptr_recv, int count_fly, int src_address)
{
    int count_recv = 0;
    memset(ptr_recv, 0, count_fly * sizeof(int));

    for (int i = 0; i < count_fly; i++)
    {
        if (i == src_address)
        {
            continue;
        }
        int value = rand() % 2;
        ptr_recv[i] = value;
        if (value == 1)
        {
            count_recv++;
        }
    }

    return count_recv;
}
/**
 * 功能：生成主动仿真的配置文件
 * 1.打开配置文件
 * 2.轮流生成TX测距消息,对于每条测距消息，for循环生成count_recv条RX测距消息
 */
void generateConfActive(FILE *fp, int count_fly)
{
    int index_src_addr = 0;
    long long last_tx_timestamp = 100000000000;
    long long max_rx_timestamp = 0;

    for (int i = 0; i < 100; i++) // 生成100条TX测距消息
    {
        long long tx_timestamp = generateTxTimestamp(last_tx_timestamp);
        last_tx_timestamp = tx_timestamp;
        fprintf(fp, "%d TX %lld\n", index_src_addr, tx_timestamp);

        int ptr_recv[20];
        int count_recv = generateRecvFlies(ptr_recv, 3, index_src_addr);
        for (int t = 0; t < count_fly; t++)
        {
            if (ptr_recv[t] == 1)
            {
                long long rx_timestamp = generateRxTimestamp(last_tx_timestamp);
                if (rx_timestamp > max_rx_timestamp)
                {
                    max_rx_timestamp = rx_timestamp;
                }
                fprintf(fp, "%d RX %lld\n", t, rx_timestamp);
            }
        }
        if (max_rx_timestamp > last_tx_timestamp)
        {
            last_tx_timestamp = max_rx_timestamp;
        }

        index_src_addr++;
        if (index_src_addr == count_fly)
        {
            index_src_addr = 0;
        }
    }
}