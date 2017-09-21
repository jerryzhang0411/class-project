/**
 * Networking
 * CS 241 - Spring 2017
 */
#include "common.h"
#include "format.h"
#include "vector.h"
#include "dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

struct packet_info{
	int parse_status;
	int header_size;
	char* action;
	char* header_buffer;
	int header_buffer_offset;
	char* filename;
	char* filename_buffer;
	int filename_offset;
	size_t filesize;
	size_t get_offset;
	int first_get;
	size_t put_offset;
	int first_put;
	int need_to_reparse_filesize;
	char* reparse_buffer;
	size_t put_temp_offset;
	int flag_reparse;
	int flag_rewrite;
};

vector *to_do_vector = NULL;
dictionary *my_dictionary = NULL;
static volatile int endSession;
static volatile int sock_fd;
static volatile int epoll_fd;
char* new_dir = NULL; // changing dir to this

//static const size_t MESSAGE_SIZE_DIGITS = 4;


void rmv_file_and_dir(){
	for(int i=0; i<(int)vector_size(to_do_vector); i++){
		remove(*vector_at(to_do_vector, i));
	}
	chdir("../");
	remove(new_dir);
}

void close_server() {
    endSession = 1;
    dictionary_destroy(my_dictionary);
    rmv_file_and_dir();
    vector_destroy(to_do_vector);
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
}

// ssize_t read(int socket, char *buffer, size_t count) { //reference: lab chatroom
//     // Your Code Here
//     size_t total_read = 0;
//     while(total_read < count){
//         ssize_t buffer_count = read(socket, buffer+total_read, (ssize_t)(count - total_read));
//         if(buffer_count > 0){
//             total_read+=buffer_count;
//         }else if(!buffer_count){
//             return total_read;
//         }else if(buffer_count == -1 && errno != EINTR){
//             return -1;
//         }else if(buffer_count == -1 && errno == EINTR){
//             continue;
//         }
//     }
//     return total_read;
// }

// ssize_t write(int socket, const char *buffer, size_t count) { //reference: lab chatroom
//     // Your Code Here
//     size_t total_read = 0;
//     while(total_read < count){
//         ssize_t buffer_count = write(socket, buffer+total_read, (ssize_t)(count - total_read));
//         if(buffer_count > 0){
//             total_read+=buffer_count;
//         }else if(!buffer_count){
//             return total_read;
//         }else if(buffer_count == -1 && errno != EINTR){
//             return -1;
//         }else if(buffer_count == -1 && errno == EINTR){
//             continue;
//         }
//     }
//     return total_read;
// }


// ssize_t get_message_size(int socket) { //reference: lab chatroom
//     int32_t size;
//     ssize_t read_bytes =
//         read(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
//     if (read_bytes == 0 || read_bytes == -1)
//         return read_bytes;

//     return (ssize_t)ntohl(size);
// }

void accept_connections(struct epoll_event *e,int epoll_fd) //Reference: Spring17 CS241 Lecture Code #32
{
	while(1)
	{
		struct sockaddr_in new_addr;
		socklen_t new_len = sizeof(new_addr);
		int new_fd = accept(e->data.fd, (struct sockaddr*) &new_addr, &new_len);

		if(new_fd == -1)
		{
			// All pending connections handled
			if(errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				perror("accept");
				exit(1);
			}
		}
		
		char *connected_ip= inet_ntoa(new_addr.sin_addr);
		int port = ntohs(new_addr.sin_port);
        printf("Accepted Connection %s port %d\n", connected_ip, port);
        //printf("my event id is %d\n", e->data.fd);
        int flags = fcntl(new_fd, F_GETFL, 0);
        fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
        
        //Connection to epoll
        struct epoll_event event;
        event.data.fd = new_fd;
        event.events = EPOLLIN | EPOLLET;
        if(epoll_ctl (epoll_fd, EPOLL_CTL_ADD, new_fd, &event) == -1)
        {
        	perror("accept epoll_ctl");
        	exit(1);
        }
        //printf("event finished: %d\n", e->data.fd);
	}
}


