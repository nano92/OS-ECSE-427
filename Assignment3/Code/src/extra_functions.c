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
    char *buff_empty_block = (char *)malloc(SFS_API_NUM_BLOCKS);
    strcpy(buff_empty_block, (*empty_block_list));
    if(buff_empty_block == NULL){
    	fprintf(stderr, "empty block buffer strdup failed\n");
    	return -1;
    }

    check = write_blocks(1 + SFS_INODE_TABLE_SIZE, EMPTY_BLOCK_LIST_SPACE, 
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
    char *buff_list = (char *)malloc(EMPTY_BLOCK_LIST_SPACE * 
    								SFS_API_BLOCK_SIZE);
    check = read_blocks(1 + SFS_INODE_TABLE_SIZE, EMPTY_BLOCK_LIST_SPACE, 
    						buff_list);
    if(check <= 0){
    	perror("write_blocks failed");
    	return -1;
    }
    memcpy(*empty_block_list, buff_list, SFS_API_BLOCK_SIZE);
    
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
int init_Inode_table(InodeTable **inode_table){
    *inode_table = malloc(sizeof(InodeTable));
    if(inode_table == NULL){
    	fprintf(stderr, "inode table malloc failed\n");
    	return -1;
    }
    
    (*inode_table)->free_inodes = (char *)malloc(MAX_INODES * sizeof(char));
    if((*inode_table)->free_inodes == NULL){
    	fprintf(stderr, "free node malloc failed\n");
    	return -1;
    }
    (*inode_table)->inodes = (Inode *)malloc(MAX_INODES * sizeof(Inode));
    if((*inode_table)->inodes == NULL){
    	fprintf(stderr, "inode malloc failed\n");
    	return -1;
    }
    
    (*inode_table)->allocated_count = 0;
    (*inode_table)->size = MAX_INODES;
    
    /*
    Set all elements from free inode list to 0
	0 = free
	1 = occupied	
	*/
    for(int i = 0; i < MAX_INODES; i++) {
        (*inode_table)->free_inodes[i] = (char)0;
    }
    

    return 1;
}

/**
 * Finds the next available inode index
 * Returns the first free available inode index
 * @param inode_index Ptr to return variable
 * @return -1 if no space found , 1 if found
 */
int next_available_inode(InodeTable **inode_table, int *inode_index){
    for(int i = 0; i < MAX_INODES; i++) {
        if((*inode_table)->free_inodes[i] == (char)0){
        	*inode_index = i;
        	return 1; 
        }
    }
    return -1;
}

int init_super_block(InodeTable **inode_table, SuperBlock **super_block){
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

	char *temp_empty_block_list = (char *)malloc(SFS_API_NUM_BLOCKS);
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

int save_root_Inode(InodeTable **inode_table, SuperBlock **super_block,
					int *start_block, int *num_blocks){
    //Initialize root directory Inode
    Inode root;
    root.mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
    root.link_counter = *num_blocks;
    root.size = *num_blocks * SFS_API_BLOCK_SIZE;
    //The file system is designed to ensure that the blocks holding the root 
    //directory are contiguous and are less than 12
    for(int i=0; i < *num_blocks; i++){
    	root.direct_ptr[i] = *start_block + i;
	}
	printf("num_blocks: %d, start_block: %d\n",*num_blocks, *start_block);
    if(add_Inode(inode_table, root, (*super_block)->root_dir_inode) < 0){
    	perror("add_Inode() error");
		return -1;
    }else{
    	return 1;
    }
}

int set_new_Inode(InodeTable **inode_table, int *index){
	//init new file inode
	Inode inode;
    inode.mode = S_IRWXU | S_IRWXG | S_IRWXO;
    inode.link_counter = 0;
    inode.size = 0;
    memset(inode.direct_ptr, -1, 12);
    inode.indirect_ptr = -1;


    if(next_available_inode(inode_table, index) < 0){
    	perror("next_available_inode() error");
    	return -1;
    }
    if(add_Inode(inode_table, inode, *index) < 0){
    	perror("add_Inode()");
    	return -1;
    }

    return 1;
}
/**
 * Save the inode at a given index, maintain the free_inodes char array list
 * up to date
 * @param inode the inode to be stored
 * @param index the index
 */
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
}

/**
 * Persists the inode table data structure on the disk
 * 
 * Basic algorithm: 
 *   The inode table is stored on the disk with its inodes entries 
 *      Iterate over the free_inode table, for each used inode store it on disk
 *      at its corresponding position
 */
int save_Inode_table(InodeTable **inode_table){
	int check;
   	InodeTable *buff_itable = malloc(SFS_INODE_TABLE_SIZE * SFS_API_BLOCK_SIZE);
   	buff_itable->size = (*inode_table)->size;
   	buff_itable->allocated_count = (*inode_table)->allocated_count;
   	buff_itable->free_inodes = strdup((*inode_table)->free_inodes);
   	buff_itable->inodes = malloc((*inode_table)->size * sizeof(Inode));
   	int count = 0;
   	for(int i=0; i < (*inode_table)->size; i++){
   		if((*inode_table)->free_inodes[i] == 1){
   			buff_itable->inodes[count] = (*inode_table)->inodes[i];
   			count++;
   		}
   	}
   /*	char *serialized_inode_table = malloc(SFS_INODE_TABLE_SIZE * SFS_API_BLOCK_SIZE);
   	
   	memcpy(serialized_inode_table, &(*inode_table)->size, sizeof(int));
   	memcpy(serialized_inode_table + sizeof(int), 
   								&(*inode_table)->allocated_count, sizeof(int));
   	memcpy(serialized_inode_table + sizeof(int) + sizeof(int), 
   						(*inode_table)->free_inodes, MAX_INODES * sizeof(char));
   	for(int i=0; i < (*inode_table)->size; i++){
   		if((*inode_table)->free_inodes[i] == 1){
   			memcpy(serialized_inode_table + 
   						sizeof(int) + sizeof(int) + 
   							(MAX_INODES * sizeof(char)) + 
   								(i * sizeof(Inode)), 
   									&(*inode_table)->inodes[i], 
   										sizeof(Inode));
   		}
   	}*/
    int new_blocks = (int)ceil((float)(MAX_INODES*sizeof(Inode)) / 
    					(float)SFS_API_BLOCK_SIZE);
    printf("size of buffer inode table: %d\n", new_blocks);
    check = write_blocks(1, SFS_INODE_TABLE_SIZE, buff_itable);
    if(check <= 0){
    	perror("write_blocks() failed");
    	return -1;
    }
    free(buff_itable->free_inodes);
    free(buff_itable->inodes);
    free(buff_itable);
    printf("size of root inode; %d\n", (*inode_table)->inodes[0].size );
    puts("Succesfull save_Inode_table()");
    return 1;
}
int save_root_dir(InodeTable **inode_table, SuperBlock **super_block, 
					char **empty_block_list, int *start_block, int *num_blocks){
	//Get quantity of contiguous empty blocks necessary to save root Directory
    //and get the index of the first block. 
    if(find_contiguous_empty_space(empty_block_list, sizeof(Directory),
    								start_block, num_blocks) < 0){
    	perror("save_empty_block_list() error");
		exit(EXIT_FAILURE);
    }
    puts("find_contiguous_empty_space()");

	Directory *buff_root_dir = (Directory *)malloc(sizeof(*num_blocks * SFS_API_BLOCK_SIZE));
    
    if(buff_root_dir == NULL){
    	perror("buff_root_dir malloc error\n");
    	return -1;
    }
    
    buff_root_dir->count = 0;
    
    //Save blocks of root directory in disk
    if(save_block(empty_block_list, start_block, num_blocks, 
    	buff_root_dir) < 0){
    	perror("save_block() error");
    	return -1;
    }
    free(buff_root_dir);

    //Save root Inode in disk
    if(save_root_Inode(inode_table, super_block, start_block, num_blocks) < 0){
    	perror("save_root_Inode() error");
    	return -1;
    }

    return 1;
}

int update_root_dir(Directory **root_dir, InodeTable **inode_table, 
					SuperBlock **super_block, int *index, char *filename){

	
	//Directory entries needs to be increased since there is a new file
	Inode *root = &((*inode_table)->inodes[(*super_block)->root_dir_inode]);
	Directory *buff_root_dir = (Directory *)malloc(sizeof((*root_dir)));
	buff_root_dir->entries = (DirectoryEntry *)malloc(((*root_dir)->count + 1) * 
											sizeof(DirectoryEntry));
	if(buff_root_dir->entries == NULL){
		perror("buffer root dir entries malloc error");
		return -1;
	}
	//Get old entries into new entries buffer
	for(int i=0; i < (*root_dir)->count; i++){
		strcpy(buff_root_dir->entries[i].filename, (*root_dir)->entries[i].filename);
		buff_root_dir->entries[i].inode_index = (*root_dir)->entries[i].inode_index;
	}
	//Add new file information to the entries buffer
	strcpy(buff_root_dir->entries[(*root_dir)->count].filename, filename);
	buff_root_dir->entries[(*root_dir)->count].inode_index = *index;
	printf("buffer filename: %s\n", buff_root_dir->entries[(*root_dir)->count].filename);

	//Clean old entries data structure from root dir and replace it with the new
	//updated one
	//memset((*root_dir)->entries, 0, 
		//((*root_dir)->count *sizeof(DirectoryEntry)));
	free((*root_dir)->entries);
	(*root_dir)->entries = (DirectoryEntry *)malloc(sizeof((*root_dir)->count * 
											sizeof(DirectoryEntry)));
	if((*root_dir)->entries == NULL){
		perror("(*root_dir)->entries malloc error");
		return -1;
	}
	
	for(int i=0; i < (*root_dir)->count; i++){
		strcpy((*root_dir)->entries[i].filename,buff_root_dir->entries[i].filename);
		(*root_dir)->entries[i].inode_index = buff_root_dir->entries[i].inode_index;
	}
	
	strcpy((*root_dir)->entries[(*root_dir)->count].filename,buff_root_dir->entries[(*root_dir)->count].filename);
	(*root_dir)->entries[(*root_dir)->count].inode_index = buff_root_dir->entries[(*root_dir)->count].inode_index;
	
	printf("root dir filename: %s\n", (*root_dir)->entries[(*root_dir)->count].filename);
	//Save root dir in disk; create a buffer in case there is an error while 
	//saving the data into the disk
	//Update count
	puts("error 1");
	(*root_dir)->count++;
	puts("error 2");
	buff_root_dir->count = (*root_dir)->count;
	puts("error 3");
	//Since root directory blocks are all contiggious, it is not necessary to 
	//check all direct pointers; just the first direct pointer is needed
	
	int new_blocks = (int)ceil((float)sizeof(buff_root_dir) / 
    					(float)SFS_API_BLOCK_SIZE);
	printf("size of buff root dir: %d\n", new_blocks);
	int check = write_blocks(root->direct_ptr[0], root->link_counter, 
								(*root_dir));
	puts("error 4");
	if(check <= 0){
		printf("check: %d\n", check);
		perror("write_blocks() error");
		return -1;
	}
	puts("error 5");
	free(buff_root_dir->entries);
	free(buff_root_dir);
	puts("error 6");
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
int get_root_dir(Directory **root_dir, InodeTable **inode_table, 
					SuperBlock **super_block){
	int check;

    if((*root_dir) != 0){
    	
		free((*root_dir)->entries); 
		//Free reference of root dir in memory to get one fresh from disk
		puts("free root dir");
		free(*root_dir); 
    }

    if(get_Inode_table(inode_table) < 0){
    	perror("get_Inode_table() error");
		return -1;
    }
    puts("get_Inode_table()");
    
    Inode *root = &((*inode_table)->inodes[(*super_block)->root_dir_inode]);
    printf("root inode size: from get root dir %d\n", root->size);
    // read the whole root directory block(s) in the buffer
    Directory *buff_root_dir = (Directory *)malloc(root->link_counter * SFS_API_BLOCK_SIZE);
    
    if(buff_root_dir == NULL){
    	fprintf(stderr, "buff_root_dir malloc error\n");
		return -1;
    }
    
    check = read_blocks(root->direct_ptr[0], root->link_counter, buff_root_dir);
    if(check <= 0){
    	fprintf(stderr, "read_blocks() failed\n");
    	return -1;
    }

    *root_dir = malloc(sizeof(Directory));
    if(*root_dir == NULL){
    	fprintf(stderr, "root_dir malloc failed\n");
    	return -1;
    }
    (*root_dir)->count = buff_root_dir->count;
    (*root_dir)->entries = (DirectoryEntry *)malloc((*root_dir)->count * sizeof(DirectoryEntry));
    
    if((*root_dir)->entries == NULL) { 
        return -1;
    }
    
  	for(int i=0; i < (*root_dir)->count; i++){
		strcpy((*root_dir)->entries[i].filename,buff_root_dir->entries[i].filename);
		(*root_dir)->entries[i].inode_index = buff_root_dir->entries[i].inode_index;
	}
    // free buffer
    free(buff_root_dir);

    return 1;
}

/**
 * Reads the inode table from the disk to main memory
 */
int get_Inode_table(InodeTable **inode_table){
    int check;
    
    if((*inode_table) != 0){
		free((*inode_table)->inodes);
		free((*inode_table)->free_inodes);
		free(*inode_table);
		puts("free inode table");
	}
    
    InodeTable *buff_inode_table = (InodeTable *)malloc(SFS_INODE_TABLE_SIZE * 
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

    *inode_table = (InodeTable *)malloc(sizeof(InodeTable));
    if(*inode_table == NULL){
    	fprintf(stderr, "free inodes malloc failed\n");
    	return -1;
    }

    (*inode_table)->size = buff_inode_table->size;
    (*inode_table)->allocated_count = buff_inode_table->allocated_count;
    (*inode_table)->free_inodes = (char *)calloc(MAX_INODES, sizeof(char));
    
    if((*inode_table)->free_inodes == NULL){
    	fprintf(stderr, "free inodes malloc failed\n");
    	return -1;
    }
    //Need to deseralized data
    memcpy((*inode_table)->free_inodes, buff_inode_table + 2*sizeof(int), MAX_INODES * sizeof(char));
    //strcpy((*inode_table)->free_inodes, buff_inode_table->free_inodes);
    if((*inode_table)->free_inodes == NULL){
   		perror("Inode table free_inodes strdup error");
   		return -1;
   	}
   	

    (*inode_table)->inodes = (Inode *)malloc((*inode_table)->size *
    											sizeof(Inode));
    if((*inode_table)->inodes == NULL){
    	fprintf(stderr, "inodes malloc failed\n");
    	return -1;
    }
    printf("root inode size %d\n",(*inode_table)->inodes[0].size);
    printf("size of root inode from get inode table: %d\n", buff_inode_table->inodes[0].size);
    for(int i=0; i < (*inode_table)->size; i++){	
    	printf("free inodes %d, index %d\n",buff_inode_table->free_inodes[i], i);	
		if((*inode_table)->free_inodes[i] == (char)1){
			(*inode_table)->inodes[i] = buff_inode_table->inodes[i];
			/*(*inode_table)->inodes[i].link_counter = 
										buff_inode_table->inodes[i].link_counter;
			(*inode_table)->inodes[i].size = buff_inode_table->inodes[i].size;
			(*inode_table)->inodes[i].indirect_ptr = 
										buff_inode_table->inodes[i].indirect_ptr;
			for(int j=0; j < (*inode_table)->inodes[i].link_counter; j++){
				(*inode_table)->inodes[i].direct_ptr[j] = 
										buff_inode_table->inodes[i].direct_ptr[j];
			}*/
		}
  			//count++;
    }
    printf("root inode size %d\n",(*inode_table)->inodes[0].size);	
	free(buff_inode_table);
    return 1;
}

/**
 * Initializes the file descriptor table data structure (in mem)
 */
int init_FD_table(FileDescriptorTable **fd_table){
    *fd_table = malloc(sizeof(FileDescriptorTable));
    
    if(*fd_table == NULL){
    	perror("fd_table malloc error");
    	return -1;
    }

    (*fd_table)->entries = malloc(SFS_MAX_FDENTRIES * 
    								sizeof(FileDescriptorEntry));
    if((*fd_table)->entries == NULL){
    	perror("fd table entries malloc error");
    	return -1;
    }
    
    (*fd_table)->size = SFS_MAX_FDENTRIES;
    for(int i = 0; i < (*fd_table)->size; i++) {
        ((*fd_table)->entries[i]).busy = 0;
    }

    return 1;
}

/**
 * Finds the next available file descriptor index
 * Returns the first free available file descriptor index
 * @param fd_index Ptr to return variable
 * @return -1 if no fd entry avail, 1 if found
 */
int next_available_fd(FileDescriptorTable **file_descriptor_table, 
						int *fd_index){
    for(int i = 0; i < (*file_descriptor_table)->size; i++) {
        if((*file_descriptor_table)->entries[i].busy == 0){
        	*fd_index = i; 
        	return 1;
        }
    }
    
    return -1;
}

int get_file(Directory **root_dir, DirectoryEntry **file, char *filename){
    int dir_index = 0;

    while(dir_index < (*root_dir)->count){
    	if(strcmp((*root_dir)->entries[dir_index].filename, filename) == 0){
    		memcpy(*file, &(*root_dir)->entries[dir_index], 
    			sizeof(DirectoryEntry));
    		return 1;
    	}
    	dir_index++;
    }
    printf("File: %s could not be found\n", filename);
    return -1;
}

/*
Set Inode of the new file and updates the Inode Table. 
A new entry is added to the root directory and it is updated to the disk
*/
int create_file(Directory **root_dir, InodeTable **inode_table, 
					SuperBlock **super_block, DirectoryEntry **file,
						char *filename){
	int index;

	if(set_new_Inode(inode_table, &index) < 0){
		perror("set_new_Inode() error");
		return  -1;
	}
	puts("set_new_Inode()");
	if(update_root_dir(root_dir, inode_table, super_block,
						 &index, filename) < 0){
		perror("update_root_dir() error");
		return -1;
	}
	puts("update_root_dir()");
	memcpy(*file, &(*root_dir)->entries[(*root_dir)->count - 1], 
		sizeof(DirectoryEntry));
	
	return 1;
}