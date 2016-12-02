#include "sfs_api.h"
#include "extra_functions.h"

SuperBlock *super_block;
//InodeTable *inode_table;
Inode *inode_table;
//Directory *root_dir;
DirectoryEntry *root_dir;
FileDescriptorTable *file_descriptor_table;
char *empty_block_list;
static int dir_current_pos = 0;

void mksfs(int fresh){
	if(fresh){
		int start_block, num_blocks;
		int added_space = 0;

		if(init_fresh_disk(SFS_API_FILENAME, 
			SFS_API_BLOCK_SIZE, SFS_API_NUM_BLOCKS) < 0){

			perror("init_fresh_disk() error");
			exit(EXIT_FAILURE);
		}

		puts("init_fresh_disk()");

		if(init_empty_block_list(&empty_block_list) < 0){
			perror("init_empty_block_list() error");
			exit(EXIT_FAILURE);
		}

		//Initialize Inode Table
		if(init_Inode_table(&inode_table) < 0){
			perror("init_Inode_table() error");
			exit(EXIT_FAILURE);
		}
		puts("init_inode_table()");
		//Initialize Super block
		if(init_super_block(&inode_table, &super_block) < 0){
			perror("init_super_block() error");
			exit(EXIT_FAILURE);
		}
		puts("init_super_block()");
		//Update empty block list and set superblock block space to 1 (full)
		if(update_empty_block_list(&empty_block_list, added_space, 1, FULL) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}
		//empty_block_list[added_space] = (char)1;
		//Increase added_space by one since the super block only takes one block
		added_space++;

		//Update empty block list Set Inode table block space to 1 (full)
		if(update_empty_block_list(&empty_block_list, added_space, 
									SFS_INODE_TABLE_SIZE, FULL) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}
		/*for(int i = 0; i < SFS_INODE_TABLE_SIZE; i++) {
            empty_block_list[added_space + i] = (char)1;
        }*/
        added_space = added_space + SFS_INODE_TABLE_SIZE;

        //Update empty blocka and set empty block list space to 1 (full)
        if(update_empty_block_list(&empty_block_list, added_space, 
									EMPTY_BLOCK_LIST_SPACE, FULL) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}

        /*for(int i = 0; i < EMPTY_BLOCK_LIST_SPACE; i++) {
            empty_block_list[added_space + i] = (char)1;
        }*/

        //added_space += EMPTY_BLOCK_LIST_SPACE;

		//save root dir
		if(save_root_dir(&inode_table, &super_block, &empty_block_list, 
							&start_block, &num_blocks) < 0){
			perror("save_root_dir() error");
			exit(EXIT_FAILURE);
		}

		puts("saved root_dir");

        if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
        	perror("get_root_dir() error");
			exit(EXIT_FAILURE);
        }
        puts("get_root_dir()");

	}else{
		
		if(init_disk(SFS_API_FILENAME, 
			SFS_API_BLOCK_SIZE, SFS_API_NUM_BLOCKS) < 0){

			perror("init_disk() error");
			exit(EXIT_FAILURE);
		}
		/*
		Get reference to superblock in memory (locally)
		*/
		int check;
		super_block = malloc(SFS_API_BLOCK_SIZE);
		if(super_block == NULL){
			perror("super_block malloc error");
			exit(EXIT_FAILURE);
		}
		//read superblock from disk	
		check = read_blocks(0, 1, super_block);
		if(check <= 0){
			perror("read_blocks error");
			exit(EXIT_FAILURE);
		}

		//Initialize Inode Table
		if(init_Inode_table(&inode_table) < 0){
			perror("init_inode_table() error");
			exit(EXIT_FAILURE);
		}
		//Reset empty_block_list global variable in memory and get the one from
		//the disk
		memset(empty_block_list, 0, SFS_API_NUM_BLOCKS  * sizeof(char));
		if(get_empty_block_list(&empty_block_list) < 0){
			perror("get_empty_block_list() error");
			exit(EXIT_FAILURE);
		}

		if(get_Inode_table(&inode_table) < 0){
			perror("get_Inode_table() error");
			exit(EXIT_FAILURE);
		}

		if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
        	perror("get_root_dir() error");
			exit(EXIT_FAILURE);
        }
	}
	
	if(init_FD_table(&file_descriptor_table) < 0){
		perror("init_FD_table() error");
		exit(EXIT_FAILURE);
	}
}
/**
 * Gets the name of the next file in the root directory
 * This maintains a global variable that is incremented on each function call
 * @param fname The return buffer variable
 * @return 1 if file found, 0 if no more file in the directory
 */
