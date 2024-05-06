CC=gcc
CFLAGS=-Wall -g

all: task_queue_system.o swarm_ranging.o

task_queue_system.o:  task_queue_system.h  task_queue_system.c  adhocdeck.h 
	$(CC) $(CFLAGS) -o ../build/task_queue_system.o -c task_queue_system.c -Ilibdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt
	
swarm_ranging.o:  swarm_ranging.h swarm_ranging.c task_queue_system.h  adhocdeck.h 
	$(CC) $(CFLAGS) -o ../build/swarm_ranging.o -c swarm_ranging.c -Ilibdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt

