
proc: swarm_ranging_proc.o
	gcc -o swarm_ranging_proc swarm_ranging_proc.o

swarm_ranging_proc.o:
	gcc -c swarm_ranging_proc.c

control: control_center.o
	gcc -o control_center control_center.o

control_center.o:
	gcc -c control_center.c

start:
	echo start

kill: 
	echo kill

clean:
	rm *.o swarm_ranging_proc control_center