void put_parse(struct epoll_event *e){
	char my_buffer[1024];
	char action_result[1024];
	char filename_result[1024];
	memset(my_buffer, 0, 1024);
	memset(action_result, 0, 1024);
	memset(filename_result, 0, 1024);
	//================================================getting action=======================================
	int count = read(e->data.fd, my_buffer, 1);
	if(count == -1)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
		{
			printf("146: EAGAIN or EWOULDBLOCK, returning!\n");
			//close(e->data.fd);

			return;
		}
		perror("151: file error!\n");
		close(e->data.fd);

		return;
	}
	errno = 0;
	struct packet_info* temp = malloc(sizeof(struct packet_info));
	temp->parse_status = 0;
	temp->get_offset = 0;
	temp->put_offset = 0;
	temp->first_get = 1;
	temp->first_put = 1;
	temp->need_to_reparse_filesize = 0;
	temp->flag_reparse = 0;
	temp->flag_rewrite = 0;
	dictionary_set(my_dictionary, (void*)&(e->data.fd), temp);
	while(strcmp(my_buffer, " ")!=0){

		if(strlen(action_result) >= 1024){
			char return_buffer[100] = "ERROR:\0";
			strcat(return_buffer, err_bad_request);
			write(e->data.fd, return_buffer, strlen(return_buffer));
			return;
		}
		strcat(action_result, my_buffer);
		memset(my_buffer, 0, 1024);
		int count = read(e->data.fd, my_buffer, 1);
		if(count == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
			{
				printf("171: EAGAIN or EWOULDBLOCK, returning!\n");
				//close(e->data.fd);

				return;
			}
			perror("176: file error!\n");
			close(e->data.fd);

			return;
		}
		if(strcmp(action_result, "LIST") == 0){
			temp->action = action_result;
			temp->parse_status = 1;
			dictionary_set(my_dictionary, (void*)&(e->data.fd), temp);
			return;
		}
	}
	errno = 0;
	temp->action = action_result;
	
//=======================================================end================================================
//======================================================getting filename=================================================
	//read(e->data.fd, NULL, 1); //get rid of space
	memset(my_buffer, 0, 1024);
	count = read(e->data.fd, my_buffer, 1);
	if(count == -1)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
		{
			printf("193: EAGAIN or EWOULDBLOCK, returning!\n");
			//close(e->data.fd);

			return;
		}
		perror("198: file error!\n");
		close(e->data.fd);

		return;
	}
	errno = 0;
	while(strcmp(my_buffer, "\n")!=0){
		if(strlen(filename_result) >= 1024){
			char return_buffer[100] = "ERROR:\0";
			strcat(return_buffer, err_bad_request);
			write(e->data.fd, return_buffer, strlen(return_buffer));
			return;
		}
		strcat(filename_result, my_buffer);
		memset(my_buffer, 0, 1024);
		count = read(e->data.fd, my_buffer, 1);
		if(count == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
			{
				printf("226: EAGAIN or EWOULDBLOCK, returning!\n");
				//close(e->data.fd);

				return;
			}else{
				perror("231: file error!\n");
				close(e->data.fd);

				return;
			}
			
		}

	}
	errno = 0;
	temp->filename = filename_result;


	//=================================================end===================================================

	//==============================================setting header size========================================
	temp->header_size = strlen(temp->action) + 1 + strlen(temp->filename) + 1 + sizeof(size_t);
	//====================================================end===============================================
	if(strcmp(action_result, "GET") == 0 || strcmp(action_result, "DELETE") == 0){
		//printf("filesize avoided\n");
		temp->parse_status = 1;
		dictionary_set(my_dictionary, (void*)&(e->data.fd), temp);
		return;
	}
	//===============================================setting file size=======================================
	size_t filesize_input = 0;
	//read(e->data.fd, NULL, sizeof("\n")); // get rid of new line
	count = read(e->data.fd, (char*)&filesize_input, sizeof(size_t));

	printf("289: my filesize_input is %zu\n", filesize_input);
	if(count == -1)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
		{
			// char debug[100];
			// memset(debug, 0, 100);
			// count = read(e->data.fd, debug, 100);
			// printf("my debug is %s\n", debug);
			temp->need_to_reparse_filesize = 1;
			temp->parse_status = 11037; // reparse for size
			dictionary_set(my_dictionary, (void*)&(e->data.fd), temp);
			printf("254: EAGAIN or EWOULDBLOCK, returning!\n");
			//close(e->data.fd);

			return;
		}else{
			perror("259: file error!\n");
			close(e->data.fd);
			return;
		}
	}
	errno = 0;
	temp->filesize = filesize_input;
	//printf("my filesize now is: %zu\n", temp->filesize);
	//=========================================================end====================================

	temp->parse_status = 1;
	dictionary_set(my_dictionary, (void*)&(e->data.fd), temp);
	return;
}

