#ifndef EXTRA_FUNCTIONS_H

#include "sfs_api.h"

#define EXTRA_FUNCTIONS_H

int init_empty_block_list(char **empty_block_list);
int save_empty_block_list(char **empty_block_list);
int get_empty_block_list(char **empty_block_list);
int update_empty_block_list(char **empty_block_list, int start_block, 
								int num_blocks);
int init_Inode_table(InodeTable **inode_table);
int next_available_inode(InodeTable **inode_table, int *inode_index);
int init_super_block(InodeTable **inode_table, SuperBlock **super_block);
int find_empty_space(char **empty_block_list, int size_to_allocate, 
						int* start_block, int* num_blocks);
int save_block(char **empty_block_list, int *start_block, int *num_blocks,
				 void *buffer);
int save_root_dir(InodeTable **inode_table, SuperBlock **super_block, 
					char **empty_block_list, int *start_block, int *num_blocks);
int save_root_Inode(InodeTable **inode_table, SuperBlock **super_block,
					int *start_block);
int add_Inode(InodeTable **inode_table, Inode inode, int index);
int save_Inode_table(InodeTable **inode_table);
int update_root_dir(Directory **root_dir, InodeTable **inode_table, 
					SuperBlock **super_block, int *index, char *filename);
int get_root_dir(Directory **root_dir, InodeTable **inode_table, 
					SuperBlock **super_block);
int get_Inode_table(InodeTable **inode_table);
int init_FD_table(FileDescriptorTable **fd_table);
int next_available_fd(FileDescriptorTable **file_descriptor_table, 
						int *fd_index);
int get_file(Directory **root_dir, InodeTable **inode_table, 
				SuperBlock **super_block, DirectoryEntry **file,
					char *filename);
int create_file(Directory **root_dir, InodeTable **inode_table, 
					SuperBlock **super_block, DirectoryEntry **file, 
						char *filename);

#endif /* EXTRA_FUNCTIONS_H */