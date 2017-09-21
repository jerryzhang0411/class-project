/**
 * Extreme Edge Cases Lab
 * CS 241 - Spring 2017
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

char **camel_caser(const char *input_str) { 
	if(input_str == NULL){
		return NULL;
	}
	int size = strlen(input_str);
	char input_s[size+1];
	for(int a=0; a<(int)strlen(input_str); a++){
		input_s[a] = input_str[a];
	}
	char** retVal = malloc(128);
	char* temp = malloc(size);
	int i=0; //whole sentence iterator
	int curr = 0; //retVal iterator
	int iscompleted = 0;
	for(int b=0; b<(int)strlen(input_s); b++){
		//printf("%d\n", i);
		//0printf("%c\n", input_s[i]);
		if(isspace(input_s[b])!=0){
			i++;
		}else if(ispunct(input_s[b])!=0){// is punct
			i++;
		}else if(isalpha(input_s[b])!=0){// is alpha
			break;
		}else if(isdigit(input_s[b])!=0){
			break;
		}else{
			i++;
		}
	}
	for(int c=0; c<(int)strlen(input_s); c++){
		if(ispunct(input_s[c])!=0){
			iscompleted = 1;
		}
	}
	if(iscompleted == 0){
		retVal[0] = NULL;
		return retVal;
	}
	if(isalpha(input_str[i])){
		input_s[i] = tolower(input_str[i]);
	}else{
		input_s[i] = input_str[i];
	}
	int sub_i=0; //iterator of each clauses
	for(; i<(int)strlen(input_s); i++){
		if(input_s[i]=='\0'){
			break;
		}
		if(ispunct(input_s[i]) != 0){ //is punctuation
			temp[sub_i] = 0;
			sub_i = 0;
			retVal[curr] = temp;
			curr++;
			temp = malloc(size);
			while(isalpha(input_s[i]) ==0){ // not alphabet
				if(isdigit(input_s[i])){ // is digit
					temp[sub_i] = input_s[i];
					sub_i++;
					i++;
				}else{ //not digit, not alphabet
					i++;
				}
			}
			if(isalpha(input_s[i])){
				temp[sub_i] = tolower(input_s[i]); //is alphabet
			}else{
				temp[sub_i] = input_s[i];
			}
			
			sub_i++;
			continue;
		}

		if(isspace(input_str[i]) == 0){ // not space
			char tempHold;
			if(isalpha(input_s[i])&&isupper(input_s[i])){ // is upper
				tempHold = tolower(input_s[i]);
				temp[sub_i] = tempHold;
				sub_i++;
				//i++;
			}else{ // is lower
				temp[sub_i] = input_s[i];
				sub_i++;
			}
		}else{ // is space ONLY
			while(isspace(input_s[i]) != 0){
				i++;
			}
			if(isalpha(input_s[i])){
				temp[sub_i] = toupper(input_s[i]);
			}else{
				temp[sub_i] = input_s[i];
			}
			
			sub_i++;
		}
	}
	retVal[curr] = NULL; 
	//printf("%s", retVal[curr-1]);
	return retVal; 
}