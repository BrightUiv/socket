#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
int getIntByIndex(char buffer[], int count_num, const char *delimiter);
int findStartIndex(FILE *fp, int set[], int number);
bool checkIsReceived(char *buffer, int count_fly, int src_addr, int msg_seq);
void getLossFlies(FILE *fp, char line[], char *out_str, int count_fly);
int getCountPart(char line[], const char *delimiter);
int getCountBodyUnit(char line[], const char *delimiter);
bool getOneRecvInfo(char *buffer, char *out_str, int count_fly, int src_addr, int msg_seq);
char *getRecvFlies(FILE *fp, char line[], int count_fly);
int getCountFlies(FILE *fp);
double getLatestTxTimestamp(FILE *fp, int src_addr, int seq_number, int count_fly);
void generateConfig(FILE *fp);
char *getTxMessage(FILE *fp, char line[], int count_fly);
int getCountRecvFlies(FILE *fp, char line[], int count_fly);

char buffer[1024];  // 用于存储文件中的一行
int set[100] = {0}; // 用于记录所有无人机的srcAddress

int main()
{
    FILE *fp;
    const char *delimiter = ","; // CSV文件的分隔符

    int count;
    int count_fly = 0; // 记录无人机的数量

    // 打开文件
    fp = fopen("output.csv", "r");
    if (fp == NULL)
    {
        perror("Unable to open file");
        return 1;
    }

    count_fly = getCountFlies(fp);
    printf("Count of fly is %d\n", count_fly);

    // 重置文件指针以重新读取文件
    fseek(fp, 0, SEEK_SET);

    findStartIndex(fp, set, 0);
    findStartIndex(fp, set, 1);
    findStartIndex(fp, set, 2);

    int count_line = 0;
    int count_target_line = 15;
    char out_str[1024];

    fseek(fp, 0, SEEK_SET);
    printf("---------------------------------------------------\n");
    generateConfig(fp);
    // while (fgets(buffer, sizeof(buffer), fp) != NULL)
    // {
    //     count_line++;
    //     if (count_line >= 12)
    //     {
    //         // // getLossFlies(fp, buffer, out_str, 3);
    //         // getRecvFlies(fp, buffer, out_str, count_fly);
    //         // printf("count_line : %d  out_str: %s \n", count_line, out_str);

    //         generateConfig(fp);
    //     }
    // }

    // 关闭文件
    fclose(fp);
    return 0;
}

/**
 *功能：从字符数组之中，获取第count_num个，使用delimiter分割的字符，并且以int类型返回
 */
int getIntByIndex(char buffer[], int count_num, const char *delimiter)
{
    char buffer_copy[1024];
    // 复制字符串到新的内存位置
    strcpy(buffer_copy, buffer);

    char *token;   // 分隔符之间的字符
    int count = 0; // 重置计数器

    // 使用strtok分割字符串
    token = strtok(buffer_copy, delimiter);
    while (token != NULL)
    {
        count++;
        if (count == count_num)
        {
            break;
        }
        token = strtok(NULL, delimiter); // 继续分割剩余的字符串
    }

    return atoi(token);
}

/**
 * 功能：返回对应位置的double类型数据
 */
double getDoubleByIndex(char buffer[], int count_num, const char *delimiter)
{
    char buffer_copy[1024];
    // 复制字符串到新的内存位置
    strcpy(buffer_copy, buffer);

    char *token;   // 分隔符之间的字符
    int count = 0; // 重置计数器

    // 使用strtok分割字符串
    token = strtok(buffer_copy, delimiter);
    while (token != NULL)
    {
        count++;
        if (count == count_num)
        {
            break;
        }
        token = strtok(NULL, delimiter); // 继续分割剩余的字符串
    }

    return strtod(token, NULL);
}

/**
 *获取csv文件之中一架无人机开始开始连续发送测距消息的首地址
 同时至少有100个连续的测距消息
 */