void reparse(struct epoll_event *e){
	//printf("imhere\n");
	struct packet_info* temp_struct = dictionary_get(my_dictionary, (void*)&(e->data.fd));
	size_t filesize_input = 0;
	size_t count = read(e->data.fd, (char*)&filesize_input, sizeof(size_t));
	if((int)count == -1)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
		{
			printf("329: EAGAIN or EWOULDBLOCK, returning!\n");
			//close(e->data.fd);
			return;
		}else{
			perror("333: file error!\n");
			close(e->data.fd);
			return;
		}
	}
	// printf("after reparse, my action is %s\n", temp_struct->action);
	// printf("after reparse, my filename is %s\n", temp_struct->filename);
	printf("after reparse, my filesize_input is %zu\n", filesize_input);
	temp_struct->parse_status = 1;
	temp_struct->filesize = filesize_input;
	dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
}

void reget(struct epoll_event *e){
	struct packet_info* temp_struct = dictionary_get(my_dictionary, (void*)&(e->data.fd));
	FILE* local_fd = fopen(temp_struct->filename, "r");
	if(!local_fd){ // no such file
		char return_buffer[100] = "ERROR:\n";
		strcat(return_buffer, err_no_such_file);
		//printf("ERROR: cannot open, writing to client: %s\n", return_buffer);
		write(e->data.fd, return_buffer, strlen(return_buffer));
		close(e->data.fd);
		return;
	}
	
	fseek(local_fd, 0, SEEK_END);
	size_t tempHold_filesize = ftell(local_fd);
	fseek(local_fd, 0, SEEK_SET);
	//printf("temp_struct filesize is %zu\n", tempHold_filesize);
	temp_struct->filesize = tempHold_filesize;
	size_t remaining_size = temp_struct->filesize;
	if(temp_struct -> first_get){
		write(e->data.fd, "OK\n", 3);
		write(e->data.fd, (char*)&(temp_struct->filesize), sizeof(size_t));
		temp_struct -> first_get = 0;
	}
	
	char temp_buffer_to_write[1024];
	while(1){
		memset(temp_buffer_to_write, 0, 1024);
		fseek(local_fd, temp_struct->get_offset, SEEK_SET);
		remaining_size = temp_struct->filesize - temp_struct->get_offset;
		
		if(remaining_size >= 1024){
			size_t temp = fread(temp_buffer_to_write, 1, 1024, local_fd);
			// temp_struct->get_offset += temp;
			ssize_t count = write(e->data.fd, temp_buffer_to_write, temp);
			if(count == -1){
				if(errno == EAGAIN || errno == EWOULDBLOCK){
					temp_struct->flag_rewrite = 1;
					dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, NULL);  
					printf("390: EAGAIN or EWOULDBLOCK, returning!\n");
					return;
				}else{
					perror("393: file error!\n");
					close(e->data.fd);
					fclose(local_fd);
					return;
				}	
			}else if(count == 0){
				printf("399: done reading!\n");
				temp_struct->flag_rewrite = 0;
				dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
				close(e->data.fd);
				fclose(local_fd);
				shutdown(e->data.fd, SHUT_WR);
				return;
			}else{
				temp_struct->get_offset += count;
				temp_struct->flag_rewrite = 1;
				dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
				//printf("411: continueing\n");
				continue;
			}
		}else{
			size_t temp = fread(temp_buffer_to_write, 1, remaining_size, local_fd);
			ssize_t another_temp = write(e->data.fd, temp_buffer_to_write, temp);
			if(another_temp == -1){
				temp_struct->flag_rewrite = 1;
				dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
				return;
			}else if(another_temp == 0){
				printf("420: done reading!\n");
				temp_struct->flag_rewrite = 0;
				dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
				close(e->data.fd);
				fclose(local_fd);
				shutdown(e->data.fd, SHUT_WR);
				return;
			}else{
				temp_struct->get_offset += another_temp;
				temp_struct->flag_rewrite = 1;
				dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
				//printf("431: continueing\n");
				continue;
			}
		}
	}
	//printf("after 390 return\n");
    return;
}

