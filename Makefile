CC=gcc
PROC_NUM=3

# Define compile-time flags
CFLAGS=-Wall -DPROC_NUM=$(PROC_NUM) -Igsrp -Igsrp/libdw3000 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread

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
$(TARGET_SERVER): $(OBJDIR)/swarm_ranging_proc.o $(OBJDIR)/socket_util.o $(OBJDIR)/swarm_ranging.o  $(OBJDIR)/task_queue_system.o  
	$(CC) $(CFLAGS) $^ -o $@

# Compile the source file into an object file
$(OBJDIR)/swarm_ranging_proc.o: swarm_ranging_proc.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link the object file to create the executable
$(TARGET_CLIENT): $(OBJDIR)/control_center.o $(OBJDIR)/socket_util.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile the socket_util
$(OBJDIR)/socket_util.o: socketUtil/SocketUtil.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/swarm_ranging.o: subdir

# Compile the source file into an object file
$(OBJDIR)/control_center.o: control_center.c
	$(CC) $(CFLAGS) -c $< -o $@

subdir:
	$(MAKE) -C gsrp

# Clean up the build directory
clean:
	rm -f $(OBJDIR)/*.o $(TARGET_SERVER) $(TARGET_CLIENT)

start:
	echo start
	eval './startCFprocs.sh $(PROC_NUM)'

kill: 
	echo kill
	pkill -f ./$(TARGET_SERVER)

# apr_test:
# 	$(CC) $(CFLAGS)  -o apr_test apr_queue/apr_test.c -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread