/**
 * Splendid Synchronization Lab 
 * CS 241 - Spring 2017
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"
queue * myqueue = NULL; 


void my_int_destructor(void *elem) { free(elem); }

void *my_int_constructor(void *elem) { 

	void * new = calloc(1, sizeof(int));
	*((int*)new) = *((int *)elem); 
	return new; 
}

// void *long_copy_constructor(void *elem) {
//   long *copy = malloc(sizeof(long));
//   *copy = *((long *)elem);
//   return copy;
// }

void* modify() {
  
  	int a = (int)queue_pull(myqueue);
	printf("%d\n", a);
	a = (int)queue_pull(myqueue);
	printf("%d\n", a);

	return NULL; 
}



int main(int argc, char **argv) {

	int my_data1 = 1; 
	int my_data2 = 2; 
	int my_data3 = 3; 
	int my_data4 = 4; 
	int my_data5 = 5; 
	int my_data6 = 6; 
	int my_data7 = 7; 


	myqueue = queue_create(5, my_int_constructor, my_int_destructor);
	queue_push(myqueue, &my_data1); 
	queue_push(myqueue, &my_data2); 
	queue_push(myqueue, &my_data3); 
	queue_push(myqueue, &my_data4); 
	queue_push(myqueue, &my_data5); 
	
	pthread_t tid1; 
	pthread_create(&tid1, NULL, modify, NULL);
	pthread_join(tid1, NULL);

	queue_push(myqueue, &my_data6); 
	queue_push(myqueue, &my_data7); 

	int a = (int)queue_pull(myqueue);
	printf("%d\n", a);
	a = (int)queue_pull(myqueue);
	printf("%d\n", a);
	a = (int)queue_pull(myqueue);
	printf("%d\n", a);
	a = (int)queue_pull(myqueue);
	printf("%d\n", a);
	a = (int)queue_pull(myqueue);
	printf("%d\n", a);
	//a = (int)queue_pull(myqueue);
	//printf("%d\n", a);
	// queue_push(myqueue, &my_data2); 

	queue_destroy(myqueue); 

  	return 0;
}
