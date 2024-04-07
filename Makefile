CC=gcc
PROC_NUM=3

# Define compile-time flags
CFLAGS=-Wall -DPROC_NUM=$(PROC_NUM)

# Define the target executable
TARGET_SERVER=swarm_ranging_proc
TARGET_CLIENT=control_center

# Output directory for object files
OBJDIR = build

# Ensure the OBJDIR exists
$(shell mkdir -p $(OBJDIR))

# Define the target, build the executable named 'program'
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Link the object file to create the executable
$(TARGET_SERVER): $(OBJDIR)/swarm_ranging_proc.o $(OBJDIR)/socket_util.o $(OBJDIR)/message_struct.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile the source file into an object file
$(OBJDIR)/swarm_ranging_proc.o: swarm_ranging_proc.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link the object file to create the executable
$(TARGET_CLIENT): $(OBJDIR)/control_center.o $(OBJDIR)/socket_util.o $(OBJDIR)/message_struct.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile the socket_util
$(OBJDIR)/socket_util.o: socketUtil/SocketUtil.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile the message_struct
$(OBJDIR)/message_struct.o: message_struct.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile the source file into an object file
$(OBJDIR)/control_center.o: control_center.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build directory
clean:
	rm -f $(OBJDIR)/*.o $(TARGET_SERVER) $(TARGET_CLIENT)

start:
	echo start
	./startCFprocs.sh $(PROC_NUM)

kill: 
	echo kill
	pkill -f ./$(TARGET_SERVER)
