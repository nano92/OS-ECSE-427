/*
Author: Luis Gallet Zambrano
Id: 260583750
Date: 05/12/2016
*/

#include "extra_functions.h"

/*
Initialize empty block list and set al elements to free (0)
*/
int init_empty_block_list(char **empty_block_list){
	//initialize empty block list and set values to 0 (empty)
	*empty_block_list = (char*)malloc(SFS_API_NUM_BLOCKS * sizeof(char));
	memset(*empty_block_list, 0, SFS_API_NUM_BLOCKS * sizeof(char));
	if(*empty_block_list == NULL){
		perror("empty_block_list malloc error");
		return -1;
	}
	//save to memory
	if(save_empty_block_list(empty_block_list) < 0){
		perror("save_empty_block_list() error");
		return -1;
	}
	return 1;
}
/*
Save empty block list to the disk
 */
int save_empty_block_list(char **empty_block_list){
    int check;
    char *buff_empty_block = (char *)malloc(SFS_API_BLOCK_SIZE);
    strcpy(buff_empty_block, (*empty_block_list));
    if(buff_empty_block == NULL){
    	perror("empty block buffer strdup failed");
    	return -1;
    }

    check = write_blocks(1 + SFS_INODE_TABLE_SIZE, 1, buff_empty_block);
    if(check <= 0){
    	perror("write_blocks error");
    	return -1;
    }
    
    free(buff_empty_block);

    return 1;
}

/*
Get empty block list from disk
 */
int get_empty_block_list(char **empty_block_list){
    int check;
    char *buff_list = (char *)malloc(SFS_API_BLOCK_SIZE);
    check = read_blocks(1 + SFS_INODE_TABLE_SIZE, 1, 
    						buff_list);
    if(check <= 0){
    	perror("write_blocks failed");
    	return -1;
    }
    strcpy(*empty_block_list, buff_list);
    
    free(buff_list);
    return 1;
}

/*
Update the empty block list by setting the required block to empty(0) or full(1)
Save it back to the disk, gets a fresh reference to memory(local)
*/
int update_empty_block_list(char **empty_block_list, int start_block, 
								int num_blocks, char status){	
	//Update empty block list values
	for(int i = start_block; i < (start_block + num_blocks); i++){
		(*empty_block_list)[i] = status;
	}

	if(save_empty_block_list(empty_block_list) < 0){
    	perror("save_empty_block_list() error\n");
    	return -1;
    }

    //Gets the up to date reference from disk to memory
    if(get_empty_block_list(empty_block_list) < 0){
		perror("get_empty_block_list() error");
		return -1;
	}

    return 1;
}

/*
Initializes the inode table data structure to default values
 */
int init_Inode_table(Inode **inode_table){
	*inode_table = malloc((MAX_INODES+1) * sizeof(Inode));
	if(*inode_table == NULL){
		perror("inode_table malloc error");
		return -1;
	}

	//Init maximum number of inodes to default values; set one extra inode for
    //the root inode
	for(int i=0; i < MAX_INODES + 1; i++){
		(*inode_table)[i].mode = 0;
		(*inode_table)[i].link_counter = 0;
		(*inode_table)[i].size = 0;
		(*inode_table)[i].indirect_ptr = -1;
		for(int j=0; j < 12; j++){
			(*inode_table)[i].direct_ptr[j] = -1;
		}	
	}
    return 1;
}

/*
Find the next available inode by checking the entire table
 */
int next_available_inode(Inode **inode_table, int *inode_index){
    for(int i = 0; i < MAX_INODES; i++) {
 	//Mode = 0 is the default value, none file have mode = 0  
        if((*inode_table)[i].mode == 0){
        	*inode_index = i;
        	return 1; 
        }
    }
    return -1;
}

/*
Find the next available directory entry in the entire directory
*/
int next_available_dir_entry(DirectoryEntry **root_dir, int *dir_index){
    for(int i = 1; i < MAX_INODES + 1; i++) {
    //inode index -1 is the default value  
        if((*root_dir)[i].inode_index == -1){
            *dir_index = i;
            return 1; 
        }
    }
    return -1;
}

