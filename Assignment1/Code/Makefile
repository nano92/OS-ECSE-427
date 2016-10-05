HEADERS = -I headers/
CFLAGS = -g -Wall $(HEADERS)

all: Shell

Shell: objects/Shell.o objects/Queue_linkedList.o
	gcc $(CFLAGS) -o $@ $^ 

objects/Shell.o: src/Shell.c headers/Shell.h 
	gcc $(CFLAGS) -c $< -o $@

objects/Queue_linkedList.o: src/Queue_linkedList.c headers/Queue_linkedList.h
	gcc $(CFLAGS) -c $< -o $@
	
clean:
	rm objects/*.o
