/**
 * Networking
 * CS 241 - Spring 2017
 */
#include "common.h"
#include "format.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

int serverSocket;
char **parse_args(int argc, char **argv);
verb check_args(char **args);

static const size_t MESSAGE_SIZE_DIGITS = 4;

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) { //reference: lab chatroom
    // Your Code Here
    size_t total_read = 0;
    while(total_read < count){
        ssize_t buffer_count = read(socket, buffer+total_read, (ssize_t)(count - total_read));
        if(buffer_count > 0){
            total_read+=buffer_count;
        }else if(!buffer_count){
            return total_read;
        }else if(buffer_count == -1 && errno != EINTR){
            return -1;
        }else if(buffer_count == -1 && errno == EINTR){
            continue;
        }
    }
    return total_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) { //reference: lab chatroom
    // Your Code Here
    size_t total_read = 0;
    while(total_read < count){
        ssize_t buffer_count = write(socket, buffer+total_read, (ssize_t)(count - total_read));
        if(buffer_count > 0){
            total_read+=buffer_count;
        }else if(!buffer_count){
            return total_read;
        }else if(buffer_count == -1 && errno != EINTR){
            return -1;
        }else if(buffer_count == -1 && errno == EINTR){
            continue;
        }
    }
    return total_read;
}


ssize_t get_message_size(int socket) { //reference: lab chatroom
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

void get_helper(char* local, char* remote){

  int local_fd = open(local, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
  if(!local_fd){
    print_client_help();
    exit(1);
  }
  
  char upload_buffer[1024];
  // FILE* local_f = fopen(local, "r");
  // fseek(local_f, 0L, SEEK_END);
  // size_t file_size = ftell(local_f);
  // printf("file_size is %zu\n", file_size);
  // fseek(local_f, 0L, SEEK_SET);
  

  sprintf(upload_buffer, "GET %s\n", remote);
  write_all_to_socket(serverSocket, upload_buffer, strlen(upload_buffer));
  // char second_buffer[1024];
  // memcpy(second_buffer, &file_size, sizeof(size_t));
  // strcat(second_buffer, " ");
  // strcat(second_buffer, remote);
  // // printf("first buffer is: %s\n", buffer);
  // // printf("second buffer is: %s\n", second_buffer);
  // //write(serverSocket, buffer, buffer_size);
  // write_all_to_socket(serverSocket, second_buffer, strlen(second_buffer));
  shutdown(serverSocket, SHUT_WR);
  ssize_t retval = 1;
  //int temp_flag = 0;

  char buffer[256];
  ssize_t total_size = 0;
  memset(buffer, 0, 11);

  retval = read_all_from_socket(serverSocket, buffer, 11);
  memcpy(&total_size, buffer+3, sizeof(size_t));

  char temp[5];
  memset(temp, 0, 5);
  memcpy(&temp, buffer, 1);
  temp[1] = '\0';
  if(strcmp(temp, "O") != 0){
    print_client_help();
    exit(1);
  }


  // char temp[11];
  // memset(temp, 0, 11);
  // memcpy(&temp, buffer, 11);
  // temp[1] = '\0';
  // if(strcmp(temp, "O") != 0){
  //   read(serverSocket)
  // }else if(strcmp(temp, "E") != 0){

  // }else{
  //   print_invalid_response();
  // }

  //total_size-=1;
  //total_size +=1;
  //printf("total size is %zu\n", total_size);
  ssize_t now_my_size = 0;
  while (retval > 0) {
    memset(buffer, 0 , 256);
    retval = read_all_from_socket(serverSocket, buffer, 256);
    //printf("retval is %zu\n", retval);
    //buffer = calloc(1, retval);
    //retval = read_all_from_socket(serverSocket, buffer, retval);

    //retval = read_all_from_socket(serverSocket, buffer, 256);
    //printf("imhere\n");
    write_all_to_socket(local_fd, buffer, retval);
    now_my_size += retval;
    if(retval < 256){
      break;
    }

    // total_size += retval;
    // if(retval < 256){
    //   break;
    // }
  }
  //printf("now_my_size is %zu\n", now_my_size);
  if(total_size < now_my_size){
    print_recieved_too_much_data();
    exit(1);
  }
  if(total_size > now_my_size){
    print_too_little_data();
    exit(1);
  }
  close(serverSocket);
  //fclose(local_fd);
  close(local_fd);
  return;
}

void put_helper(char* local, char* remote){
  //printf("local is: %s\n", local);
  FILE* temp_file = fopen(local, "r");
  int file_under_process = open(local, O_RDWR); //local
  if(!file_under_process){
    printf("%s\n", err_no_such_file);
    exit(1);
  }
  char buffer[1024];
  fseek(temp_file, 0L, SEEK_END);
  size_t file_size = ftell(temp_file);
  fseek(temp_file, 0L, SEEK_SET);
  
  sprintf(buffer, "PUT %s\n", remote);
  write_all_to_socket(serverSocket, buffer, strlen(buffer));
  char second_buffer[1024];
  memcpy(second_buffer, &file_size, sizeof(size_t));
  write_all_to_socket(serverSocket, second_buffer, sizeof(size_t)); 


  char line [256];
  //char *line = NULL;
  int read = -1;

  while (1) {
    //printf("line is: %s\n", line);
    //write(serverSocket, buffer, (int)(read));
    memset(line, 0, 256); 
    //read = read_all_from_socket(file_under_process, line, 256);
    read = fread(line, 1,256, temp_file);
    //printf("line is %s\n", line);
    write_all_to_socket(serverSocket, line, read);
    
    // printf("read is :%zu\n", read);
    // printf("my line is: %s\n", line);
    if(read == -1 || read < 256){
      break;
    }
    // if(current_size + read >= 1024){
    //   memset(second_buffer, 0, 1024);
    //   strcat(second_buffer, line);
    //   write_all_to_socket(serverSocket, second_buffer, (size_t)read); 
    // }else{
    //   strcat(second_buffer, line);
    //   write_all_to_socket(serverSocket, second_buffer, (size_t)read);
    // }
    // //printf("my buffer now is:%s\n", second_buffer);
    // current_size += read;
    // if(current_size >= (ssize_t)file_size){
    //   break;
    // }
  }
  shutdown(serverSocket, SHUT_WR);
  close(serverSocket);
  //free(line);
  fclose(temp_file);
  close(file_under_process);
}

void delete_helper(char* local, char* remote){
  char buffer_upload[1024];
  memset(buffer_upload, 0 ,1024);
  sprintf(buffer_upload, "DELETE %s\n", remote);
  write_all_to_socket(serverSocket, buffer_upload, strlen(buffer_upload));
  shutdown(serverSocket, SHUT_WR);
  ssize_t retval = 1;

  char buffer[256];
  memset(buffer, 0, 256);
  retval = read_all_from_socket(serverSocket, buffer, 256);
  if(buffer[0] == 'O'){
    print_success();
  }else{
    printf("%s", err_no_such_file);
  }
  // while (retval > 0) {
  //   retval = read_all_from_socket(serverSocket, buffer, 256);

  //   write_all_to_socket(1, buffer, retval);
  //   if(retval < 256){
  //     break;
  //   }

  // }

  shutdown(serverSocket, SHUT_WR);
  close(serverSocket);
  return;
}

void list_helper(){
  
  char buffer_upload[1024];
  memset(buffer_upload, 0 ,1024);
  sprintf(buffer_upload, "LIST\n");
  //printf("buffer_upload is:%s\n", buffer_upload);
  write_all_to_socket(serverSocket, buffer_upload, strlen(buffer_upload));
  shutdown(serverSocket, SHUT_WR);
  ssize_t retval = 1;
  //int temp_flag = 0;

  char buffer[256];
  ssize_t total_size = 0;
  memset(buffer, 0, 11);
  retval = read_all_from_socket(serverSocket, buffer, 11);
  memcpy(&total_size, buffer+3, sizeof(size_t));
  //total_size +=1;
  //printf("total size is %zu\n", total_size);
  ssize_t now_my_size = 0;
  while (retval > 0) {
    memset(buffer, 0 , 256);
    retval = read_all_from_socket(serverSocket, buffer, 256);
    //printf("retval is %zu\n", retval);
    //buffer = calloc(1, retval);
    //retval = read_all_from_socket(serverSocket, buffer, retval);

    //retval = read_all_from_socket(serverSocket, buffer, 256);
    //printf("imhere\n");
    write_all_to_socket(1, buffer, retval);
    now_my_size += retval;
    if(retval < 256){
      break;
    }

    // total_size += retval;
    // if(retval < 256){
    //   break;
    // }
  }
  //printf("now_my_size is %zu\n", now_my_size);
  if(total_size < now_my_size){
    print_recieved_too_much_data();
    exit(1);
  }

  if(total_size > now_my_size){
    print_too_little_data();
    exit(1);
  }

  shutdown(serverSocket, SHUT_RD);
  close(serverSocket);
  return;
  //free(line);
}

int main(int argc, char **argv) {
  // Good luck!
  char** parsed_commands = parse_args(argc, argv);
  verb action = check_args(parsed_commands);
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(serverSocket == -1){
      printf("socket error\n");
      exit(1);
  }
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int getaddr_status = getaddrinfo(parsed_commands[0], parsed_commands[1], &hints, &result);
  if(getaddr_status != 0){
      printf("getaddrinfo error\n");
      exit(1);
  }
  int connect_status = connect(serverSocket, result->ai_addr, result->ai_addrlen);
  if(connect_status != 0){
      printf("connect error\n");
      exit(1);
  }
  //===============================================connect to server==================================
  if(action == PUT){
    put_helper(parsed_commands[4], parsed_commands[3]);
  }
  if(action == GET){
    get_helper(parsed_commands[4], parsed_commands[3]);
  }
  if(action == DELETE){
    delete_helper(parsed_commands[4], parsed_commands[3]);
  }
  if(action == LIST){
    list_helper();
  }




  free(parsed_commands);
  free(result);
  return 0;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
  if (argc < 3) {
    return NULL;
  }

  char *host = strtok(argv[1], ":");
  char *port = strtok(NULL, ":");
  if (port == NULL) {
    return NULL;
  }

  char **args = calloc(1, (argc + 1) * sizeof(char *));
  args[0] = host;
  args[1] = port;
  args[2] = argv[2];
  char *temp = args[2];
  while (*temp) {
    *temp = toupper((unsigned char)*temp);
    temp++;
  }
  if (argc > 3) {
    args[3] = argv[3];
  } else {
    args[3] = NULL;
  }
  if (argc > 4) {
    args[4] = argv[4];
  } else {
    args[4] = NULL;
  }

  return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
  if (args == NULL) {
    print_client_usage();
    exit(1);
  }

  char *command = args[2];

  if (strcmp(command, "LIST") == 0) {
    return LIST;
  }

  if (strcmp(command, "GET") == 0) {
    if (args[3] != NULL && args[4] != NULL) {
      return GET;
    }
    print_client_help();
    exit(1);
  }

  if (strcmp(command, "DELETE") == 0) {
    if (args[3] != NULL) {
      return DELETE;
    }
    print_client_help();
    exit(1);
  }

  if (strcmp(command, "PUT") == 0) {
    if (args[3] == NULL || args[4] == NULL) {
      print_client_help();
      exit(1);
    }
    return PUT;
  }

  // Not a valid Method
  print_client_help();
  exit(1);
}