/*
Init super block data structure. It finds and sets the root inode as well
*/
int init_super_block(Inode **inode_table, SuperBlock **super_block){
	int check, root_inode_index;
	if(next_available_inode(inode_table, &root_inode_index) < 0){
		perror("Could not find an inode for root dir");
		return -1;
	}
    SuperBlock *buff_superblock = (SuperBlock *)malloc(SFS_API_BLOCK_SIZE);
    if(buff_superblock == NULL){
    	perror("super block buffer malloc failed");
    	return -1;
    }
    *super_block = (SuperBlock *)malloc(sizeof(SuperBlock));
    if(*super_block == NULL){
    	perror("super block malloc failed");
    	return -1;
    }

    (*super_block)->magic_number = SFS_MAGIC_NUMBER;
    (*super_block)->block_size = SFS_API_BLOCK_SIZE;
    (*super_block)->file_sys_size = SFS_API_NUM_BLOCKS;
    (*super_block)->inode_table_length = SFS_INODE_TABLE_SIZE;
    (*super_block)->root_dir_inode = root_inode_index;
     
    memcpy(buff_superblock, *super_block, sizeof(SuperBlock));    
    check = write_blocks(0, 1, buff_superblock);
    if (check <= 0){
    	perror("write_blocks failed");
    	return -1;
    }
    free(buff_superblock);

    return 1;
}
/*
Gets total amount of contiguous blocks found that will suffice for the amount
of byte requested. It sets the start block and the number of contiguous blocks
*/
int find_contiguous_empty_space(char **empty_block_list, int size_to_allocate, 
						int *start_block, int *num_blocks){
    
    char full, together;

    int blocks_quantity = (int)ceil((float)size_to_allocate / 
    					(float)SFS_API_BLOCK_SIZE);
    
    for(int i = 0; i < SFS_API_NUM_BLOCKS; i++){
        full = (*empty_block_list)[i];
        //Get block and checks if its empty
        if(full == 0){
            bool_t flag = true;
            int j = i;
            //Check if there are enough empty blocks from the start point
            while(flag && (j < (i + blocks_quantity))){
                together = (*empty_block_list)[j];
                if (together == 0){
                	flag = true;
                }else{
                	flag = false;
                }
                j++;
            }
            //If flag is true, there is enough contiguous blocks to allocate 
            //data
            if(flag){
                *start_block = i;
                *num_blocks = blocks_quantity;
                return 1;
            }
        }
    }
    *start_block = *num_blocks = -1;
    return -1;
}
/*
Check every element in the empty block list and returns the index of the
first empty block
*/
int find_first_empty_space(char **empty_block_list){
	
	for(int i=0; i < SFS_API_NUM_BLOCKS; i++){
		if((*empty_block_list)[i] == 0){
			return i;
		}
	}
	return -1;
}
/*
Comparator function to be used by quick sort method
*/
static int comparator(const void *p1, const void *p2)
{
	return(*(int*)p1 - *(int *)p2);
}
/*
Returns the quantity of empty blocks from the empty block list
*/
int get_remaining_empty_space(char **empty_block_list){
	int count = 0;
	//In order to speed up the search, the empty block list is copied into a 
	//a temporary list, then this list is sorted in increasing order and all 
	//zeros occurrences are counted

	char *temp_empty_block_list = calloc(SFS_API_NUM_BLOCKS, sizeof(char));
	if(temp_empty_block_list == NULL){
		perror("temp_empty_block_list malloc error");
		return -1;
	}
	strcpy(temp_empty_block_list,*empty_block_list); 
	if(temp_empty_block_list == NULL){
		perror("temp_empty_block_list malloc error");
		return -1;
	}
	qsort((void *)(temp_empty_block_list), SFS_API_NUM_BLOCKS,
					sizeof(char), comparator);
    if(temp_empty_block_list[0] == 1){
		//All elements in list are 1, thus the disk is full
		return -1;
	}else{
		while(temp_empty_block_list[count] == 0){
            count++;
		}
	}
	free(temp_empty_block_list);

	return count;
}

/*
Method to save a block into the disk, it updates the empty block list as well
 */
