/*
Author: Luis Gallet Zambrano
Id: 260583750
Date: 05/12/2016
*/

#ifndef SFS_API_H
#define SFS_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include "disk_emu.h"

#define SFS_API_FILENAME    	"SFS_API.sfs"
#define SFS_API_BLOCK_SIZE  	1024
#define SFS_API_NUM_BLOCKS  	2048
#define SFS_MAGIC_NUMBER    	0xACBD0005
#define SFS_INODE_TABLE_SIZE    20
#define SFS_MAX_FILENAME    	21
#define SFS_MAX_FDENTRIES   	1024
#define MAX_INODES 		(int)floor((float)(SFS_INODE_TABLE_SIZE*SFS_API_BLOCK_SIZE) / (float)sizeof(Inode))
#define EMPTY_BLOCK_LIST_SPACE 	(int)ceil((float)SFS_API_NUM_BLOCKS / (float)SFS_API_BLOCK_SIZE)
#define FULL 					(char)1
#define EMPTY 					(char)0
#define ADD						(char)1
#define REMOVE 					(char)0 
#define NUM_INDIRECT_PTR (int)floor((float)(SFS_API_BLOCK_SIZE - sizeof(int))/(float)sizeof(int))

typedef struct{
	int magic_number;
	int block_size;
	int file_sys_size;
	int inode_table_length;
	int root_dir_inode;
} SuperBlock;

typedef struct{
	mode_t mode;
	int link_counter;
	int size;
	int direct_ptr[12];
	int indirect_ptr;
} Inode;
	
typedef struct{
	char filename[SFS_MAX_FILENAME];
	int inode_index;
} DirectoryEntry;

typedef struct{
	int iNode;
	int read_ptr;
	int write_ptr;
	int busy;
} FileDescriptorEntry;

typedef enum { false , true } bool_t;

void mksfs(int fresh);
int sfs_get_next_file_name(char *fname);
int sfs_get_file_size(char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_frseek(int fileID, int loc);
int sfs_fwseek(int fileID, int loc);
int sfs_fwrite(int fileID, char *buf, int length);
int sfs_fread(int fileID, char *buf, int length);
int sfs_remove(char *file);

#endif /* SFS_API_H */