int findStartIndex(FILE *fp, int set[], int number)
{

    int previous_src; // 前一个数字
    int current_src;
    int consecutive_lines = 0; // 连续行数
    int start_index;           // 连续行的开始索引
    int current_index;         // 当前行的索引（从1开始计数）
    int flag = 0;
    char line[1024]; // 用于读取一行

    // 下面开始处理测距文件：判断每条测距消息到底多少无人机收到，多少无人机没收到
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        // 处理具体set[0]无人机
        if (getIntByIndex(line, 7, ",") == set[number])
        {
            if (flag == 0) // 无人机的第一行
            {
                flag = 1;
                previous_src = getIntByIndex(line, 8, ",");
                start_index = getIntByIndex(line, 11, ",");
                consecutive_lines = 1;
                continue;
            }

            current_src = getIntByIndex(line, 8, ",");
            current_index = getIntByIndex(line, 11, ",");

            if (previous_src + 1 == current_src)
            {
                consecutive_lines++;
            }
            else
            {
                if (consecutive_lines >= 100)
                {
                    printf("Fly %d starts at line index %d.\n", set[number], start_index);
                    break;
                }
                consecutive_lines = 1;       // 重置连续行数
                start_index = current_index; // 更新开始索引
            }
            previous_src = current_src;
        }
    }
    if (consecutive_lines >= 100)
    {
        printf("Fly %d starts at line index %d.\n", set[number], start_index);
    }
    fseek(fp, 0, SEEK_SET);
    return start_index;
}

/**
 * 1.生成TX消息，
 * 2.生成RX消息
 * 3.一条TX消息下面，多条RX消息
 */

void generateConfig(FILE *fp)
{

    // 其中fp指向文件的首行
    int count_fly = getCountFlies(fp);
    FILE *fp_ret; // 返回conf文件指针
    fp_ret = fopen("simulate.conf", "w");
    if (fp_ret == NULL)
    {
        perror("Unable to open the conf\n");
    }

    int count_line = 0;

    // 生成一个.conf文件
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        count_line++;
        if (count_line >= 12)
        {
            int index = getIntByIndex(buffer, 11, ",");
            // 解析一行测距消息，生成TX消息，需要srcAddr、tx_timestamp、
            char *ptr_tx = getTxMessage(fp, buffer, count_fly);
            fprintf(fp_ret, "%s\n", ptr_tx);
            // 对于TX的测距生成和接收无人机数量相等的RX测距消息
            int count_recv_flies = getCountRecvFlies(fp, buffer, count_fly);
            char *ptr = getRecvFlies(fp, buffer, count_fly);
            // printf("count_line %d   content is %s\n", count_line, ptr);
            int index_r = 1;
            for (int i = 0; i < count_recv_flies; i++)
            {
                int src_addr = getIntByIndex(ptr, index_r, ",");
                double rx_timestamp = getDoubleByIndex(ptr, index_r + 1, ",");
                fprintf(fp_ret, "%d RX %f\n", src_addr, rx_timestamp);
                // printf("Line %d  %d RX %f\n", count_line, src_addr, rx_timestamp);
                index_r = index_r + 2;
            }
            free(ptr);
        }
    }
    printf("count_line is %d \n", count_line);
    // return fp_ret;
}

/**
 * 返回一个Tx的测距消息的配置文件的char类型指针
 * 注意：每次调用getTxMessage()函数的时候，指针已经指向下面一行了
 */
char *getTxMessage(FILE *fp, char line[], int count_fly)
{
    char *ptr_tx = malloc(100 * sizeof(char));
    ptr_tx[0] = '\0';

    // 1.获取srcAddr
    int count_number = (count_fly - 1) * 3;
    int src_addr = getIntByIndex(line, count_number + 1, ",");
    char temp[50];
    sprintf(temp, "%d", src_addr);
    strcat(ptr_tx, temp); // strcat()结尾会自动加上'\0'

    // 2.加上'TX'
    strcat(ptr_tx, " TX ");

    // 3.获取tx_timestamp
    // int seq_number = getIntByIndex(line, count_number + 2, ",");
    int seq_number = getIntByIndex(line, 8, ",");
    // printf("seq_number is  %d\n", seq_number);
    double tx_timestamp = getLatestTxTimestamp(fp, src_addr, seq_number, count_fly);
    sprintf(temp, "%f", tx_timestamp);
    strcat(ptr_tx, temp);

    return ptr_tx;
}

/**
 * 参数：FILE* fp、int src_addr、int seq_number
 * 1.从当前文件指针的位置，开始往下一个此架无人机发送的测距消息，并且获取lastTimestamp
 */