int save_block(char **empty_block_list, int *start_block, int *num_blocks,
				 void *buffer){
    int check = write_blocks(*start_block, *num_blocks, buffer);
    if(check <= 0){
    	perror("write_blocks() error");
    	return -1;
    }
    
    //Update empty block list
    if(update_empty_block_list(empty_block_list, *start_block, *num_blocks, 
    							FULL) < 0){
    	perror("update_empty_block_list() error");
    	return -1;
    }
    return 1;
}

/*
Initialize the root inode and save the inode table to the disks
*/
int save_root_Inode(Inode **inode_table, SuperBlock **super_block,
					int *start_block, int *num_blocks){
    //Initialize root directory Inode
    (*inode_table)[(*super_block)->root_dir_inode].mode = 
                                        S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
    (*inode_table)[(*super_block)->root_dir_inode].link_counter = *num_blocks;
    (*inode_table)[(*super_block)->root_dir_inode].size = 
                                            *num_blocks * SFS_API_BLOCK_SIZE;
    //The file system is designed to ensure that the blocks holding the root 
    //directory are contiguous and are less than 12
    for(int i=0; i < *num_blocks; i++){
    	(*inode_table)[(*super_block)->root_dir_inode].direct_ptr[i] = 
                                                            *start_block + i;
	}
	if(save_Inode_table(inode_table) < 0){
    	perror("save_Inode_table() error");
    	return -1;
    }
    return 1;
}

/*
Initialize a new inode by finding an empty one and setting up its mode
*/
int set_new_Inode(Inode **inode_table, int *index){  
    if(next_available_inode(inode_table, index) < 0){
    	perror("next_available_inode() error");
    	return -1;
    }
    //Set mode of new node, since we do not have any other information
    (*inode_table)[*index].mode = S_IRWXU | S_IRWXG | S_IRWXO;
    if(save_Inode_table(inode_table) < 0){
    	perror("save_Inode_table() error");
    	return -1;
    }
    return 1;
}

/*
Save inode table to the disk
*/
int save_Inode_table(Inode **inode_table){
	int check;
   	Inode *buffer_iTable = malloc(SFS_INODE_TABLE_SIZE * 
                                            SFS_API_BLOCK_SIZE);
   
   	
    memcpy(buffer_iTable, (*inode_table),SFS_INODE_TABLE_SIZE * 
                                            SFS_API_BLOCK_SIZE);

   
    check = write_blocks(1, SFS_INODE_TABLE_SIZE, buffer_iTable);
    if(check <= 0){
    	perror("write_blocks() failed");
    	return -1;
    }
    free(buffer_iTable);
    return 1;
}
/*
Function to save the root directory. It find sufficient blocks to save it and
init to default values all of its possible entries
*/
int save_root_dir(Inode **inode_table, SuperBlock **super_block, 
					char **empty_block_list, int *start_block, int *num_blocks){
	//Get quantity of contiguous empty blocks necessary to save root Directory
    //and get the index of the first block. 
    if(find_contiguous_empty_space(empty_block_list, (MAX_INODES+1)*sizeof(DirectoryEntry),
    								start_block, num_blocks) < 0){
    	perror("save_empty_block_list() error");
		exit(EXIT_FAILURE);
    }
	DirectoryEntry *buff_root_dir = (DirectoryEntry *)malloc(*num_blocks 
                                                        * SFS_API_BLOCK_SIZE);
    
    if(buff_root_dir == NULL){
    	perror("buff_root_dir malloc error\n");
    	return -1;
    }
    
    //Init all possible entries to defualt values
    for(int i=1; i < MAX_INODES + 1; i++){
    	memset(buff_root_dir[i].filename, '\0', SFS_MAX_FILENAME);
    	buff_root_dir[i].inode_index = -1;
    }
    
    //Save blocks of root directory in disk
    if(save_block(empty_block_list, start_block, num_blocks, 
    	buff_root_dir) < 0){
    	perror("save_block() error");
    	return -1;
    }
    //Save root Inode in disk
    if(save_root_Inode(inode_table, super_block, start_block, num_blocks) < 0){
    	perror("save_root_Inode() error");
    	return -1;
    }

    return 1;
}

