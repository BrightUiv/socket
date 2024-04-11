CC=gcc
CFLAGS=-Wall

# swarm: swarm_ranging.c
# 	$(CC) $(CFLAGS) -Ilibdw3000 -o swarm_ranging.o swarm_ranging.c

<<<<<<< HEAD
# apr_test:apr_test.c
# 	$(CC) $(CFLAGS)  -o apr_test apr_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread

task_queue_system:task_queue_system.o swarm_ranging.o 
	$(CC) $(CFLAGS) -o task_queue_system task_queue_system.o swarm_ranging.o
swarm_ranging.o: swarm_ranging.c swarm_ranging.h adhocdeck.h
	$(CC) $(CFLAGS) -c swarm_ranging.c
task_queue_system.o: task_queue_system.c task_queue_system.h swarm_ranging.h adhocdeck.h
	$(CC) $(CFLAGS) -c task_queue_system.c -Ilibdw3000 -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread
=======
apr_test:
	$(CC) $(CFLAGS)  -o apr_test apr_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread
>>>>>>> 69f0368021e49e46a788292c12b2ecf47fd1b6a6



