/**
 * Mad Mad Access Pattern
 * CS 241 - Spring 2017
 */
#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/
int debug = 0;
void recursion_helper(FILE* data_file, int offset, int idx, char **argv){
	debug++;
	if(offset <= 0){
		printNotFound(argv[idx]);
		return;
	}
	
	if(fseek(data_file, offset, SEEK_SET) == -1){ // getting rid of header
		formatFail(argv[1]);
		exit(2);
	}

	BinaryTreeNode my_node;
	fread(&my_node, sizeof(BinaryTreeNode), 1, data_file);

	if(fseek(data_file, offset + sizeof(BinaryTreeNode), SEEK_SET) == -1){ // nothing
		formatFail(argv[1]);
		exit(2);
	}else{
		char my_word[128];
		if(!fgets(my_word, 128, data_file)){
			formatFail(argv[1]);
			exit(2);
		}else{
			//printf("my_word is %s\n", my_word);
			int status = strcmp(argv[idx], my_word);
			if(!status){
				printFound(argv[idx], my_node.count, my_node.price);
				return;
			}else if(status > 0){
				recursion_helper(data_file, my_node.right_child, idx, argv);
			}else{
				recursion_helper(data_file, my_node.left_child, idx, argv);
			}
		}
	}

	//printf("debug value is %d\n", debug);
}

int main(int argc, char **argv) { 
	if(argc <= 2){
		printArgumentUsage();
		exit(1);
	}
	FILE* data_file = fopen(argv[1], "r");
	if(!data_file){
		openFail(argv[1]);
		exit(2);
	}

	char header_buffer[16];
	fread(header_buffer, 4, 1, data_file);
	if(strcmp(header_buffer, "BTRE") != 0){
		formatFail(argv[1]);
		exit(2);
	}

	//fseek(data_file, 4, SEEK_SET); // getting rid of the header

	for(int i=2; i<argc; i++){

		recursion_helper(data_file, 4, i, argv);
	}
	
	fclose(data_file);
	return 0;
}