void reput(struct epoll_event *e){
	struct packet_info* temp_struct = dictionary_get(my_dictionary, (void*)&(e->data.fd));
	errno = 0;
	int local_fd;
	if(temp_struct->first_put){
		vector_push_back(to_do_vector, temp_struct->filename);
		local_fd = open(temp_struct->filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
		temp_struct->first_put = 0;
	}else{
		local_fd = open(temp_struct->filename, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
	}
	

	ssize_t count;
	ssize_t now_my_size = temp_struct->put_temp_offset;
	//printf("now my size get! it is: %zu\n", now_my_size);
	while(1){
		char remaining_buffer[1024];
		memset(remaining_buffer, 0, 1024);
		count = read(e->data.fd, remaining_buffer, 1024);
		//printf("in reput, my count read is %zu\n", count);
		if(count == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
			{
				temp_struct-> flag_reparse = 1;
				temp_struct->put_temp_offset = now_my_size;
				//printf("before setting, now_my_size is :%zu\n", now_my_size);
				dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
				printf("290: EAGAIN or EWOULDBLOCK, returning!\n");
				//close(e->data.fd);
				close(local_fd);
				return;
			}else{
				perror("295: file error!\n");
				close(e->data.fd);
				close(local_fd);
				return;
			}
		}else if(count == 0){
			break;
		}
		errno = 0;
		write(local_fd, remaining_buffer, count);
		now_my_size += count;
		// if(count < 1024){
		// 	break;
		// }
	}

	close(local_fd);
	//printf("now in reput my now_my_size is: %zu\n", now_my_size);
	if((ssize_t)(temp_struct->filesize) > now_my_size){
	    print_too_little_data();
	    remove(temp_struct->filename);
		char return_buffer[100] = "ERROR:\0";
		strcat(return_buffer, err_bad_file_size);
		write(e->data.fd, return_buffer, strlen(return_buffer));
		close(e->data.fd);
		return;
		
	}

	if((ssize_t)(temp_struct->filesize) < now_my_size){
	    print_recieved_too_much_data();
	    remove(temp_struct->filename);
		char return_buffer[100] = "ERROR:\0";
		strcat(return_buffer, err_bad_file_size);
		write(e->data.fd, return_buffer, strlen(return_buffer));
		close(e->data.fd);
		return;
	}
	char return_buffer[100] = "OK\n";
	printf("415: OK sent!\n");
	//printf("strlen return_buffer is %zu\n", strlen(return_buffer));
	temp_struct-> flag_reparse = 0;
	shutdown(e->data.fd, SHUT_RD);
	write(e->data.fd, return_buffer, 3);
	shutdown(e->data.fd, SHUT_WR);
	close(e->data.fd);
	//printf("my vector has size: %zu\n", vector_size(to_do_vector));
	return;
}

int flag = 0; // flag for rescanning the size

void handle_data(struct epoll_event *e) //Reference: Spring17 CS241 Lecture Code #32
{
	
	//while(1) {
	if(dictionary_contains(my_dictionary, (void*)&(e->data.fd))){

		struct packet_info* temp_struct = dictionary_get(my_dictionary, (void*)&(e->data.fd));
		printf("entering! writeflag is %d\n", temp_struct-> flag_rewrite);
		if(temp_struct-> flag_reparse == 1){
			//printf("imhere!\n");
			reput(e);
			return;
		}
		if(temp_struct-> flag_rewrite == 1){
			//printf("imhere!!\n");
			reget(e);
			return;
		}
	}else{
		printf("does not contain!\n");
	}
	
	if(flag == 1){
		if(dictionary_contains(my_dictionary, (void*)&(e->data.fd))){
			if(((struct packet_info*)dictionary_get(my_dictionary, (void*)&(e->data.fd)))->parse_status == 11037){
				reparse(e); // if success, status ==1
			}
			if(((struct packet_info*)dictionary_get(my_dictionary, (void*)&(e->data.fd)))->parse_status != 1){
				return;
			}
		}
	}
	
	if(flag == 0){
		put_parse(e);
		if(((struct packet_info*)dictionary_get(my_dictionary, (void*)&(e->data.fd)))->parse_status != 1){
			flag = 1;
			return;
		}
	}

	flag = 0;
		struct packet_info* temp_struct = dictionary_get(my_dictionary, (void*)&(e->data.fd));

		// while(temp_struct->parse_status == 1103711037){
		// 	printf("imhere\n");
		// 	reput(e);
		// }
		//printf("now my size is %zu\n", temp_struct->filesize);
		//printf("now my action is: %s\n", temp_struct->action);
		if (strcmp(temp_struct->action, "LIST") == 0) {
			//printf("imhere\n");
			write(e->data.fd, "OK\n", 3);
			printf("299: OK sent!\n");
			size_t my_size = 0;
			for(int i=0; i<(int)vector_size(to_do_vector); i++){
				my_size += (strlen(*vector_at(to_do_vector, i)) +1);
			}
			write(e->data.fd, (char*)&my_size, sizeof(size_t));
			//printf("my_size is %zu\n", my_size);
		    for(int i=0; i<(int)vector_size(to_do_vector); i++){
		    	write(e->data.fd, *vector_at(to_do_vector, i), strlen(*vector_at(to_do_vector, i)));
		    	write(e->data.fd, "\n", 1);
		    }
		    shutdown(e->data.fd, SHUT_RDWR);
		    close(e->data.fd);
		    return;
		}
		else if (strcmp(temp_struct->action, "GET") == 0) {
			//printf("the filename is: %s\n", temp_struct->filename);
			FILE* local_fd = fopen(temp_struct->filename, "r");
			if(!local_fd){ // no such file
				char return_buffer[100] = "ERROR:\n";
				strcat(return_buffer, err_no_such_file);
				//printf("ERROR: cannot open, writing to client: %s\n", return_buffer);
				write(e->data.fd, return_buffer, strlen(return_buffer));
				close(e->data.fd);
				return;
			}
			
			fseek(local_fd, 0, SEEK_END);
			size_t tempHold_filesize = ftell(local_fd);
			fseek(local_fd, 0, SEEK_SET);
			//printf("temp_struct filesize is %zu\n", tempHold_filesize);
			temp_struct->filesize = tempHold_filesize;
			size_t remaining_size = temp_struct->filesize;
			if(temp_struct -> first_get){
				write(e->data.fd, "OK\n", 3);
				write(e->data.fd, (char*)&(temp_struct->filesize), sizeof(size_t));
				temp_struct -> first_get = 0;
			}
			
			char temp_buffer_to_write[1024];
			while(1){
				memset(temp_buffer_to_write, 0, 1024);
				fseek(local_fd, temp_struct->get_offset, SEEK_SET);
				remaining_size = temp_struct->filesize - temp_struct->get_offset;
				
				if(remaining_size >= 1024){
					size_t temp = fread(temp_buffer_to_write, 1, 1024, local_fd);
					
					ssize_t count = write(e->data.fd, temp_buffer_to_write, temp);

					if(count == -1){
						if(errno == EAGAIN || errno == EWOULDBLOCK){
							temp_struct->flag_rewrite = 1;
							dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
							epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, NULL);  
							printf("539: EAGAIN or EWOULDBLOCK, returning!\n");
							return;
						}else{
							perror("545: file error!\n");
							close(e->data.fd);
							fclose(local_fd);
							return;
						}	
					}else if(count == 0){
						printf("549: done reading!\n");
						temp_struct->flag_rewrite = 0;
						dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
						close(e->data.fd);
						fclose(local_fd);
						shutdown(e->data.fd, SHUT_WR);
						return;
					}else{
						temp_struct->get_offset += count;
						temp_struct->flag_rewrite = 1;
						dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
						//printf("556: continueing\n");
						continue;
					}
				}else{
					size_t temp = fread(temp_buffer_to_write, 1, remaining_size, local_fd);
					ssize_t another_temp = write(e->data.fd, temp_buffer_to_write, temp);
					if(another_temp == -1){
						temp_struct->flag_rewrite = 1;
						dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
						return;
					}else if(another_temp == 0){
						printf("654: done reading!\n");
						temp_struct->flag_rewrite = 0;
						dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
						close(e->data.fd);
						fclose(local_fd);
						shutdown(e->data.fd, SHUT_WR);
						return;
					}else{
						temp_struct->get_offset += another_temp;
						temp_struct->flag_rewrite = 1;
						dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
						//printf("663: continueing\n");
						continue;
					}
				}
			}
			//printf("after 539 return\n");
		    return;
		}
		else if (strcmp(temp_struct->action, "PUT") == 0) {
			errno = 0;
			int local_fd;
			if(temp_struct->first_put){
				for(int i=0; i<(int)vector_size(to_do_vector); i++){
					if(strcmp(temp_struct->filename, *vector_at(to_do_vector, i)) == 0){
						vector_erase(to_do_vector, i);
					}
				}
				vector_push_back(to_do_vector, temp_struct->filename);
				local_fd = open(temp_struct->filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
				temp_struct->first_put = 0;
			}else{
				local_fd = open(temp_struct->filename, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
			}
			
	
			ssize_t count;
			ssize_t now_my_size = 0;
			while(1){
				char remaining_buffer[1024];
				memset(remaining_buffer, 0, 1024);
				count = read(e->data.fd, remaining_buffer, 1024);
				if(count == -1)
				{
					if(errno == EAGAIN || errno == EWOULDBLOCK) //error exit
					{
						temp_struct-> flag_reparse = 1; // reput
						temp_struct->put_temp_offset = now_my_size;
						//printf("before setting, now_my_size is :%zu\n", now_my_size);
						dictionary_set(my_dictionary, (void*)&(e->data.fd), temp_struct);
						printf("540: EAGAIN or EWOULDBLOCK, returning!\n");
						//close(e->data.fd);
						close(local_fd);
						return;
					}else{
						perror("545: file error!\n");
						close(e->data.fd);
						close(local_fd);
						return;
					}
				}else if(count == 0){
					break;
				}
				errno = 0;
				write(local_fd, remaining_buffer, count);
				now_my_size += count;
				// if(count < 1024){
				// 	break;
				// }
			}

			close(local_fd);
			if((ssize_t)(temp_struct->filesize) > now_my_size){
			    print_too_little_data();
			    remove(temp_struct->filename);
				char return_buffer[100] = "ERROR:\0";
				strcat(return_buffer, err_bad_file_size);
				write(e->data.fd, return_buffer, strlen(return_buffer));
				close(e->data.fd);
				return;
				
			}

			if((ssize_t)(temp_struct->filesize) < now_my_size){
			    print_recieved_too_much_data();
			    remove(temp_struct->filename);
				char return_buffer[100] = "ERROR:\0";
				strcat(return_buffer, err_bad_file_size);
				write(e->data.fd, return_buffer, strlen(return_buffer));
				close(e->data.fd);
				return;
			}
			char return_buffer[100] = "OK\n";
			printf("761: OK sent!\n");
			//printf("strlen return_buffer is %zu\n", strlen(return_buffer));
			temp_struct-> flag_reparse = 0;
			shutdown(e->data.fd, SHUT_RD);
			write(e->data.fd, return_buffer, 3);
			shutdown(e->data.fd, SHUT_WR);
			close(e->data.fd);
			//printf("my vector has size: %zu\n", vector_size(to_do_vector));
			return;
		}
		else if (strcmp(temp_struct->action, "DELETE") == 0) {
			for(int i=0; i<(int)vector_size(to_do_vector); i++){
				//printf("temp_struct->filename is %s\n", temp_struct->filename);
				//printf("*vector_at(to_do_vector, i) is %s\n", *vector_at(to_do_vector, i));
				if(strcmp(temp_struct->filename, *vector_at(to_do_vector, i)) == 0){
					vector_erase(to_do_vector, i);
					write(e->data.fd, "OK\n", 3);
					shutdown(e->data.fd, SHUT_WR);
					close(e->data.fd);
					return;
				}
			}
			shutdown(e->data.fd, SHUT_WR);
			close(e->data.fd);
			return;
		}else{
			char return_buffer[100] = "ERROR:\0";
			strcat(return_buffer, err_bad_request);
			write(e->data.fd, return_buffer, strlen(return_buffer));
			close(e->data.fd);
			exit(1);
		}
		
		
		shutdown(e->data.fd, SHUT_WR);
		rmv_file_and_dir();
		close(e->data.fd);
		//break;
	//}
		//return;
}

int main(int argc, char **argv) { //Reference: Spring17 CS241 Lecture Code #32, lab chatroom
  // Good luck!
	if(argc != 2){
		print_error_message("./server <port>");
		exit(1);
	}
	struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }
	my_dictionary = dictionary_create(shallow_hash_function, int_compare, int_copy_constructor, int_destructor, NULL, NULL);
	to_do_vector = vector_create(NULL, NULL, NULL);

	char template[32];
	memset(template, 0, 32);
	strcat(template, "storage");
	strcat(template, "XXXXXX");
	new_dir = mkdtemp(template);
	print_temp_directory(new_dir);
	chdir(new_dir); // working space stuff

	int s;
	// Create the socket as a nonblocking socket
    sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	s = getaddrinfo(NULL, argv[1], &hints, &result);
	if(s!=0){
		printf("getaddrinfo:%s\n", gai_strerror(s));
		exit(1);
	}

	int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); // set port reuseable

	if(bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0){
		print_error_message("bind() error");
		exit(1);
	}

	if(listen(sock_fd, 1024) != 0){ 
		print_error_message("listen() error");
		exit(1);
	}

	struct sockaddr_in sin;
    socklen_t socklen = sizeof(sin);
    if (getsockname(sock_fd, (struct sockaddr *)&sin, &socklen) == -1)
      perror("getsockname");
    else
      printf("Listening on port number %d\n", ntohs(sin.sin_port));  

    //setup epoll
	epoll_fd = epoll_create(1);
	if(epoll_fd == -1)
    {
        perror("epoll_create()");
        exit(1);
    }

    struct epoll_event event;
	event.data.fd = sock_fd;
	event.events = EPOLLIN | EPOLLET;

	//Add the sever socket to the epoll
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event))
	{
        perror("epoll_ctl()");
        exit(1);
	}
	
	// Event loop
	freeaddrinfo(result);
	endSession = 0;
	while(!endSession) {
		struct epoll_event new_event;
		memset(&new_event, 0, sizeof(new_event));
		if(epoll_wait(epoll_fd, &new_event, 1, -1) > 0)
		{
			//Probably check for errors
			
			// New Connection Ready
			if(sock_fd == new_event.data.fd)
				accept_connections(&new_event, epoll_fd);
			else
				handle_data(&new_event);	
		}
	}
	
    return 0;
}