/*
Add a new entry to the root directory, or remove on (set entry to default 
values)and save it to the disk
*/
int update_root_dir(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block, int *index, int *dir_index, 
                        char *filename, char update){

	Inode *root = &((*inode_table)[(*super_block)->root_dir_inode]);
	DirectoryEntry *buff_root_dir = (DirectoryEntry *)malloc(MAX_INODES*
                                                        sizeof(DirectoryEntry));
	if(buff_root_dir == NULL){
		perror("buffer root dir entries malloc error");
		return -1;
	}
	
	if(update == ADD){
		 if(next_available_dir_entry(root_dir, dir_index) < 0){
	        perror("next_available_dir_entry error");
	        return -1;
	    }
	    strcpy((*root_dir)[*dir_index].filename, filename);
	    (*root_dir)[*dir_index].inode_index = *index;
	}else if(update == REMOVE){
		memset((*root_dir)[*dir_index].filename, '\0', SFS_MAX_FILENAME);
	    (*root_dir)[*dir_index].inode_index = *index;
	}

    memcpy(buff_root_dir, (*root_dir), MAX_INODES * sizeof(DirectoryEntry));

	int check = write_blocks(root->direct_ptr[0], root->link_counter, 
								buff_root_dir);
	if(check <= 0){
		perror("write_blocks() error");
		return -1;
	}
	free(buff_root_dir);
	//Get updated reference from root dir and other important structres from 
  	//disk to memory
  	if(get_root_dir(root_dir, inode_table, super_block) < 0){
        	perror("get_root_dir() error");
			return -1;
    }

	return 1;
}

/**
Get root directory reference from disk to memory(local). It get the inode table
as well
 */
int get_root_dir(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block){
	int check;

    if((*root_dir) != 0){
		//Free reference of root dir in memory to get one fresh from disk
		free(*root_dir); 
    }

    if(get_Inode_table(inode_table) < 0){
    	perror("get_Inode_table() error");
		return -1;
    }
    
    Inode *root = &((*inode_table)[(*super_block)->root_dir_inode]);
    //read the whole root directory block(s) in the buffer
    DirectoryEntry *buff_root_dir = (DirectoryEntry *)malloc(root->link_counter 
                                                        * SFS_API_BLOCK_SIZE);
    
    if(buff_root_dir == NULL){
    	perror("buff_root_dir malloc error");
		return -1;
    }
    
    check = read_blocks(root->direct_ptr[0], root->link_counter, buff_root_dir);
    if(check <= 0){
    	perror("read_blocks() failed");
    	return -1;
    }

    *root_dir = malloc(root->link_counter * SFS_API_BLOCK_SIZE);
    if(*root_dir == NULL){
    	perror("root_dir malloc failed");
    	return -1;
    }
    memcpy(*root_dir, buff_root_dir, root->link_counter * SFS_API_BLOCK_SIZE);   
    free(buff_root_dir);
    return 1;
}

/*
Gets the inode table from disk to memory(local)
 */
int get_Inode_table(Inode **inode_table){
    int check;
    
    if((*inode_table) != 0){
    	//Free reference of inode table in memory to get one fresh from disk
		free(*inode_table);
	}
    
    Inode *buff_inode_table = (Inode *)malloc(SFS_INODE_TABLE_SIZE * 
    										SFS_API_BLOCK_SIZE);
    if(buff_inode_table == NULL){
    	perror("buff_inode_table malloc error");
    	return -1;
    }

    check = read_blocks(1, SFS_INODE_TABLE_SIZE, buff_inode_table);
    if(check <= 0){
    	perror("read_blocks() failed");
    	return -1;
    }

    *inode_table = (Inode *)malloc(SFS_INODE_TABLE_SIZE * 
                                            SFS_API_BLOCK_SIZE);
    if(*inode_table == NULL){
    	fprintf(stderr, "free inodes malloc failed\n");
    	return -1;
    }

    memcpy(*inode_table, buff_inode_table, 
                                    SFS_INODE_TABLE_SIZE * SFS_API_BLOCK_SIZE);
    
	free(buff_inode_table);
    return 1;
}

/*
Initializes the file descriptor table data structure in memory to its default 
values
 */
