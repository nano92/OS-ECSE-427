# Makefile
# Modified by: Luis Gallet Zambrano
# Id: 260583750
# Date: 05/12/2016

#The makefile was slightly modified in order to test the sfs implementation
#without using the test files

# To compile with fuse, make or make fuse both works
# To compile with test1, make test1
# To compile with test2, make test2

HEADERS = -I headers/
SOURCES= disk_emu.c sfs_api.c extra_functions.c fuse_wrappers.c
SOURCES_TEST1= disk_emu.c extra_functions.c sfs_api.c sfs_test1.c tests.c
SOURCES_TEST2= disk_emu.c extra_functions.c sfs_api.c sfs_test2.c tests.c

SOURCES_PREFIX = $(addprefix src/, $(SOURCES))
OBJECTS_SOURCES= $(addprefix objects/, $(SOURCES:.c=.o))
SOURCES_TEST1_PREFIX = $(addprefix src/, $(SOURCES_TEST1))
SOURCES_TEST2_PREFIX = $(addprefix src/, $(SOURCES_TEST2))

CC = clang -g -Wall  -D_FILE_OFFSET_BITS=64 $(HEADERS)
LDFLAGS = -lm `pkg-config fuse --cflags --libs` 
EXECUTABLE=Luis_Gallet_sfs

all: $(SOURCES_PREFIX)
	$(CC) $(LDFLAGS) -o $(EXECUTABLE) $(SOURCES_PREFIX) 

test1: $(SOURCES_TEST1_PREFIX) 
	$(CC) -o $(EXECUTABLE) $(SOURCES_TEST1_PREFIX) -lm

test2: $(SOURCES_TEST2_PREFIX)
	$(CC) -o $(EXECUTABLE) $(SOURCES_TEST2_PREFIX) -lm

fuse:  $(OBJECTS_SOURCES) 
	$(CC) -o $(EXECUTABLE) $^ $(LDFLAGS)

objects/disk_emu.o: src/disk_emu.c headers/disk_emu.h
	$(CC) -c $< -o $@

objects/extra_functions.o: src/extra_functions.c headers/extra_functions.h
	$(CC) -c $< -o $@

objects/sfs_api.o: src/sfs_api.c headers/sfs_api.h
	$(CC) -c $< -o $@

objects/fuse_wrappers.o: src/fuse_wrappers.c
	$(CC) -c $< -o $@

clean:
	rm objects/*.o
	rm $(EXECUTABLE)
	rm SFS_API.sfs
