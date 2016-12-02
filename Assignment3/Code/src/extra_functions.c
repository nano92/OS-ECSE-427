#include "extra_functions.h"


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
/**
 * Persists the free block list data structure on the disk
 * 
 * Basic algorithm:
 *   Free block list is stored as a (char) array of length = number of blocks
 */
int save_empty_block_list(char **empty_block_list){
    int check;
    char *buff_empty_block = (char *)malloc(SFS_API_BLOCK_SIZE);
    printf("size of buffer empty block: %lu\n", sizeof(buff_empty_block));
    strcpy(buff_empty_block, (*empty_block_list));
    if(buff_empty_block == NULL){
    	fprintf(stderr, "empty block buffer strdup failed\n");
    	return -1;
    }

    check = write_blocks(1 + SFS_INODE_TABLE_SIZE, 1, 
    					buff_empty_block);
    if(check <= 0){
    	perror("write_blocks error");
    	return -1;
    }
    
    free(buff_empty_block);

    return 1;
}

/**
 * Reads the free block list data structure from the disk to main memory
 * 
 * Basic algorithm:
 *   Free block list is stored as a (char) array of length = number of blocks
 *   Read as a whole block (no iteration)
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

int update_empty_block_list(char **empty_block_list, int start_block, 
								int num_blocks, char status){
	//Ensure to get the last version on disk
	if(get_empty_block_list(empty_block_list) < 0){
		perror("get_empty_block_list() error");
		return -1;
	}
	
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
	puts("Succesfull update_empty_block_list()");

    return 1;
}

/*
 * Initializes the inode table data structure (in mem)
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

/**
 * Finds the next available inode index
 * Returns the first free available inode index
 * @param inode_index Ptr to return variable
 * @return -1 if no space found , 1 if found
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

int next_available_dir_entry(DirectoryEntry **root_dir, int *dir_index){
    for(int i = 0; i < MAX_INODES; i++) {
    //Mode = 0 is the default value, none file have mode = 0  
        if((*root_dir)[i].inode_index == -1){
            *dir_index = i;
            return 1; 
        }
    }
    return -1;
}

int init_super_block(Inode **inode_table, SuperBlock **super_block){
	int check, root_inode_index;
	if(next_available_inode(inode_table, &root_inode_index) < 0){
		fprintf(stderr, "Could not find an inode for root dir\n");
		return -1;
	}
    puts("next_available_inode()");
    SuperBlock *buff_superblock = (SuperBlock *)malloc(SFS_API_BLOCK_SIZE);
    if(buff_superblock == NULL){
    	fprintf(stderr, "super block buffer malloc failed\n");
    	return -1;
    }
    printf("root inode indes: %d\n", root_inode_index );
    *super_block = (SuperBlock *)malloc(sizeof(SuperBlock));
    if(*super_block == NULL){
    	fprintf(stderr, "super block malloc failed\n");
    	return -1;
    }

    (*super_block)->magic_number = SFS_MAGIC_NUMBER;
    (*super_block)->block_size = SFS_API_BLOCK_SIZE;
    (*super_block)->file_sys_size = SFS_API_NUM_BLOCKS;
    (*super_block)->inode_table_length = SFS_INODE_TABLE_SIZE;
    (*super_block)->root_dir_inode = root_inode_index;
     
    memcpy(buff_superblock, *super_block, sizeof(SuperBlock));    // magic
    check = write_blocks(0, 1, buff_superblock);
    if (check <= 0){
    	fprintf(stderr, "write_blocks failed\n");
    	return -1;
    }
    free(buff_superblock);

    return 1;
}

int find_contiguous_empty_space(char **empty_block_list, int size_to_allocate, 
						int *start_block, int *num_blocks){
    
    char full, together;

    int blocks_quantity = (int)ceil((float)size_to_allocate / 
    					(float)SFS_API_BLOCK_SIZE);
    
    // Loops through all blocks
    for(int i = 0; i < SFS_API_NUM_BLOCKS; i++){
        full = (*empty_block_list)[i];
        // Checks for available index
        if(full == 0){
            bool_t flag = true;
            int j = i;
            // Looping from first free index to free index + required length
            while(flag && (j < (i + blocks_quantity))){
                together = (*empty_block_list)[j];
                if (together == 0){
                	flag = true;
                }else{
                	flag = false;
                }
                j++;
            }
            // if taken , all needed space was free
            if(flag){
            	// Set values
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

	puts("No more free space available in disk");
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
	/*for(int i=0; i < SFS_API_NUM_BLOCKS; i++){
		if(temp_empty_block_list[i] == 0){
			count++;
		}
	}*/
}

