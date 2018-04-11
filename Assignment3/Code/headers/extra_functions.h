/*
Author: Luis Gallet Zambrano
Id: 260583750
Date: 05/12/2016
*/

#ifndef EXTRA_FUNCTIONS_H

#include "sfs_api.h"

#define EXTRA_FUNCTIONS_H

int init_empty_block_list(char **empty_block_list);
int save_empty_block_list(char **empty_block_list);
int get_empty_block_list(char **empty_block_list);
int update_empty_block_list(char **empty_block_list, int start_block, 
								int num_blocks, char status);
int init_Inode_table(Inode **inode_table);
int next_available_inode(Inode **inode_table, int *inode_index);
int next_available_dir_entry(DirectoryEntry **root_dir, int *dir_index);
int init_super_block(Inode **inode_table, SuperBlock **super_block);
int find_contiguous_empty_space(char **empty_block_list, int size_to_allocate, 
						int* start_block, int* num_blocks);
int find_first_empty_space(char **empty_block_list);
int get_remaining_empty_space(char **empty_block_list);
int save_block(char **empty_block_list, int *start_block, int *num_blocks,
				 void *buffer);
int save_root_dir(Inode **inode_table, SuperBlock **super_block, 
					char **empty_block_list, int *start_block, int *num_blocks);
int save_root_Inode(Inode **inode_table, SuperBlock **super_block,
					int *start_block, int *num_blocks);
//int add_Inode(Inode **inode_table, Inode inode, int index);
int save_Inode_table(Inode **inode_table);
int update_root_dir(DirectoryEntry **root_dir, Inode **inode_table, 
						SuperBlock **super_block, int *index, int *dir_index, 
                        	char *filename, char update);
int get_root_dir(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block);
int get_Inode_table(Inode **inode_table);
int init_FD_table(FileDescriptorEntry **fd_table);
int next_available_fd(FileDescriptorEntry **file_descriptor_table, 
						int *fd_index);
int get_file(DirectoryEntry **root_dir, DirectoryEntry **file, char *filename);
int create_file(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block, DirectoryEntry **file, 
						char *filename);
int get_indirection_block(int **indirection_ptr, Inode **inode_table, 
							int inode);
int get_last_block(Inode **inode_table, int iNode, int block_ptr, 
					int *last_block, char **empty_block_list);
#endif /* EXTRA_FUNCTIONS_H */