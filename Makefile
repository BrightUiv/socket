proc:
	gcc -o swarm_ranging_proc swarm_ranging_proc.c

control:
	gcc -o control_center control_center.c

start:
	echo start

kill: 
	echo kill
