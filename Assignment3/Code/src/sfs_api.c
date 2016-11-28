#include "sfs_api.h"
#include "extra_functions.h"

SuperBlock *super_block;
InodeTable *inode_table;
Directory *root_dir;
FileDescriptorTable *file_descriptor_table;
char *empty_block_list;
static int dir_current_pos = 0;

void mksfs(int fresh){
	if(fresh){
		int start_block, num_blocks;
		int offset = 0;

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
		if(update_empty_block_list(&empty_block_list, offset, 1) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}
		//empty_block_list[offset] = (char)1;
		//Increase offset by one since the super block only takes one block
		offset++;

		//Update empty block list Set Inode table block space to 1 (full)
		if(update_empty_block_list(&empty_block_list, offset, 
									SFS_INODE_TABLE_SIZE) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}
		/*for(int i = 0; i < SFS_INODE_TABLE_SIZE; i++) {
            empty_block_list[offset + i] = (char)1;
        }*/
        offset += SFS_INODE_TABLE_SIZE;

        //Update empty blocka and set empty block list space to 1 (full)
        if(update_empty_block_list(&empty_block_list, offset, 
									EMPTY_BLOCK_LIST_SPACE) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}

        /*for(int i = 0; i < EMPTY_BLOCK_LIST_SPACE; i++) {
            empty_block_list[offset + i] = (char)1;
        }*/

        //offset += EMPTY_BLOCK_LIST_SPACE;

		//save root dir
		if(save_root_dir(&inode_table, &super_block, &empty_block_list, 
							&start_block, &num_blocks) < 0){
			perror("save_root_dir() error");
			exit(EXIT_FAILURE);
		}

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
  	
  	if(get_file(&root_dir, &inode_table, &super_block, &file, path) < 0){
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
	//check if file exists
	DirectoryEntry *file = malloc(sizeof(DirectoryEntry));
  	if(file == NULL){
  		perror("file malloc error");
  		return -1;
  	} 
  	if(get_file(&root_dir, &inode_table, &super_block, &file, name) < 0){
  		if(create_file(&root_dir, &inode_table, &super_block, &file, name) < 0){
  			perror("create_file() error");
  			return -1;
  		}
  	}
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

    	return fd_index;
    }
}
int sfs_fclose(int fileID){
  return 0;
}
int sfs_frseek(int fileID, int loc){
  return 0;
}
int sfs_fwseek(int fileID, int loc){
  return 0;
}
int sfs_fwrite(int fileID, char *buf, int length){
 /*
	check if fid is in fileDescTable
		if not return -1

	block # = floor(wptr/1024) (size of block) //index of block to write to
	block_offset = wptr % size
 */ 
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
 	memcpy(buffer + offeset, data, 1024 * offset)
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
	return 1;

}