CC=gcc
CFLAGS=-Wall

# swarm: swarm_ranging.c 
# 	$(CC) $(CFLAGS) -Ilibdw3000 -o swarm_ranging.o swarm_ranging.c


# apr_test:apr_test.c
# 	$(CC) $(CFLAGS)  -o apr_test apr_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread

task_queue_system:task_queue_system.o swarm_ranging.o 
	$(CC) $(CFLAGS) -o task_queue_system task_queue_system.o swarm_ranging.o -I/home/mao/srtp/gsrp/libdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt

task_queue_system.o:  task_queue_system.h swarm_ranging.h task_queue_system.c  adhocdeck.h 
	# $(CC) $(CFLAGS) -c task_queue_system.c -I/home/mao/srtp/gsrp/libdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt
	
swarm_ranging.o:  swarm_ranging.h swarm_ranging.c task_queue_system.h  adhocdeck.h 
	$(CC) $(CFLAGS) -c swarm_ranging.c -I/home/mao/srtp/gsrp/libdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread -lm -lrt





# apr_test:
# 	$(CC) $(CFLAGS)  -o apr_test apr_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread





