

PROC_NUM	= 3


proc: swarm_ranging_proc.o
	gcc -o  swarm_ranging_proc swarm_ranging_proc.o

swarm_ranging_proc.o:
	gcc -c swarm_ranging_proc.c

control:
	gcc -DPROC_NUM=$(PROC_NUM) -o control_center control_center.c

control_center.o:
	gcc -c control_center.c

start:
	echo start

kill: 
	echo kill

clean:
	rm *.o swarm_ranging_proc control_center
