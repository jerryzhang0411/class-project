/**
 * Chatroom Lab
 * CS 241 - Spring 2017
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    uint32_t usable_size = htonl(size);
    char* input = (char*)&usable_size;
    return write_all_to_socket(socket, input, MESSAGE_SIZE_DIGITS);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t total_read = 0;
    while(total_read < count){
        ssize_t buffer_count = read(socket, buffer+total_read, (ssize_t)(count - total_read));
        if(buffer_count > 0){
            total_read+=buffer_count;
        }else if(!buffer_count){
            return 0;
        }else if(buffer_count == -1 && errno != EINTR){
            return -1;
        }else if(buffer_count == -1 && errno == EINTR){
            continue;
        }
    }
    return total_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t total_read = 0;
    while(total_read < count){
        ssize_t buffer_count = write(socket, buffer+total_read, (ssize_t)(count - total_read));
        if(buffer_count > 0){
            total_read+=buffer_count;
        }else if(!buffer_count){
            return 0;
        }else if(buffer_count == -1 && errno != EINTR){
            return -1;
        }else if(buffer_count == -1 && errno == EINTR){
            continue;
        }
    }
    return total_read;
}