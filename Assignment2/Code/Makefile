
#Simple shell implementation Makefile
#Author: Luis Gallet Zambrano 260583750
#Last date modified: 04/10/2016

HEADERS = -I headers/
OBJECTS_SERVER = objects/jobs_queue.o objects/print_server.o
OBJECTS_CLIENT = objects/jobs_queue.o objects/print_client.o
CFLAGS = -g -Wall $(HEADERS) 
LDFLAGS = -pthread -lrt 

all: server client

server: $(OBJECTS_SERVER)
	gcc -o $@ $^ $(LDFLAGS)

client: $(OBJECTS_CLIENT)
	gcc -o $@ $^ $(LDFLAGS)

objects/print_client.o: src/print_client.c headers/jobs_queue.h
	gcc $(CFLAGS) -c $< -o $@

objects/print_server.o: src/print_server.c headers/jobs_queue.h
	gcc $(CFLAGS) -c $< -o $@

#objects/sema.o: src/sema.c headers/sema.h 
	#gcc $(CFLAGS) -c $< -o $@ 

objects/jobs_queue.o: src/jobs_queue.c headers/jobs_queue.h
	gcc $(CFLAGS) -c $< -o $@

#objects/Queue_linkedList.o: src/Queue_linkedList.c headers/Queue_linkedList.h
	#gcc $(CFLAGS) -c $< -o $@
	
clean:
	rm objects/*.o