int sfs_get_next_file_name(char *fname){
    //Get updated reference from root dir and other important structres from 
  	//disk to memory
  	if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
        	perror("get_root_dir() error");
			return -1;
    }

    if(dir_current_pos >= root_dir->count) { 
        return 0;
    }
    strcpy(fname, root_dir->entries[dir_current_pos].filename);
   	dir_current_pos++;
    return 1;
}
int sfs_get_file_size(char* path){
	int size;
  	DirectoryEntry *file = malloc(sizeof(DirectoryEntry));
  	if(file == NULL){
  		perror("file malloc error");
  		return -1;
  	} 

  	//Get updated reference from root dir and other important structres from 
  	//disk to memory
  	if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
        	perror("get_root_dir() error");
			return -1;
    }  	
  	if(get_file(&root_dir, &file, path) < 0){
  		perror("get_file() erorr");
  		return -1;
  	}
  	size = inode_table->inodes[file->inode_index].size;
  	free(file);
  	
  	return size;
}
int sfs_fopen(char *name){
	int fd_index;
	//check the length of the file name
	if(strlen(name) > SFS_MAX_FILENAME){
		return -1;
	}
	 DirectoryEntry *file = malloc(sizeof(DirectoryEntry));
	 //Get updated reference from root dir and other important structres from 
  	//disk to memory
  	// if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
   //      	perror("get_root_dir() error");
			// return -1;
   //  }

  	if(get_file(&root_dir, &file, name) < 0){
  		if(create_file(&root_dir, &inode_table, &super_block, &file, name) < 0){
  			perror("create_file() error");
  			return -1;
  		}
  	}
  	
  	
  	printf("inode index: %d\n", file->inode_index);
  	//DirectoryEntry *file =  &(root_dir->entries[root_dir->count]);
    // check if the file is already opened
    for(int i = 0; i < file_descriptor_table->size; i++) {
        if((file_descriptor_table->entries[i]).busy == 1 && 
        	(file_descriptor_table->entries[i]).iNode==file->inode_index){
            printf("File %s is already open", file->filename);
            return -1;
        }
    }

    if(next_available_fd(&file_descriptor_table, &fd_index) < 0){
    	printf("No more fd availables to open file: %s,\
    		close some files before trying again", name);
    	return -1;
    }else{
    	file_descriptor_table->entries[fd_index].busy = 1;
    	file_descriptor_table->entries[fd_index].iNode = file->inode_index;
    	//File is opened if append mode. Thus, the read pointer is set at the 
    	//beginning of the file and the write pointer at the end
    	file_descriptor_table->entries[fd_index].read_ptr = 0;
    	file_descriptor_table->entries[fd_index].write_ptr = 
    							(inode_table->inodes[file->inode_index]).size;
    	free(file);
    }
    return fd_index;
}
int sfs_fclose(int fileID){
 	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if(fileID >= file_descriptor_table->size){
		printf("sfs_fclose() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table->entries[fileID].busy == 0){
		printf("sfs_fclose() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);
		return -1;
	}else{	
		//Sets the requested file descriptr free to be used by another file.
		//This means that the file cannot longer be accessed for editing, thus 
		//it is closed
		file_descriptor_table->entries[fileID].busy = 0;
		return 0;
	}
}
int sfs_frseek(int fileID, int loc){
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if(fileID >= file_descriptor_table->size){
		printf("sfs_frseek() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table->entries[fileID].busy == 0){
		printf("sfs_frseek() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	}else{
		//Move read pointer to desired location in the file
		file_descriptor_table->entries[fileID].read_ptr = loc;
		return 0;
	}
}
int sfs_fwseek(int fileID, int loc){
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if(fileID >= file_descriptor_table->size){
		printf("sfs_frseek() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table->entries[fileID].busy == 0){
		printf("sfs_frseek() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	}else{
		//Move write pointer to desired location in the file
		file_descriptor_table->entries[fileID].write_ptr = loc;
		return 0;
	}
}
int sfs_fwrite(int fileID, char *buf, int length){
 /*
	check if fid is in fileDescTable
		if not return -1

	block # = floor(wptr/1024) (size of block) //index of block to write to
	block_added_space = wptr % size
 */
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if(fileID >= file_descriptor_table->size){
		printf("sfs_fwrite() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table->entries[fileID].busy == 0){
		printf("sfs_fwrite() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	}
	//check if there is enough space on disk to allocate requiered data
	int free_blocks = get_remaining_empty_space(&empty_block_list);
	int requiered_blocks = (int)floor((float)(length/(float)SFS_API_BLOCK_SIZE));

	if(free_blocks < 0 || (free_blocks < requiered_blocks)){
		puts("sfs_fwrite() error: There is not enough space on disk");
		return -1;
	}

	int written_bytes = 0;
	FileDescriptorEntry *fd = &(file_descriptor_table->entries[fileID]);
	
	//First case:  write to an empty file
	//check if file is empty
	if((inode_table->inodes[fd->iNode]).size == 0){
		//check if there are enough contiguous blocks to write 
		//the requiered data
		int start_block, num_blocks;
		if(find_contiguous_empty_space(&empty_block_list,length, &start_block,
										&num_blocks) < 0){
			//Not enough contiguous blocks were find
	    	//check if the data needs 12 blocks or more to be allocated
	    	if(requiered_blocks <= 12){
	    		int one_block = 1;
	    		//Save the requiered data one block at a time
	    		for(int i=0; i < requiered_blocks; i++){
	    			//The start block will be the first empty block that can be found
			    	start_block = find_first_empty_space(&empty_block_list);
			    	if(start_block < 0){
			    		return -1;	
			    	}
	    			char *temp_buf = malloc(requiered_blocks*SFS_API_BLOCK_SIZE);
	    			if(temp_buf == NULL){
	    				perror("temp_buf malloc error");
	    				return -1;
	    			}
	    			memcpy(temp_buf, buf, length);
	    			if(save_block(&empty_block_list, &start_block, &one_block, 
	    							temp_buf+(i*SFS_API_BLOCK_SIZE)) < 0){
	    				perror("save_block() error");
	    				return -1;
	    			}
	    			inode_table->inodes[fd->iNode].direct_ptr[i] = start_block;
	    			inode_table->inodes[fd->iNode].link_counter++;
			    	free(temp_buf);
	    		}

	    	}else{
	    		//need to use indirect pointer
	    	}

    	}else{
    		//Enough contiguous blocks were find
    		//check if the data needs 12 blocks or more to be allocated
	    	if(requiered_blocks <= 12){
	    		char *temp_buf = malloc(requiered_blocks * SFS_API_BLOCK_SIZE);
    			if(temp_buf == NULL){
    				perror("temp_buf malloc error");
    				return -1;
    			}
    			memcpy(temp_buf, buf, length);
    			if(save_block(&empty_block_list, &start_block, &num_blocks, 
    							temp_buf) < 0){
    				perror("save_block() error");
    				return -1;
    			}
    			for(int i=0; i < requiered_blocks; i++){
    				inode_table->inodes[fd->iNode].
    											direct_ptr[i] = start_block + i;
    				inode_table->inodes[fd->iNode].link_counter++;
    			}
		    	free(temp_buf);
	    	}else{
	    		//need to use indirect pointer
	    	}
    	}
    	//Check if last used block was filled completely. If it was, then 
		//the written bytes is the full size of the requiered blocks. If it
		//was not then then we only take into consideration the bytes 
		//written to the last block plus the full size of the rest of them
		int block_added_space = length % SFS_API_BLOCK_SIZE;
		if(block_added_space == 0){
			inode_table->inodes[fd->iNode].size = 
				written_bytes = 
					requiered_blocks * SFS_API_BLOCK_SIZE;
		}else{
			inode_table->inodes[fd->iNode].size = 
				written_bytes = 
					(requiered_blocks-1)*SFS_API_BLOCK_SIZE+block_added_space;
		}
		
		return written_bytes;

	}else{
		//File is not empty
	}

	return 0;
}
int sfs_fread(int fileID, char *buf, int length){
  return 0;
}
int sfs_remove(char *file){
  return 0;
}

/*
getFirstBlock(block #, block_offeset)
 buffer_read_block(index)
 	memcpy(buffer + offeset, data, 1024 * added_space)
	writeBlock(index, buffer )

currentblock = first_block + 1
currentptr = data + first_length
for(int i=0; i< #blocks; i++)
	currentblock_index = getBlockIndex(currentblock)
	writeblock(currentblock_index, currentptr)
	currentblock ++
	currentptr += 1024

fullBLockstoWriteTo = floor((toalLength - #ofbiytesThatSavedInFirstblock)/1024)
*/

int main(){
	mksfs(1);
	int fds[20];
    for(int i = 0; i < 20; i++) {
        char name[1024];
        sprintf(name, "file_%d", i);
        fds[i] = sfs_fopen(name);
        
        if(fds[i] >= 0) {
            printf("Opened %s\n", name);
        }
        
    }
	return 1;

}