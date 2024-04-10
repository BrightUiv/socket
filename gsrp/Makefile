CC=gcc
CFLAGS=-Wall

swarm: swarm_ranging.c
	$(CC) $(CFLAGS) -Ilibdw3000 -o swarm_ranging.o swarm_ranging.c

apr_test:apr_test.c
	$(CC) $(CFLAGS)  -o apr_test.o apr_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread
