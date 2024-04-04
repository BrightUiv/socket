CC=gcc
PROC_NUM=3

# Define compile-time flags
CFLAGS=-Wall -DPROC_NUM=$(PROC_NUM)

# Define the target executable
TARGET_SERVER=swarm_ranging_proc
TARGET_CLIENT=control_center

# The first target is the default, build the executable named 'program'
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Link the object file to create the executable
$(TARGET_SERVER): swarm_ranging_proc.o
	$(CC) $(CFLAGS)  swarm_ranging_proc.o -o swarm_ranging_proc

# Compile the source file into an object file
swarm_ranging_proc.o: swarm_ranging_proc.c
	$(CC) $(CFLAGS) -c swarm_ranging_proc.c

# Link the object file to create the executable
$(TARGET_CLIENT): control_center.o
	$(CC) $(CFLAGS) -o control_center control_center.o

# Compile the source file into an object file
control_center.o: control_center.c
	$(CC) $(CFLAGS) -c control_center.c

swarm_ranging.o: gsrp/swarm_ranging.c
	$(CC) $(CFLAGS) -Igsrp/libdw3000 -o swarm_ranging.o gsrp/swarm_ranging.c

# Clean up the build directory
clean:
	rm -f *.o $(TARGET_SERVER) $(TARGET_CLIENT)

start:
	echo start
	eval './startCFprocs.sh $(PROC_NUM)'

kill: 
	echo kill
	pkill -f ./$(TARGET_SERVER)
