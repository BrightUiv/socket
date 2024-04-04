#!/bin/bash

# 检查是否提供了参数
if [ $# -eq 0 ]; then
    echo "Usage: $0 <number_of_times>"
    exit 1
fi

# 从第一个参数获取循环次数
loop_count=$1

# 检查参数是否为数字
if ! [[ $loop_count =~ ^[0-9]+$ ]]; then
   echo "Error: '$loop_count' is not a valid number."
   exit 1
fi

# 使用for循环执行指定次数的命令
for (( i=0; i<loop_count; i++ )); do
    echo "Running command for iteration $i"    
    ./swarm_ranging_proc server$i $i &
    echo "Finished running the command ${loop_count} times."
done
