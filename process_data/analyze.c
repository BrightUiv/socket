#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
int getIntByIndex(char buffer[], int count_num, const char *delimiter);
int findStartIndex(FILE *fp, int set[], int number);
bool checkIsReceived(char *buffer, int count_fly, int src_addr, int msg_seq);
void getLossFlies(FILE *fp, char line[], char *out_str, int count_fly);
char buffer[1024]; // 用于存储文件中的一行

// typedef enum
// {
//     TYPE_INT,
//     TYPE_FLOAT,
//     TYPE_STRING,
//     TYPE_ERROR
// } DataType;

// typedef struct
// {
//     DataType type;
//     union
//     {
//         int intValue;
//         float floatValue;
//         char *stringValue;
//     } data;
// } DataItem;

int main()
{
    FILE *fp;
    const char *delimiter = ","; // CSV文件的分隔符

    int count;
    int count_fly = 0; // 记录无人机的数量
    int index[100];    // 用于计算出无人机的数量
    int set[100];      // 用于记录无人机的srcAddress

    memset(index, 0, sizeof(index));

    // 打开文件
    fp = fopen("output.csv", "r");
    if (fp == NULL)
    {
        perror("Unable to open file");
        return 1;
    }

    // 读取每一行
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
    printf("Count of fly is %d\n", count_fly);

    // 重置文件指针以重新读取文件
    fseek(fp, 0, SEEK_SET);

    findStartIndex(fp, set, 0);
    findStartIndex(fp, set, 1);
    findStartIndex(fp, set, 2);

    int count_line = 0;
    int count_target_line = 15;
    char out_str[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        count_line++;
        if (count_line >= 12)
        {
            getLossFlies(fp, buffer, out_str, 3);
            printf("count_line : %d  out_str: %s \n", count_line, out_str);
        }
    }

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
    // printf("Location %d  return   %s\n", count_num, token);

    return atoi(token);
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

// 为control_center进程生成配置文件
// 功能:生成对应的配置文件
// 要点：1.找到一条测距消息的未收到的无人机srcAddress,并且使用一个数组记录下编号
// 2.
FILE *generateConfig(FILE *fp)
{
    while (1)
    {
        // fgets()传输读取当前文件的指针位置,
    }
    return NULL;
}
/**
 * 功能：返回某条测距消息未收到无人机的编号
 * 返回方式：一个int fly[]集合，初始值都设置为-1，不为0的时候为无人机的编号
 *count_fly:总的无人机测距数量，知道文件读取到哪一行了
 *line[]表是当前所读取的测距消息
 * out_str表示返回的未收到此消息无人机的编号,通过逗号分隔
 *
 **/
void getLossFlies(FILE *fp, char line[], char *out_str, int count_fly)
{
    if (fp == NULL)
    {
        printf("File pointer is null.\n");
        return;
    }

    out_str[0] = '\0'; // 初始化输出字符串
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
    // printf("++++++++++++++++++++++++++\n");
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
            // printf("loss_scr_addr is: %d\n", loss_scr_addr);
            // 将整数格式化为字符串并添加到 outStr
            sprintf(temp, "%d", loss_scr_addr);
            strcat(out_str, ",");
            strcat(out_str, temp);
        }
    }
    fseek(fp, initial_pos, SEEK_SET);
}

/**
 * 说明：一条buffer就是一条无人机发送的测距消息
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

// 存在问题，如果一个message之中，只有一个body如何解决
// 首先计算出一共有多少个body,总的逗号分隔数，减去header的字段数，除以3，计算出body_unit的总数
// 出现不匹配问题，往下扫描后，没有及时返回