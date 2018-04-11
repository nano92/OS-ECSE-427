/*
Author: Luis Gallet Zambrano
Id: 260583750
Date: 05/12/2016
*/

#include "sfs_api.h"
#include "extra_functions.h"

SuperBlock *super_block;
Inode *inode_table;
DirectoryEntry *root_dir;
FileDescriptorEntry *file_descriptor_table;
char *empty_block_list;
static int dir_current_pos = 1;
static int total_files = 0;

void mksfs(int fresh){
	if(fresh){
		int start_block, num_blocks;
		int added_space = 0;

		if(init_fresh_disk(SFS_API_FILENAME, 
			SFS_API_BLOCK_SIZE, SFS_API_NUM_BLOCKS) < 0){

			perror("init_fresh_disk() error");
			exit(EXIT_FAILURE);
		}

		if(init_empty_block_list(&empty_block_list) < 0){
			perror("init_empty_block_list() error");
			exit(EXIT_FAILURE);
		}

		//Initialize Inode Table
		if(init_Inode_table(&inode_table) < 0){
			perror("init_Inode_table() error");
			exit(EXIT_FAILURE);
		}
	
		//Initialize Super block
		if(init_super_block(&inode_table, &super_block) < 0){
			perror("init_super_block() error");
			exit(EXIT_FAILURE);
		}

		//Update empty block list and set superblock block space to 1 (full)
		if(update_empty_block_list(&empty_block_list, added_space, 1, (char)FULL) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}
		//Increase added_space by one since the super block only takes one block
		added_space++;

		//Update empty block list Set Inode table block space to 1 (full)
		if(update_empty_block_list(&empty_block_list, added_space, 
									SFS_INODE_TABLE_SIZE, (char)FULL) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}
		
        added_space += SFS_INODE_TABLE_SIZE;

        //Update empty blocka and set empty block list space to 1 (full)
        if(update_empty_block_list(&empty_block_list, added_space, 
									EMPTY_BLOCK_LIST_SPACE, (char)FULL) < 0){
			perror("update_empty_block_list() error");
			exit(EXIT_FAILURE);
		}

		//save root dir
		if(save_root_dir(&inode_table, &super_block, &empty_block_list, 
							&start_block, &num_blocks) < 0){
			perror("save_root_dir() error");
			exit(EXIT_FAILURE);
		}

		//Update root dir and inode table data structures in memory (local)
        if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
        	perror("get_root_dir() error");
			exit(EXIT_FAILURE);
        }

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

int sfs_get_next_file_name(char *fname){
    if(dir_current_pos == total_files + 1){
    	dir_current_pos = 1;
    	return 0;
    }
   
    strcpy(fname, root_dir[dir_current_pos].filename);
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
	
  	if(get_file(&root_dir, &file, path) < 0){
  		perror("get_file() erorr");
  		return -1;
  	}
  	size = inode_table[file->inode_index].size;
  	free(file);
  	
  	return size;
}

int sfs_fopen(char *name){
	int fd_index = -1;
	//check the length of the file name
	if(strlen(name) > SFS_MAX_FILENAME){
		return -1;
	}
	 DirectoryEntry *file = malloc(sizeof(DirectoryEntry));
	
  	if(get_file(&root_dir, &file, name) < 0){
  		if(create_file(&root_dir, &inode_table, &super_block, &file, name) < 0){
  			perror("create_file() error");
  			return -1;
  		}
  		//If a file is created, total files global counter is incremented
  		total_files++;
  	}

    //check if the file is already opened
    for(int i = 0; i < SFS_MAX_FDENTRIES; i++) {
        if((file_descriptor_table[i].busy == 1) && 
        		(file_descriptor_table[i].iNode == file->inode_index)){
            
            printf("File %s is already open\n", file->filename);
            return -1;
        }
    }

    if(next_available_fd(&file_descriptor_table, &fd_index) < 0){
    	printf("No more fd availables to open file: %s,\
    		close some files before trying again", name);
    	return -1;
    }else{
    	file_descriptor_table[fd_index].busy = 1;
    	file_descriptor_table[fd_index].iNode = file->inode_index;
    	//File is opened in append mode. Thus, the read pointer is set at the 
    	//beginning of the file and the write pointer at the end
    	file_descriptor_table[fd_index].read_ptr = 0;
    	file_descriptor_table[fd_index].write_ptr = 
    							(inode_table[file->inode_index]).size;
    	free(file);
    }
    return fd_index;
}
int sfs_fclose(int fileID){
 	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if((fileID >= SFS_MAX_FDENTRIES) || (fileID < 0)){
		printf("sfs_fclose() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table[fileID].busy == 0){
		printf("sfs_fclose() error: Requested fd:%d is not assigned to an opened file\n",fileID);
		return -1;
	}else{	
		//Sets the requested file descriptor free to be used by another file.
		//This means that the file cannot longer be accessed for editing, thus 
		//it is closed
		file_descriptor_table[fileID].iNode = -69;
        file_descriptor_table[fileID].read_ptr = -1;
        file_descriptor_table[fileID].write_ptr = -1;
		file_descriptor_table[fileID].busy = 0;
		return 0;
	}
}
int sfs_frseek(int fileID, int loc){
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if(fileID >= SFS_MAX_FDENTRIES || fileID < 0){
		printf("sfs_frseek() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table[fileID].busy == 0){
		printf("sfs_frseek() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	//Check if the requiered location is less than 0
	}else if(loc < 0){
		puts("sfs_frseek() error: location requiered is negative");
		return -1;
	//Check if the requiered location is greater than file size
	}else if(loc > inode_table[file_descriptor_table[fileID].iNode].size){
		puts("sfs_frseek() error: location requiered is greater than file size");
		return -1;
	}else{
		//Move read pointer to desired location in the file
		file_descriptor_table[fileID].read_ptr = loc;
		return 0;
	}
	
}
int sfs_fwseek(int fileID, int loc){
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if((fileID >= SFS_MAX_FDENTRIES) || (fileID < 0)){
		printf("sfs_fwseek() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table[fileID].busy == 0){
		printf("sfs_fwseek() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	//Check if the requiered location is less than 0
	}else if(loc < 0){
		puts("sfs_fwseek() error: location requiered is negative");
		return -1;
	//Check if the requiered location is greater than file size
	}else if(loc > inode_table[file_descriptor_table[fileID].iNode].size){
		puts("sfs_fwseek() error: location requiered is greater than file size");
		return -1;
	}else{
		//Move read pointer to desired location in the file
		file_descriptor_table[fileID].write_ptr = loc;
		return 0;
	}
}
int sfs_fwrite(int fileID, char *buf, int length){
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if((fileID >= SFS_MAX_FDENTRIES) || (fileID < 0)){
		printf("sfs_fwrite() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table[fileID].busy == 0){
		printf("sfs_fwrite() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	}
	FileDescriptorEntry *fd = &(file_descriptor_table[fileID]);
	
	//check if there is enough space on disk to allocate requiered data
	int free_blocks = get_remaining_empty_space(&empty_block_list);
	
	int requiered_blocks = (int)((float)length/(float)SFS_API_BLOCK_SIZE);
	
	if(free_blocks < 0 || 
		(free_blocks < requiered_blocks) || 
			(requiered_blocks > 12 + NUM_INDIRECT_PTR)){
		puts("sfs_fwrite() error: There is not enough space on disk");
		return -1;
	}
	
	int written_bytes = 0;
	int check, last_block;
	int one_block = 1;
	int space_to_fill = 0;

	if(length < 0){
		puts("sfs_fwrite() error: buffer length is zero");
		return -1;
	}

	int block_ptr = (int)floor((float)fd->write_ptr / (float)SFS_API_BLOCK_SIZE);
	int block_offset = fd->write_ptr % SFS_API_BLOCK_SIZE;
	
	
	//Get index of last block used or index of a new empty block
	if(get_last_block(&inode_table, fd->iNode, block_ptr, &last_block, 
						&empty_block_list) < 0 ){
		perror("get_last_block() error");
		return -1;
	}

	if(block_offset > 0){
		//Check if the data that will be written will bigger than than the space
		//left on the block
		if((block_offset + length) >  SFS_API_BLOCK_SIZE){
			space_to_fill = SFS_API_BLOCK_SIZE - block_offset;
		}else{
			space_to_fill = length;
		}

		char *buffer = malloc(SFS_API_BLOCK_SIZE);
		check = read_blocks(last_block, 1, buffer);
		if(check <= 0){
			perror("read_blocks() error");
			return -1;
		}
		//Add amount of data to fill the block
		memcpy(buffer + block_offset, buf, space_to_fill);
		//Save last block to disk and updates the empty block list
		if(save_block(&empty_block_list, &last_block, &one_block, buffer) < 0){
			perror("save_block() error");
			return -1;
		}
		free(buffer);

		//Decrease length to write by the amount of bytes that we already 
		//written
		length -= space_to_fill;
		//Increase buf, write pointer, written bytes by the amount of bytes 
		//already written
		buf += space_to_fill;
		fd->write_ptr += space_to_fill;
		written_bytes += space_to_fill;
		//Recalculate requiered full blocks with updated length
		requiered_blocks = (int)((float)length/(float)SFS_API_BLOCK_SIZE);
	}
		
	if(length > 0){
		for(int i=0; i < requiered_blocks; i++){
			//Check if file needs to add a block to the indirect pointer list 
			//and the indirect pointer block has no been initialized
			if(inode_table[fd->iNode].link_counter >= 12 && 
					inode_table[fd->iNode].indirect_ptr == -1){
				//Get an empty block
				inode_table[fd->iNode].indirect_ptr = 
								find_first_empty_space(&empty_block_list);
				if(inode_table[fd->iNode].indirect_ptr < 0){
					perror("find_first_empty_space() error");
					return -1;
				}
				//Init indirect pointer list with default values (-1), and save
				//it to the disk
				int *buffer_ind_ptr = malloc(SFS_API_BLOCK_SIZE);
				for(int i=0; i < NUM_INDIRECT_PTR; i++){
					buffer_ind_ptr[i] = -1;
				}
				//Save indirection pointer block to the disk and update empty
				//block list
				if(save_block(&empty_block_list, 
					&(inode_table[fd->iNode].indirect_ptr), 
						&one_block, buffer_ind_ptr) < 0){
					perror("save_block() error");
					return -1;
				}
				free(buffer_ind_ptr);	
			}
			if(inode_table[fd->iNode].link_counter >= 12){
				/*
				Get indirection pointer block from disk, find an empty block and
				added it to the indirection pointer list. Save the indirection 
				pointer list back to its associated block
				*/
				int *buffer_ind_ptr = malloc(SFS_API_BLOCK_SIZE);
				check = read_blocks(inode_table[fd->iNode].indirect_ptr, 1, 
										buffer_ind_ptr);
				if(check <= 0){
					perror("read_blocks() error");
					return -1;
				}
				last_block = find_first_empty_space(&empty_block_list);
				if(last_block < 0){
					perror("find_first_empty_space() error");
					return -1;
				}
				buffer_ind_ptr[block_ptr -12] = last_block;
				check = write_blocks(inode_table[fd->iNode].indirect_ptr, 1, 
							buffer_ind_ptr);
				if(check <= 0){
					perror("write_blocks() error");
					return -1;
				}
				free(buffer_ind_ptr);
			}else{
				/*
				Finds an empty block and add it to the direct pointer list
				*/
				last_block = find_first_empty_space(&empty_block_list);
				if(last_block < 0){
					perror("find_first_empty_space() error");
					return -1;
				}
				inode_table[fd->iNode].direct_ptr[block_ptr] = last_block;
			}

			//Increase block pinter and link counter, since new blocks have been
			//added to the direct pointer list or indirect pointer list
			block_ptr++;
			inode_table[fd->iNode].link_counter++;

			//Get a whole block of data (1024 bytes) to the buffer and save it
			char *buffer = malloc(SFS_API_BLOCK_SIZE);
			memcpy(buffer, buf, SFS_API_BLOCK_SIZE);
			
			//Increase pointer to next position
			buf += SFS_API_BLOCK_SIZE;
			//Increase all data counter
			fd->write_ptr += SFS_API_BLOCK_SIZE;
			written_bytes += SFS_API_BLOCK_SIZE;
			//Decrease the length by what have been written already
			length -= SFS_API_BLOCK_SIZE;
			
			//Save buffer to memory and update empty block list
			if(save_block(&empty_block_list, &last_block, &one_block, buffer) < 0){
				perror("save_block() error");
				return -1;
			}
			free(buffer);
		}

		//This parts handles the case where we have more data to save but is 
		//less than a whole block size
		if(length > 0){
			char *buffer = malloc(SFS_API_BLOCK_SIZE);
			int reminaing_data = length;

			//Gets the next block to write the data to, either from direct or 
			//indirect pointer list or an empty block
			if(get_last_block(&inode_table, fd->iNode, block_ptr, 
								&last_block, &empty_block_list) < 0){
				perror("get_ptr() error");
				return -1;
			}
		
			check = read_blocks(last_block, 1, buffer);
			if(check <= 0){
				perror("read_blocks() error");
			}
			memcpy(buffer, buf, reminaing_data);
			//Save buffer to memory and update empty block list
			if(save_block(&empty_block_list, &last_block, &one_block, buffer) < 0){
				perror("save_block() error");
				return -1;
			}
			free(buffer);

			fd->write_ptr += reminaing_data;
			written_bytes += reminaing_data;				
		}
		
	}
	
	inode_table[fd->iNode].size += written_bytes;

	//Save updated inode table to the disk
	if(save_Inode_table(&inode_table) < 0){
		perror("save_Inode_table() error");
		return -1;
	}
	//Update data structures from disk to memory
	if(get_root_dir(&root_dir, &inode_table, &super_block) < 0){
		perror("get_root_dir() error");
		return -1;
	}


	return written_bytes;

}

int sfs_fread(int fileID, char *buf, int length){
	//check if the the file descriptor exists, since there is a fixed number of 
	//file descriptors available
	if(fileID >= SFS_MAX_FDENTRIES || fileID < 0){
		printf("sfs_fread() error: Requested fd: %d does not exist\n",fileID);
		return -1;
	//check if the file descriptor is actually not being used	
	}else if(file_descriptor_table[fileID].busy == 0){
		printf("sfs_fread() error: Requested fd:%d is not assigned to an \
				opened file\n",fileID);		
		return -1;
	}
	FileDescriptorEntry *fd = &(file_descriptor_table[fileID]);

	if(length < 0){
		puts("sfs_fread() error: Attempt to read a negative lenght");
		return -1;
	}else if(inode_table[fd->iNode].size == 0){
		puts("sfs_fread() error: Attempt to read to an empty file");
		return -1;
	}
	
	//If user tries to read a greater length than the file size, the amount of
	//bytes read get truncated at file size
	int read_length;
	if(length > inode_table[fd->iNode].size){
		read_length = inode_table[fd->iNode].size;
	}else{
		read_length = length;
	}
	
	int block_ptr = (int)floor((float)fd->read_ptr / (float)SFS_API_BLOCK_SIZE);;
	int block_offset = fd->read_ptr % SFS_API_BLOCK_SIZE;

	int read_bytes = 0;
	int check, block_to_read;
	int data_to_read = 0;

	//Get index of last block used or index of a new empty block
	if(get_last_block(&inode_table, fd->iNode, block_ptr, &block_to_read, 
						&empty_block_list) < 0 ){
		perror("get_last_block() error");
		return -1;
	}

	if(block_offset > 0){
		//Assure to read sufficient amount of bytes for first block
		if(block_offset + read_length > SFS_API_BLOCK_SIZE){
			data_to_read = SFS_API_BLOCK_SIZE - block_offset;
		}else{
			data_to_read = read_length;
		}
	
		//Get first block to be read and copy the contents specified to a buffer
		char *buffer = malloc(SFS_API_BLOCK_SIZE);
		check = read_blocks(block_to_read, 1, buffer);
		if(check <= 0){
			perror("read_blocks() error");
			return -1;
		}
		memcpy(buf, buffer + block_offset, data_to_read);
		free(buffer);

		//Update the lenght to be read by the amount of what have been read 
		//already
		read_length -= data_to_read;
		//Increase all data counters
		fd->read_ptr += data_to_read;
		read_bytes += data_to_read;
		//Increase data buffer pointer to save data at the next available 
		//location
		buf += data_to_read;
		//Increse block pointer and set block offset to 0
		block_ptr++;
		block_offset = 0;
	}
	//Amount of full blocks to read
	int requiered_blocks = (int)((float)read_length / (float)SFS_API_BLOCK_SIZE);
		
	if(read_length > 0){
		for(int i=0; i < requiered_blocks; i++){
			//Get block of indirect pointer list of from direct pointer list
			if(block_ptr >= 12){
				int *buffer_ind_ptr = malloc(SFS_API_BLOCK_SIZE);
				check = read_blocks(inode_table[fd->iNode].indirect_ptr, 1, 
										buffer_ind_ptr);
				if(check <= 0){
					perror("read_blocks() error");
					return -1;
				}
				block_to_read = buffer_ind_ptr[block_ptr -12];
				free(buffer_ind_ptr);
			}else{
				block_to_read = inode_table[fd->iNode].direct_ptr[block_ptr];
			}
			//Get block from disk
			char *buffer = malloc(SFS_API_BLOCK_SIZE);
			check = read_blocks(block_to_read, 1, buffer);
			if(check <= 0){
				perror("read_blocks() error");
				return -1;
			}
			//Add one whole block of data to the buffer
			memcpy(buf, buffer, SFS_API_BLOCK_SIZE);
			free(buffer);

			//Decrease lennght to be read by whole block size amount
			read_length -= SFS_API_BLOCK_SIZE;
			//increase data counters
			fd->read_ptr += SFS_API_BLOCK_SIZE;
			read_bytes += SFS_API_BLOCK_SIZE;
			//Increment block pointer
			block_ptr++;
			//Increment buffer to save read data at next available location
			buf += SFS_API_BLOCK_SIZE;
			
		}

		//Case where theres is still data to read but it is less than a whole
		//block size
		if(read_length > 0){
			int reminaing_data = read_length;
			if(block_ptr >= 12){
				int *buffer_ind_ptr = malloc(SFS_API_BLOCK_SIZE);
				check = read_blocks(inode_table[fd->iNode].indirect_ptr, 1, 
										buffer_ind_ptr);
				if(check <= 0){
					perror("read_blocks() error");
					return -1;
				}
				block_to_read = buffer_ind_ptr[block_ptr -12];
				free(buffer_ind_ptr);
			}else{
				block_to_read = inode_table[fd->iNode].direct_ptr[block_ptr];
			}
			
			//Get next block to read from
			char *buffer = malloc(SFS_API_BLOCK_SIZE);
			check = read_blocks(block_to_read, 1, buffer);
			if(check <= 0){
				perror("read_blocks() error");
			}
			memcpy(buf, buffer, reminaing_data);
			free(buffer);

			fd->read_ptr += reminaing_data;
			read_bytes += reminaing_data;
			read_length -= reminaing_data;
			buf += reminaing_data;	
		}

	}
	
	return read_bytes;
}

int sfs_remove(char *file){
	DirectoryEntry *file_entry = malloc(sizeof(DirectoryEntry));
	int *indirection_ptr = 0;
	
 	//Check if file exists
 	if(get_file(&root_dir, &file_entry, file) < 0){
 		puts("sfs_remove() error: requested file does not exist");
 		return -1;
 	}

 	//Get file index at root directory
 	int file_index;
 	int default_inode_index = -1;
 	for(int i=0; i < MAX_INODES; i++){
 		if(strncmp(file, root_dir[i].filename, SFS_MAX_FILENAME + 1) == 0){
 			file_index = i;
 			break;
 		}
 	}

 	//Close the file if it is opened
 	for(int i=0; i < SFS_MAX_FDENTRIES; i++){
 		if(file_descriptor_table[i].iNode == file_entry->inode_index){
 			int check = sfs_fclose(i);
		 	if(check < 0){
		 		puts("File was already closed");
		 	}
		 	break;
 		}
 	}

 	//Remove file from root directory
 	if(update_root_dir(&root_dir, &inode_table, &super_block, 
 						&default_inode_index, &file_index, '\0', REMOVE) < 0){
 		perror("update_root_dir() error");
 		return -1;
 	}

 	//Remove all reference to allocated blocks for the file. It is faster to set
 	//all of its blocks to "free" than actually erase the data. New data will 
 	//override it
 	for(int i=0; i < inode_table[file_entry->inode_index].link_counter; i++){
 		if(i >= 12){
 			if(indirection_ptr == 0){
 				if(get_indirection_block(&indirection_ptr,&inode_table, 
 											file_entry->inode_index)< 0){
					perror("get_indirection_block() error");
					return -1;
				}
 			}
 			//Update indirection pointer block to free
 			if(update_empty_block_list(&empty_block_list, 
 							inode_table[file_entry->inode_index].indirect_ptr, 
 								1, EMPTY) < 0){
	 			perror("update_empty_block_list() errror");
	 			return -1;
 			}
 			//Set all block index saved in indirection pointer list
 			for(int i=0; 
 				i < sizeof(indirection_ptr)/sizeof(indirection_ptr[0]); 
 					i++){
 				if(update_empty_block_list(&empty_block_list, indirection_ptr[i], 
 										1, EMPTY) < 0){
	 				perror("update_empty_block_list() errror");
	 				return -1;
 				}
 			}
 			break;
 		}
 		
 		if(update_empty_block_list(&empty_block_list, 
 							inode_table[file_entry->inode_index].direct_ptr[i], 
 								1, EMPTY) < 0){
 			perror("update_empty_block_list() errror");
 			return -1;
 		}
 	}

 	//Reset file iNode
 	inode_table[file_entry->inode_index].mode = 0;
	inode_table[file_entry->inode_index].size = 0;
	inode_table[file_entry->inode_index].link_counter = 0;
	inode_table[file_entry->inode_index].indirect_ptr = -1;
	//memset(inode_table[file_entry->inode_index].direct_ptr, -1, 12); 
	for(int i=0; i < 12; i++){
		inode_table[file_entry->inode_index].direct_ptr[i] = -1;
	}

	if(save_Inode_table(&inode_table) < 0){
		perror("save_Inode_table() error");
		return -1;
	}	

	total_files--;

 	return 1;
}