CC=gcc
CFLAGS=-Wall -g

task_queue_system:task_queue_system.o swarm_ranging.o 
	$(CC) $(CFLAGS) -o task_queue_system task_queue_system.o swarm_ranging.o -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt

task_queue_system.o:  task_queue_system.h  task_queue_system.c  adhocdeck.h 
	# $(CC) $(CFLAGS) -c task_queue_system.c -Ilibdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt
	
swarm_ranging.o:  swarm_ranging.h swarm_ranging.c task_queue_system.h  adhocdeck.h 
	$(CC) $(CFLAGS) -c swarm_ranging.c -Ilibdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt

adhocdeck.o:  adhocdeck.c  adhocdeck.h 
	$(CC) $(CFLAGS) -c adhocdeck.c -Ilibdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib 


# apr_test:
# 	$(CC) $(CFLAGS)  -o apr_test apr_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread





