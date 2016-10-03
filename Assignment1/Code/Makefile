HEADERS = -I headers/
CFLAGS = -g -Wall $(HEADERS)

all: Shell2

Shell2: objects/Shell2.o objects/Queue_linkedList.o
	gcc $(CFLAGS) -o $@ $^ 

objects/Shell2.o: src/Shell2.c 
	gcc $(CFLAGS) -c $< -o $@

objects/Queue_linkedList.o: src/Queue_linkedList.c headers/Queue_linkedList.h
	gcc $(CFLAGS) -c $< -o $@
	
clean:
	rm objects/*.o