/**
 * Main method to allocate block of data on the disk. This method makes sure to
 * flag block used by the allocation as used in the free block list as well as
 * persisting data from buffer in main memory.
 * @param start_block the disk block to start from
 * @param nblocks the number of block to write 
 * @param buff the actual buffer containing data
 */
int save_block(char **empty_block_list, int *start_block, int *num_blocks,
				 void *buffer){
    int check = write_blocks(*start_block, *num_blocks, buffer);
    if(check <= 0){
    	fprintf(stderr, "write_blocks() failed\n");
    	return -1;
    }
    
    //Update empty block list
    if(update_empty_block_list(empty_block_list, *start_block, *num_blocks, 
    							FULL) < 0){
    	perror("update_empty_block_list() error");
    	return -1;
    }

    /*for(int i = *start_block; i < (*start_block + *num_blocks); i++){
        (*empty_block_list)[i] = 1;
    }
    
    if(save_empty_block_list(empty_block_list) < 0){
    	fprintf(stderr, "save_empty_block_list() failed\n");
    	return -1;
    }*/

    return 1;
}

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

int set_new_Inode(Inode **inode_table, int *index){
	//init new file inode    
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
/**
 * Save the inode at a given index, maintain the free_inodes char array list
 * up to date
 * @param inode the inode to be stored
 * @param index the index
 
int add_Inode(InodeTable **inode_table, Inode inode, int index){
    (*inode_table)->allocated_count++;
    (*inode_table)->free_inodes[index] = (char)1;
    (*inode_table)->inodes[index] = inode;
    
    if(save_Inode_table(inode_table) < 0){
    	perror("save_inode_table() failed");
    	return -1;
    }else{
    	return 1;
    }
}*/

/**
 * Persists the inode table data structure on the disk
 * 
 * Basic algorithm: 
 *   The inode table is stored on the disk with its inodes entries 
 *      Iterate over the free_inode table, for each used inode store it on disk
 *      at its corresponding position
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
    puts("Succesfull save_Inode_table()");
    return 1;
}
int save_root_dir(Inode **inode_table, SuperBlock **super_block, 
					char **empty_block_list, int *start_block, int *num_blocks){
	//Get quantity of contiguous empty blocks necessary to save root Directory
    //and get the index of the first block. 
    if(find_contiguous_empty_space(empty_block_list, (MAX_INODES+1)*sizeof(DirectoryEntry),
    								start_block, num_blocks) < 0){
    	perror("save_empty_block_list() error");
		exit(EXIT_FAILURE);
    }
    puts("find_contiguous_empty_space()");
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
    puts("Go to save_root_Inode()");
    //Save root Inode in disk
    if(save_root_Inode(inode_table, super_block, start_block, num_blocks) < 0){
    	perror("save_root_Inode() error");
    	return -1;
    }

    return 1;
}

int update_root_dir(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block, int *index, int *dir_index, 
                        char *filename){

	
	//Directory entries needs to be increased since there is a new file
	Inode *root = &((*inode_table)[(*super_block)->root_dir_inode]);
	DirectoryEntry *buff_root_dir = (DirectoryEntry *)malloc(MAX_INODES*
                                                        sizeof(DirectoryEntry));
	if(buff_root_dir == NULL){
		perror("buffer root dir entries malloc error");
		return -1;
	}
	
    memcpy(buff_root_dir, (*root_dir), MAX_INODES * sizeof(DirectoryEntry));
	
    if(next_available_dir_entry(root_dir, dir_index) < 0){
        perror("next_available_dir_entry error");
        return -1;
    }
    strcpy((*root_dir)[*dir_index].filename, filename);
    (*root_dir)[*dir_index].inode_index = *index;

    memcpy(buff_root_dir, (*root_dir), MAX_INODES * sizeof(DirectoryEntry));

	printf("root dir filename: %s\n", (*root_dir)[*dir_index].filename);
	
	int check = write_blocks(root->direct_ptr[0], root->link_counter, 
								buff_root_dir);
	if(check <= 0){
		printf("check: %d\n", check);
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
 * Reads the root directory entry from the disk to the main memory
 * - Finds the root inode
 * - Reads the datablock contents from 0 to allocated_ptr (root directory could
 *   span over multiple blocks)
 * - For each root entry, read name & extension
 */
int get_root_dir(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block){
	int check;

    if((*root_dir) != 0){
    	
		//free((*root_dir)->entries); 
		//Free reference of root dir in memory to get one fresh from disk
		puts("free root dir");
		free(*root_dir); 
    }

    if(get_Inode_table(inode_table) < 0){
    	perror("get_Inode_table() error");
		return -1;
    }
    puts("get_Inode_table()");
    
    Inode *root = &((*inode_table)[(*super_block)->root_dir_inode]);
    // read the whole root directory block(s) in the buffer
    DirectoryEntry *buff_root_dir = (DirectoryEntry *)malloc(root->link_counter 
                                                        * SFS_API_BLOCK_SIZE);
    
    if(buff_root_dir == NULL){
    	fprintf(stderr, "buff_root_dir malloc error\n");
		return -1;
    }
    
    check = read_blocks(root->direct_ptr[0], root->link_counter, buff_root_dir);
    if(check <= 0){
    	fprintf(stderr, "read_blocks() failed\n");
    	return -1;
    }

    *root_dir = malloc(root->link_counter * SFS_API_BLOCK_SIZE);
    if(*root_dir == NULL){
    	fprintf(stderr, "root_dir malloc failed\n");
    	return -1;
    }
    memcpy(*root_dir, buff_root_dir, root->link_counter * SFS_API_BLOCK_SIZE);   
    free(buff_root_dir);
    return 1;
}

/**
 * Reads the inode table from the disk to main memory
 */
int get_Inode_table(Inode **inode_table){
    int check;
    
    if((*inode_table) != 0){
		//free((*inode_table)->inodes);
		//free((*inode_table)->free_inodes);
		free(*inode_table);
		puts("free inode table");
	}
    
    Inode *buff_inode_table = (Inode *)malloc(SFS_INODE_TABLE_SIZE * 
    										SFS_API_BLOCK_SIZE);
    if(buff_inode_table == NULL){
    	perror("buff_inode_table malloc error");
    	return -1;
    }

    check = read_blocks(1, SFS_INODE_TABLE_SIZE, buff_inode_table);
    if(check <= 0){
    	fprintf(stderr, "read_blocks() failed\n");
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
    
    printf("root inode size %d\n",buff_inode_table[0].size);	
	free(buff_inode_table);
    return 1;
}

/**
 * Initializes the file descriptor table data structure (in mem)
 */
int init_FD_table(FileDescriptorEntry **fd_table){
    *fd_table = malloc(SFS_MAX_FDENTRIES * sizeof(FileDescriptorEntry));
    
    if(*fd_table == NULL){
    	perror("fd_table malloc error");
    	return -1;
    }

    //Init all fd available to default values
    for(int i = 0; i < SFS_MAX_FDENTRIES; i++) {
        (*fd_table)[i].iNode = -1;
        (*fd_table)[i].read_ptr = -1;
        (*fd_table)[i].write_ptr = -1;
        (*fd_table)[i].busy = 0;
    }
    return 1;
}

/**
 * Finds the next available file descriptor index
 * Returns the first free available file descriptor index
 * @param fd_index Ptr to return variable
 * @return -1 if no fd entry avail, 1 if found
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

int get_file(DirectoryEntry **root_dir, DirectoryEntry **file, char *filename){

    for(int i=0; i < MAX_INODES; i++){
    	if(strcmp((*root_dir)[i].filename, filename) == 0){
    		memcpy(*file, &(*root_dir)[i], sizeof(DirectoryEntry));
    		return 1;
    	}
    }
    printf("File: %s could not be found\n", filename);
    return -1;
}

/*
Set Inode of the new file and updates the Inode Table. 
A new entry is added to the root directory and it is updated to the disk
*/
int create_file(DirectoryEntry **root_dir, Inode **inode_table, 
					SuperBlock **super_block, DirectoryEntry **file,
						char *filename){
	int inode_index, dir_index;

	if(set_new_Inode(inode_table, &inode_index) < 0){
		perror("set_new_Inode() error");
		return  -1;
	}
	puts("set_new_Inode()");
	if(update_root_dir(root_dir, inode_table, super_block,
						 &inode_index, &dir_index, filename) < 0){
		perror("update_root_dir() error");
		return -1;
	}
	puts("update_root_dir()");
	memcpy(*file, &(*root_dir)[dir_index], 
		sizeof(DirectoryEntry));
	
	return 1;
}