double getLatestTxTimestamp(FILE *fp, int src_addr, int seq_number, int count_fly)
{
    double tx_timestamp = 0;
    char buffer[500];
    long long initial_pos = ftell(fp); // 获取文件当前的位置

    int index_body_unit = 1;
    for (int j = 0; j <= count_fly + 3; j++) // TODO:这个地方存在点问题，多往下几行
    {
        if (fgets(buffer, sizeof(buffer), fp) != NULL)
        {

            int addr = getIntByIndex(buffer, 7, ",");
            int seq_number_last = getIntByIndex(buffer, 10, ",");
            // printf("addr is: %d seq_number_last is %d  src_addr is %d seq_number is %d\n", addr, seq_number_last, src_addr, seq_number);
            if (addr == src_addr && seq_number_last == seq_number)
            {
                tx_timestamp = getDoubleByIndex(buffer, 9, ",");
                fseek(fp, initial_pos, SEEK_SET);

                return tx_timestamp;
                // printf("timestamp is %f\n", tx_timestamp);
            }
        }
        else
        {
            printf("reach the end of the file\n");
        }
    }

    fseek(fp, initial_pos, SEEK_SET);
    return -1111.111111;
}

int getCountRecvFlies(FILE *fp, char line[], int count_fly)
{
    int count_recv_flies = 0;

    char *ptr_ret = malloc(512 * sizeof(char));
    if (fp == NULL)
    {
        printf("File pointer is null.\n");
        return -1;
    }

    ptr_ret[0] = '\0'; // 初始化输出字符串

    long initial_pos = ftell(fp); // 获取文件的当前位置
    /* TODO:定位msg_seq的时候 */
    int msg_seq = getIntByIndex(line, 8, ",");
    int src_addr = getIntByIndex(line, 7, ",");

    if (initial_pos == -1)
    {
        perror("Failed to get the file position");
        return -1;
    }

    // 开辟一块缓冲区,用于读取文件内容一行
    char buffer[1024];

    // 连续读取count_fly-1行的测距消息
    for (int i = 1; i <= count_fly - 1; i++)
    {
        if (fgets(buffer, sizeof(buffer), fp) == NULL)
        {
            printf("Failed to read line or end of file reached.\n");
            break;
        }

        if (getOneRecvInfo(buffer, ptr_ret, count_fly, src_addr, msg_seq) == true)
        {
            count_recv_flies++;
        }
    }

    fseek(fp, initial_pos, SEEK_SET);
    free(ptr_ret);
    return count_recv_flies;
}
/**
 *功能：获得所有收到这条消息无人机的srcAddr，timestamp
        分析这条TX消息之下的所有RX信息
 *
 * */
char *getRecvFlies(FILE *fp, char line[], int count_fly)
{
    char *ptr_ret = (char *)malloc(1024 * sizeof(char));
    if (ptr_ret != NULL)
    {
        memset(ptr_ret, 0, 1024);
    }
    if (fp == NULL)
    {
        printf("File pointer is null.\n");
        return NULL;
    }

    ptr_ret[0] = '\0'; // 初始化输出字符串

    long initial_pos = ftell(fp); // 获取文件的当前位置
    /* TODO:定位msg_seq的时候 */
    int msg_seq = getIntByIndex(line, 8, ",");
    int src_addr = getIntByIndex(line, 7, ",");

    if (initial_pos == -1)
    {
        perror("Failed to get the file position");
        return NULL;
    }

    // 开辟一块缓冲区,用于读取文件内容一行
    char buffer[1024];

    // 连续读取count_fly-1行的测距消息
    for (int i = 1; i <= count_fly - 1; i++)
    {
        if (fgets(buffer, sizeof(buffer), fp) == NULL)
        {
            printf("Failed to read line or end of file reached.\n");
            break;
        }
        getOneRecvInfo(buffer, ptr_ret, count_fly, src_addr, msg_seq);
    }

    fseek(fp, initial_pos, SEEK_SET);
    return ptr_ret;
}

/**
 *  功能：返回某条测距消息未收到无人机的编号
 *  count_fly:总的无人机测距数量，知道文件读取到哪一行了
 *  line[]表是当前所读取的测距消息
 *  out_str表示返回的未收到此消息无人机的编号,通过逗号分隔
 *
 **/
