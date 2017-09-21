/**
 * Machine Problem: Shell
 * CS 241 - Fall 2016
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct process {
  char *command;
  char *status;
  pid_t pid;
} process;

void noyoudont(int signum){
	if(signum){
		return;
	}
}



char myDir[1024];    // current working dir
char wala[1024];

void * processCopyConstructor(void *elem){
	process * retVal = (process*)elem;
	((process*)elem)->command = strdup(retVal->command);
	((process*)elem)->status = retVal->status;
	((process*)elem)->pid = retVal->pid;
	return retVal;
}

void processDestructor(void *elem){
	if(elem != NULL){
		free(((process*)elem)->command);

	}
	free((process*)elem);
	elem = NULL;
}

void * processDefaultConstructor(void){
	process * retVal = malloc(sizeof(process));
	retVal->command = NULL;
	retVal->status = STATUS_RUNNING;
	retVal->pid = 0;
	return retVal;
}

static vector * myProcesses;

size_t myFind(pid_t inputpid){
	for(size_t i=0; i<vector_size(myProcesses); i++){
		process* temp = (process*)(vector_get(myProcesses, i));
		if(temp->pid == inputpid){
			return i;
		}
	}
	return (size_t)1103711037;
}

void myHandler(){
	pid_t myPid;
	int status;
	size_t temp;
	while ((myPid = waitpid((pid_t) (-1), &status, WNOHANG) )> 0){
		if(WIFSTOPPED(status)){
			return;
		}else{
			temp = myFind(myPid);
			if(temp != 1103711037){
				vector_erase(myProcesses, temp);
				return;
			}
		}
	}
}

void myChangeDir(char ** myCompare){
	if(myCompare[1] == NULL){
		print_invalid_command(myCompare[0]);
		return;
	}
	if(*myCompare[1] != '/'){
		char* dest = get_full_path(myCompare[1]);
		if(chdir(dest) != 0){
			//printf("my dest is:%s", dest);
			print_no_directory(myCompare[1]);
			return;
		}
		free(dest);
	}else{
		if(myCompare[1] != 0){
			print_no_directory(myCompare[1]);
			return;
		}
	}
	getcwd(myDir, sizeof(myDir));
	
}

void myKill(char ** myCompare){
	if(!myCompare[1]){
		const char* command = myCompare[0];
		print_invalid_command(command);
	}
	pid_t my_pid = atoi(myCompare[1]);
	//printf("my pid is:%d\n", my_pid);
	if(myFind(my_pid) != 1103711037){
		kill(my_pid, SIGTERM);
		process * temp = (process*)(vector_get(myProcesses, myFind(my_pid)));
		print_killed_process(my_pid, temp->command);
    }else{
    	print_no_process_found(my_pid);
    }
}

void myStop(char ** myCompare){
	if(!myCompare[1]){
		const char* command = myCompare[0];
		print_invalid_command(command);
		return;
	}
	pid_t my_pid = atoi(myCompare[1]);
	if(myFind(my_pid) != 1103711037){
		kill(my_pid, SIGSTOP);
		process * temp = (process*)(vector_get(myProcesses, myFind(my_pid)));
		temp->status = STATUS_STOPPED;
		print_stopped_process(my_pid, temp->command);
	}else{
		print_no_process_found(my_pid);
		return;
	}
}

void myCont(char ** myCompare){
	if(!myCompare[1]){
		const char* command = myCompare[0];
		print_invalid_command(command);
		return;
	}
	pid_t my_pid = atoi(myCompare[1]);
	if(myFind(my_pid) != 1103711037){
		kill(my_pid, SIGCONT);
		process * temp = (process*)(vector_get(myProcesses, myFind(my_pid)));
		temp->status = STATUS_RUNNING;
	}else{
		print_no_process_found(my_pid);
		return;
	}
}

int isBgd(char * myTest){
	char* last_letter = &myTest[strlen(myTest)-1];
	if(strcmp(last_letter, "&") == 0){
		return 1;
	}else{
		return 0;
	}
}


void myPS(){
	//printf("your input is %s\n", bytes);
	print_process_info(STATUS_RUNNING, getpid(), wala);
	for(int i=0; i<(int)vector_size(myProcesses); i++){
		process * temp = (process*)(vector_get(myProcesses, i));
		print_process_info(temp->status, temp->pid, temp->command);
	}
}

void cleanup(){
	//int status;
  	//while (waitpid((pid_t) (-1), 0, WNOHANG) > 0) {
  		for(size_t i=0; i<vector_size(myProcesses); i++){
			process * temp = (process*)(vector_get(myProcesses, i));
			kill(temp->pid, SIGKILL);
		}

  	//}
	
		vector_destroy(myProcesses);
	
  	exit(0);
}


void Bgd_Exe(char*bytes){
	size_t numtokens = 0;
	const char * str = bytes;
	char ** myCompare = strsplit(str, " ", &numtokens);
	pid_t myPid = fork();
	if(myPid == 0){  // child
		if(execvp(myCompare[0], myCompare) < 0){
			//printf("1\n");
			print_exec_failed(bytes);
			exit(1);
		}
	}else if(myPid == -1){
		//printf("2\n");
		print_fork_failed();
		exit(1);
	}else{  // parent

		print_command_executed(myPid);
		process* aprocess = processDefaultConstructor();
		aprocess->command = strdup(bytes);
		aprocess->status = STATUS_RUNNING;
		aprocess->pid = myPid;
		vector_push_back(myProcesses, aprocess);
		
		//printf("%zu\n", vector_size(myProcesses));
	}
	free_args(myCompare);
}

void external_Exe(char* bytes){
	size_t numtokens = 0;
	const char * str = bytes;
	char ** myCompare = strsplit(str, " ", &numtokens);
	pid_t myPid = fork();
	if(myPid == 0){  // child
		if(execvp(myCompare[0], myCompare) < 0){
			//printf("1\n");
			print_exec_failed(bytes);
			exit(1);
		}
	}else if(myPid == -1){
		//printf("2\n");
		print_fork_failed();
		exit(1);
	}else{  // parent

		print_command_executed(myPid);
		process* aprocess = processDefaultConstructor();
		char * ctemp = malloc(strlen(bytes)+1);
		ctemp[strlen(bytes)] = '\0';
		for(size_t i=0; i<strlen(bytes); i++){
			ctemp[i] = bytes[i];
		}
		aprocess->command = ctemp;
		aprocess->status = STATUS_RUNNING;
		aprocess->pid = myPid;
		vector_push_back(myProcesses, aprocess);

		int status;
		waitpid(myPid, &status, 0);
		size_t temp = myFind(myPid);
		free(ctemp);
		vector_erase(myProcesses, temp);
		//printf("%zu\n", vector_size(myProcesses));
	}
	free_args(myCompare);
	
}

void Buildin_Exe(char *bytes){
			  //pid_t myPid = getpid(); //foreground
		      
		      //print_prompt(myDir, myPid);
		      //printf("my bytes is %s\n", bytes);
		      size_t numtokens = 0;
		      const char * str = bytes;
		      char ** myCompare = strsplit(str, " ", &numtokens);
		      if(strcmp(myCompare[0], "cd") == 0){ // change directory
		      	myChangeDir(myCompare);
		      }else if(strcmp(myCompare[0], "ps") == 0){ // print process info
		      	myPS();
		      }else if(strcmp(myCompare[0], "kill") == 0){ // kill
		      	myKill(myCompare);
		      }else if(strcmp(myCompare[0], "stop") == 0){
		      	myStop(myCompare);
		      }else if(strcmp(myCompare[0], "cont") == 0){
		      	myCont(myCompare);
		      }else if(strcmp(myCompare[0], "exit") == 0){
		      	cleanup();
		      }else{ // external
		      	if(isBgd(myCompare[0])){
		      		Bgd_Exe(bytes);
		      	}else{
		      		external_Exe(bytes);
		      	}
		      }
		      free_args(myCompare);
}



int shell(int argc, char *argv[]) {
	signal(SIGCHLD, myHandler);
	signal(SIGINT, noyoudont);
	//signal(SIGCHLD, cleanup);
  // TODO: This is the entry point for your shell.
  if(argc == 0 || argv == NULL){
  	exit(1);
  }
  myProcesses = vector_create(processCopyConstructor, processDestructor, processDefaultConstructor);
  for(int i=0; i<argc; i++){
  	strcat(wala, argv[i]);
  	strcat(wala, " ");
  }
  if(argc == 3){
  	FILE * file = fopen(argv[2], "r+");                
  	if(strcmp(argv[1], "-f") == 0){ //./zzz -f xxx.txt
		if(!file){
		  print_script_file_error();
		  exit(1);
		}else{
			char* bytes = NULL;
		    size_t len = 0;
		    while((getline(&bytes, &len, file) != -1)){
		    	getcwd(myDir, sizeof(myDir));
		    	if(strcmp(bytes, "\n") == 0){
	  				print_prompt(myDir, getpid());
		    		continue;
		    	}	
		      if(strchr(bytes, '\n') != NULL){
		        char * temp = strchr(bytes, '\n');
		        *temp = '\0';
		        //printf("my cwd is %s\n", myDir);
		        print_prompt(myDir, getpid());
		      }
		      if(isBgd(bytes)){  // have & at the back, background exp: cd djfkal&
		      	bytes[strlen(bytes)-1] = '\0';
		      	print_command(bytes);
		      	Bgd_Exe(bytes);
		      }else{
		      	print_command(bytes);
		      	Buildin_Exe(bytes);

		      }
		      
		    }
		    free(bytes);
		    bytes = NULL;
		    fclose(file);
		    pid_t myPid = getpid();
			print_prompt(myDir, myPid);  // print prompt for the next commend
		  } // end of while
  	}
  }else if(argc == 1){
  	getcwd(myDir, sizeof(myDir));
  	pid_t myPid = getpid();
  	print_prompt(myDir, myPid);
  	char * bytes = NULL;
  	size_t size;
  	while(1){
  		getcwd(myDir, sizeof(myDir));
  		if(getline(&bytes, &size, stdin) == -1){ // no input line
  			cleanup();
	  		exit(0);
	  	}else{
	  		if(strcmp(bytes, "\n") == 0){
	  			print_prompt(myDir, myPid);
		    	continue;
		    }
	  		if(strchr(bytes, '\n') != NULL){
		        char * temp = strchr(bytes, '\n');
		        *temp = '\0';
		    }
		    
	  		if(isBgd(bytes)){  // have & at the back, background
		      	bytes[strlen(bytes)-1] = '\0';
		      	Bgd_Exe(bytes);
		      }else{
		      	Buildin_Exe(bytes);
		      }
			myPid = getpid();
			print_prompt(myDir, myPid);  // print prompt for the next commend

		}
		free(bytes);
		bytes = NULL;
  	}
  	
  }else{  // invalid use of shell
  	print_usage();
  }
  cleanup();
  return 0;
}
