/**
* Finding Filesystems
* CS 241 - Spring 2017
*/
#include "format.h"
#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int compare_min(uint64_t number1, uint64_t number2){
	if(number1 < number2){
		return number1;
	}else{
		return number2;
	}
}

void fs_ls(file_system *fs, char *path) {
    // Arrrrrgh Matey
	inode* my_node = get_inode(fs, path);
    if(!my_node){
    	print_no_file_or_directory();
    	return;
    }else{
    	if(is_file(my_node)){
    		print_file(path);
    	}else{
    		uint64_t total_size = my_node->size;
    		while(1){
    			for(int i=0; i<NUM_DIRECT_INODES; i++){
    				data_block_number temp_number = my_node->direct[i];
    				data_block actual_block = fs->data_root[temp_number];
    				dirent my_dirent;
    				char* starting_point = (char*)&actual_block;
    				for(int i=0; i<(int)sizeof(data_block); i+=256){
    					if(!total_size){
    						return;
    					}
    					make_dirent_from_string(starting_point, &my_dirent);
    					inode* temp_node = fs->inode_root + my_dirent.inode_num; // points to the inode under process
    					if(!temp_node){
    						print_no_file_or_directory();
    						return;
    					}else if(is_file(temp_node)){
    						print_file(my_dirent.name);
    					}else if(is_directory(temp_node)){
    						print_directory(my_dirent.name);
    					}
    					total_size -=256;
    					starting_point += 256;
    				}
    			}
    			my_node = fs->inode_root + my_node->indirect; // updating for indirect nodes
    		}
    	}
    }
}

void fs_cat(file_system *fs, char *path) {
    // Shiver me inodes
    inode* my_node = get_inode(fs, path);
    if(!my_node){
    	print_no_file_or_directory();
    	return;
    }else{
    	uint64_t total_size = my_node->size;
    	while(1){
    		for(int i=0; i<NUM_DIRECT_INODES; i++){
	    		data_block_number temp_number = my_node->direct[i];
	    		data_block actual_block = fs->data_root[temp_number];
	    		write(1, &actual_block, compare_min(total_size, sizeof(data_block))); // wrting either size or remaining bytes to the block
	    		total_size -= compare_min(total_size, sizeof(data_block));
	    		if(!total_size){
	    			return;
	    		}
	    	}
	    	my_node = fs->inode_root + my_node->indirect; // updating for indirect nodes
    	}	
    }
    return;
}