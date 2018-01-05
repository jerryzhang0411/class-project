/**
 * MapReduce
 * CS 241 - Spring 2017
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

void close_both_end(int* input){
    close(input[0]);
    close(input[1]);
}

int countera = 0;
int counterb = 0;
int main(int argc, char **argv) {

    int mapper_count = atoi(argv[5]);

    if(argc != 6 || mapper_count <= 0){
        printf("argc or mapper_count failed!\n");;
        return 1;
    } // edge cases

    FILE* input_file = fopen(argv[1], "r");
    int output_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU); // Open the output file.
    if(!input_file || !output_file){
        perror("File Error!");
        exit(1);
    }

    int my_mapper_status[mapper_count];
    int my_reducer_status;
    pid_t all_my_mapper[mapper_count];
    pid_t my_only_reducer;
    char indicator[256];
    int all_my_input_pipes[mapper_count][2];
    
    for(int i=0; i < mapper_count; i++){
        pipe(all_my_input_pipes[i]);
    } // Create an input pipe for each mapper.

    int reducer_pipe[2];
    pipe(reducer_pipe); // Create one input pipe for the reducer.

    for(int i=0; i < mapper_count; i++){
        countera++;
        pid_t fork_status = fork();
        if(fork_status == -1){
            perror("Fork Failed!");
            exit(1);
        }else if(fork_status == 0){ // child
            close(all_my_input_pipes[i][0]);
            dup2(all_my_input_pipes[i][1], 1);
            close(all_my_input_pipes[i][1]);
            close_both_end(reducer_pipe);
            sprintf(indicator, "%d", i);
            execlp("./splitter", "./splitter", argv[1], argv[5], indicator, NULL);
            perror("Exec Failed1"); // only print when if execlp failed
            exit(1);
        } //else{
        //     char buf[100]; 
        //     read(all_my_input_pipes[i][0], buf, 100); 
        //     fprintf(stderr, "curr: %s\n",buf );
        // }
            //printf("countera is %d\n", countera);
    } // Start a splitter process for each mapper.

    
    for(int i=0; i < mapper_count; i++){
        counterb++;
        all_my_mapper[i] = fork();
        if(all_my_mapper[i] == -1){
            perror("Fork Failed!");
            return 1;
        }else if(all_my_mapper[i] == 0){
            close(all_my_input_pipes[i][1]);
            close(reducer_pipe[0]);
            dup2(all_my_input_pipes[i][0], 0);
            dup2(reducer_pipe[1], 1);
            execlp(argv[3], argv[3], NULL);
            perror("Exec Failed2");
            exit(1);
        }
        close_both_end(all_my_input_pipes[i]);
        //printf("counterb is %d\n", counterb);
    } // Start all the mapper processes.

    my_only_reducer = fork();
    if(my_only_reducer == -1){
        perror("Fork Failed!");
        return 1;
    }else if(my_only_reducer == 0){
        close(reducer_pipe[1]);
        dup2(reducer_pipe[0], 0);
        dup2(output_file, 1);
        execlp(argv[4], argv[4], NULL);
        perror("Exec Failed3");
        exit(1);
    }// Start the reducer process.

    close_both_end(reducer_pipe);

    for(int i=0; i<mapper_count; i++){
        close_both_end(all_my_input_pipes[i]);
    }
    for(int i=0; i<mapper_count; i++){
        //printf("imhere\n");
        waitpid(all_my_mapper[i], &my_mapper_status[i], 0); // Wait for the reducer to finish.
        if(my_mapper_status[i]!=0){
            printf("%d mapper failed with status %d\n", i, my_mapper_status[i]); // Print nonzero subprocess exit codes.
        }
    }

    waitpid(my_only_reducer, &my_reducer_status, 0);
    if(my_reducer_status!=0){
        printf("the reducer failed with status %d\n", my_reducer_status);
    }
    fclose(input_file);
    close(output_file);


    print_num_lines(argv[2]); // Count the number of lines in the output file.

    return 0;
}