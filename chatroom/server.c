/**
 * Chatroom Lab
 * CS 241 - Spring 2017
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);

static volatile int endSession;
static volatile int sock_fd;
static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    endSession = 1;
    // add any additional flags here you want.
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    // Your code here.
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients[i] != -1){
            shutdown(clients[i], SHUT_RDWR);
            close(clients[i]);
        }
    }
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprintf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) { // reference: Fall 2016 CS 241 Angrave Lecture Code
    /*QUESTION 1*/
    /*QUESTION 2*/
    /*QUESTION 3*/

    /*QUESTION 8*/

    /*QUESTION 4*/
    /*QUESTION 5*/
    /*QUESTION 6*/

    /*QUESTION 9*/

    /*QUESTION 10*/
    endSession = 0;
    int s;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        fprintf(stderr, "socket error\n");
        exit(1);
    }
    int optval = 1;
    setsockopt(sock_fd,SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype =SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, port, &hints, &result);
    if(s != 0){
        fprintf(stderr, "getaddrinfo error\n");
        exit(1);
    }

    if(bind(sock_fd, result->ai_addr, result->ai_addrlen)!=0){
        perror("bind()");
        exit(1);
    }
    if(listen(sock_fd, MAX_CLIENTS) != 0){
        perror("listen()");
        exit(1);
    }

    for(int i=0; i<MAX_CLIENTS; i++){
        clients[i] = -1; // initializing
    }

    while(!endSession){
        if(clientsCount < MAX_CLIENTS){
            struct sockaddr_in *result_addr = (struct sockaddr_in*)result->ai_addr;
            printf("Listening on the file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port) );
        
            printf("Waiting for connections...\n");
            struct sockaddr clientinfo;
            socklen_t clientlen = sizeof(struct sockaddr);
            memset(&clientinfo, 0, clientlen);
            int client_fd = accept(sock_fd, &clientinfo, &clientlen);
            if(client_fd == -1){
                continue;
            }
            printf("Connection model client_fd = %d\n", client_fd);
            // char buffer[1000];
            // int len = read(client_fd, buffer, 999);
            // printf("Read %d chars\n", len);
            // if(len > 0){
            //     buffer[len] = '\0';
            //     printf("%s\n", buffer);
            // }

            intptr_t retVal = -1;
            pthread_mutex_lock(&mutex);
            for(int i=0; i<MAX_CLIENTS; i++){
                if(clients[i] == -1){
                    clients[i] = client_fd;
                    char buffer[1000];
                    inet_ntop(AF_INET, &clientinfo, buffer, clientlen);
                    retVal = i;
                    break;
                }
                
            }
            clientsCount++;
            //printf("current clientsCount is: %d\n", clientsCount);
            pthread_mutex_unlock(&mutex);

            pthread_t new_thread;
            if(pthread_create(&new_thread, NULL, process_client, (void*)retVal)){
                //fprintf(stderr, "pthread_create failed\n");
                exit(1);
            }

        }


    }
    //free(&hints);
    freeaddrinfo(result);

    return;
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "./server <port>\n");
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    // signal(SIGINT, close_server);
    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}