int init_FD_table(FileDescriptorEntry **fd_table){
    *fd_table = malloc(SFS_MAX_FDENTRIES * sizeof(FileDescriptorEntry));
    
    if(*fd_table == NULL){
    	perror("fd_table malloc error");
    	return -1;
    }

    //Init all fd available to default values
    for(int i = 0; i < SFS_MAX_FDENTRIES; i++) {
        (*fd_table)[i].iNode = -69;
        (*fd_table)[i].read_ptr = -1;
        (*fd_table)[i].write_ptr = -1;
        (*fd_table)[i].busy = 0;
    }
    return 1;
}

/*
Finds the next available fd in the table
 */
int next_available_fd(FileDescriptorEntry **file_descriptor_table, 
						int *fd_index){
    for(int i = 0; i < SFS_MAX_FDENTRIES; i++) {
        if((*file_descriptor_table)[i].busy == 0){
        	*fd_index = i; 
        	return 1;
        }
    }
    
    return -1;
}

/*
Get file from root directory if its found
*/
int get_file(DirectoryEntry **root_dir, DirectoryEntry **file, char *filename){

    for(int i=0; i < MAX_INODES; i++){
    	if(strncmp((*root_dir)[i].filename, filename, SFS_MAX_FILENAME + 1) == 0){
    		memcpy(*file, &(*root_dir)[i], sizeof(DirectoryEntry));
    		return 1;
    	}
    }
    return -1;
}

/*
Set Inode of the new file and updates the Inode Table. 
A new entry is added to the root directory and it is updated in the disk
*/
int create_file(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block, DirectoryEntry **file,
						char *filename){
	int inode_index, dir_index;

	if(set_new_Inode(inode_table, &inode_index) < 0){
		perror("set_new_Inode() error");
		return  -1;
	}

	if(update_root_dir(root_dir, inode_table, super_block,
						 &inode_index, &dir_index, filename, ADD) < 0){
		perror("update_root_dir() error");
		return -1;
	}

	memcpy(*file, &(*root_dir)[dir_index], 
		sizeof(DirectoryEntry));
	
	return 1;
}

/*
Get indirection block list from the disk
*/
int get_indirection_block(int **indirection_ptr, Inode **inode_table, 
							int inode){
	int check;
	//get indirection block from disk
	*indirection_ptr = malloc(SFS_API_BLOCK_SIZE);
	int *buff_ind_ptr = malloc(SFS_API_BLOCK_SIZE);
	check = read_blocks((*inode_table)[inode].indirect_ptr, 1, buff_ind_ptr);
	if(check <= 0){
		perror("read_blocks() error");
		return -1;
	}
	memcpy(*indirection_ptr, buff_ind_ptr, 
										NUM_INDIRECT_PTR * sizeof(int));
	free(buff_ind_ptr);

	return 1;
	
}

/*
This function gets the requiered block from the indirect pointer list or from 
the direct pointer list. If the block pointer points to a position that does 
not have a block, the one empty block is found and the lists are updated
*/
int get_last_block(Inode **inode_table, int iNode, int block_ptr, int *last_block, char **empty_block_list){
	int check;
	if(block_ptr >= 12){
		int *buffer_ind_ptr = malloc(SFS_API_BLOCK_SIZE);
		check = read_blocks((*inode_table)[iNode].indirect_ptr, 1, 
								buffer_ind_ptr);
		if(check <= 0){
			perror("read_blocks() error");
			return -1;
		}
		*last_block = buffer_ind_ptr[block_ptr];
		
		if(*last_block == -1){
			*last_block = find_first_empty_space(empty_block_list);
			if(*last_block < 0){
				perror("find_first_empty_space() error");
				return -1;
			}
		}
		buffer_ind_ptr[block_ptr] = *last_block;
		check = write_blocks((*inode_table)[iNode].indirect_ptr, 1, 
								buffer_ind_ptr);
		if(check <= 0){
			perror("write_blocks() error");
			return -1;
		}

		free(buffer_ind_ptr);
	}else{
		*last_block = (*inode_table)[iNode].direct_ptr[block_ptr];
		if(*last_block == -1){
			*last_block = find_first_empty_space(empty_block_list);
			if(*last_block < 0){
				perror("find_first_empty_space() error");
				return -1;
			}
		}
		(*inode_table)[iNode].direct_ptr[block_ptr] = *last_block;
	}
	

	return 1;
}