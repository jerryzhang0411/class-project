/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#include "pied_piper.h"
#include "utils.h"
#include <fcntl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRIES 3

int first_flag = 1;
int last_flag = 0;
char* cmd1;
char* cmd2;

void runpipe(int pfd[])
{
	int pid;

	switch (pid = fork()) {

	case 0: /* child */
		dup2(pfd[0], 0);
		close(pfd[1]);	/* the child does not need this end of the pipe */
		execvp(&cmd2[0], &cmd2);
		perror(&cmd2[0]);

	default: /* parent */
		dup2(pfd[1], 1);
		close(pfd[0]);	/* the parent does not need this end of the pipe */
		execvp(&cmd1[0], &cmd1);
		perror(&cmd1[0]);

	case -1:
		perror("fork failed");
		exit(1);
	}
}

int pied_piper(int input_fd, int output_fd, char **executables) {
  // Your code goes here...
	for(int i=0; executables[i]!=NULL; i++){
		
		cmd1 = executables[i];
		cmd2 = executables[i+1];
		if(cmd2 == NULL){
			last_flag = 1;
			if(output_fd == -1){
				execvp
			}
			break;
		}

		int pid, status;
		int fd[2];

		pipe(fd);

		switch (pid = fork()) {

		case 0: /* child */
			runpipe(fd);
			exit(0);

		default: /* parent */
			while ((pid = wait(&status)) != -1)
				fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
			break;

		case -1:
			perror("fork failed");
			exit(1);
		}
	}
    
    return EXIT_SUCCESS;
}
