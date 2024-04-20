#ifndef __HEADER_H__
#define __HEADER_H__
/**
    header.h的功能为，替换swarm_ranging.c和swarm_ranging.h中涉及到操作系统的部分；
    task_queue_system.h文件的功能：修改swarm_ranging中的函数
    编译顺序：header.o---->swarm_ranging.o-->task_queue_system.o
*/
#include<stdlib.h>
#include<stdio.h>
#include "adhocdeck.h"

int uwbSendPacketBlock(UWB_Packet_t *packet);


#endif
