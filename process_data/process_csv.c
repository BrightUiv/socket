#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main()
{
    // 文件指针
    FILE *fp, *output;

    // 打开文件，读取模式
    fp = fopen("test1.csv", "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    // 打开输出文件，写入模式
    output = fopen("output.csv", "w");
    if (output == NULL)
    {
        perror("Error opening output file");
        fclose(fp);
        return -1;
    }

    int ch;

    // 读取每一个字符
    while ((ch = fgetc(fp)) != EOF)
    {
        // 转换字符为小写以统一处理
        char lower_ch = tolower(ch);

        // 检查并过滤不需要的字符
        if (lower_ch != '{' && lower_ch != '}' && lower_ch != '"' && lower_ch != ' ' && lower_ch != '\'' && lower_ch != ':' && lower_ch != 'a' && lower_ch != 'b' && lower_ch != 'c' && lower_ch != 'd' && lower_ch != 'e' && lower_ch != 'f' && lower_ch != 'g')
        {
            // 如果不是上述字符，写入到输出文件
            fputc(ch, output);
        }
    }

    // 关闭文件
    fclose(fp);
    fclose(output);

    printf("File processed successfully\n");
    return 0;
}