// 需要修改：因为涉及到pkl文件之中的顺序
void getLossFlies(FILE *fp, char line[], char *out_str, int count_fly)
{
    if (fp == NULL)
    {
        printf("File pointer is null.\n");
        return;
    }

    out_str[0] = '\0'; // 初始化输出字符串,考虑到strcat()函数
    // 获取文件的当前位置
    long initial_pos = ftell(fp);
    /* TODO:定位msg_seq的时候 */
    int msg_seq = getIntByIndex(line, 8, ",");
    int src_addr = getIntByIndex(line, 7, ",");

    if (initial_pos == -1)
    {
        perror("Failed to get the file position");
        return;
    }

    // 开辟一块缓冲区,用于读取文件内容一行
    char buffer[1024];

    // 连续读取count_fly-1行的测距消息
    for (int i = 1; i <= count_fly - 1; i++)
    {
        char temp[50]; // 临时存储每个整数的字符串表示
        if (fgets(buffer, sizeof(buffer), fp) == NULL)
        {
            printf("Failed to read line or end of file reached.\n");
            break;
        }

        if (checkIsReceived(buffer, count_fly, src_addr, msg_seq) == false)
        {
            // 记录下这行buffer无人机的编号
            int loss_scr_addr = getIntByIndex(buffer, 7, ",");
            // 将整数格式化为字符串并添加到 outStr
            sprintf(temp, "%d", loss_scr_addr);
            strcat(out_str, ",");
            strcat(out_str, temp);
        }
    }
    fseek(fp, initial_pos, SEEK_SET);
}

/**
 * 功能：判断当前的测距消息是否被其余无人机收到
 */
bool checkIsReceived(char *buffer, int count_fly, int src_addr, int msg_seq)
{
    int pos_addr = 1;
    for (int i = 0; i < count_fly - 1; i++)
    {
        int address = getIntByIndex(buffer, pos_addr, ",");
        int seq_number = getIntByIndex(buffer, pos_addr + 1, ",");
        // printf("buffer: %s  address: %d seq_number: %d\n", buffer, address, seq_number);

        if (address == src_addr && seq_number == msg_seq)
        {
            return true; // 表示这架无人机收到上一次的群发消息
        }
        pos_addr = pos_addr + 3;
    }

    return false;
}

/**
 * 返回值：一条接收消息无人机的srcAddr,timestamp,
 * 功能：分析一条TX往下一行的测距消息
 */
bool getOneRecvInfo(char *buffer, char *out_str, int count_fly, int src_addr, int msg_seq)
{
    int pos_addr = 1;
    for (int i = 0; i < count_fly - 1; i++)
    {
        int address = getIntByIndex(buffer, pos_addr, ",");
        int seq_number = getIntByIndex(buffer, pos_addr + 1, ",");
        double rx_timestamp = getDoubleByIndex(buffer, pos_addr + 2, ",");

        char temp[50];
        int my_address = getIntByIndex(buffer, 7, ",");
        if (address == src_addr && seq_number == msg_seq)
        {

            sprintf(temp, "%d", my_address);
            strcat(out_str, temp); // strcat必须要以'\0' 空终止字符结尾
            strcat(out_str, ",");

            sprintf(temp, "%f", rx_timestamp); // sprintf() 自动在输出的字符串末尾添加 \0
            strcat(out_str, temp);
            strcat(out_str, ",");

            return true; // 表示这架无人机收到上一次的群发消息
        }

        pos_addr = pos_addr + 3;
    }

    return false;
}

int getCountBodyUnit(char line[], const char *delimiter)
{
    // 1.首先计算出使用逗号分隔的部分数量
    int count_part = getCountPart(line, delimiter);
    // 2.出去body_unit部分还剩8项
    int count_body_units = count_part - 8;
    // 3.每个body_unit由三项组成
    int count_body_unit = (count_part - count_body_units) / 3;
    return count_body_unit;
}

int getCountPart(char line[], const char *delimiter)
{
    int total_count = 0;

    char buffer_copy[1024];
    // 复制字符串到新的内存位置
    strcpy(buffer_copy, line);

    char *token; // 分隔符之间的字符

    // 使用strtok分割字符串
    token = strtok(buffer_copy, delimiter);
    while (token != NULL)
    {
        total_count++;
        token = strtok(NULL, delimiter); // 继续分割剩余的字符串
    }
    return total_count;
}

int getCountFlies(FILE *fp)
{
    int index[100] = {0}; // 用于计算出无人机的数量
    char buffer[520];
    int count_fly = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        index[getIntByIndex(buffer, 7, ",")]++;
    }

    // 计算出无人机的总数
    for (int i = 0; i < 100; i++)
    {
        if (index[i] != 0)
        {
            set[count_fly] = i;
            count_fly++;
        }
    }
    fseek(fp, 0, SEEK_SET);
    return count_fly;
}