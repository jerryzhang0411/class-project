/**
 * Splendid Synchronization Lab 
 * CS 241 - Spring 2017
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_node's.
 */
typedef struct queue_node {
  void *data;
  struct queue_node *next;
} queue_node;

struct queue {
  /* The function callback for the user to define the way they want to copy
   * elements */
  copy_constructor_type copy_constructor;

  /* The function callback for the user to define the way they want to destroy
   * elements */
  destructor_type destructor;

  /* queue_node pointers to the head and tail of the queue */
  queue_node *head, *tail;

  /* The number of elements in the queue */
  ssize_t size;

  /**
   * The maximum number of elements the queue can hold.
   * max_size is non-positive if the queue does not have a max size.
   */
  ssize_t max_size;

  /* Mutex and Condition Variable for thread-safety */
  pthread_cond_t cv;
  pthread_mutex_t m;
};

#define DATA(p) (p->data)

queue *queue_create(ssize_t max_size, copy_constructor_type copy_constructor,
                    destructor_type destructor) {
  /* Your code here */
  queue* retVal = malloc(sizeof(queue));
  retVal->max_size = max_size;
  retVal->size = 0;
  retVal->copy_constructor = copy_constructor;
  retVal->destructor = destructor;
  retVal->head = NULL;
  retVal->tail = NULL;
  pthread_mutex_init(&(retVal->m), NULL);
  pthread_cond_init(&(retVal->cv), NULL);
  return retVal;
}

void queue_destroy(queue *this) {
  while(this->head){
    queue_node* tempHold = this->head->next;
    this->destructor(this->head);
    this->head = tempHold;
  }
  pthread_mutex_destroy(&(this->m));
  pthread_cond_destroy(&(this->cv));
  free(this);
}

void queue_push(queue *this, void *data) {
  pthread_mutex_lock(&(this->m));
  
  while(this->max_size>0 && this->size >= this->max_size) {
    pthread_cond_wait(&(this->cv),&(this->m)); 
  }
  queue_node* tempHold = malloc(sizeof(queue_node));
  tempHold->data = data;
  tempHold->next = NULL;
  this->size++; 
  if(this->tail != NULL){
    this->tail->next = tempHold;
    this->tail = tempHold;
  }else{
    this->head = tempHold;
    this->tail = tempHold;
  }
  pthread_cond_broadcast(&(this->cv));
  pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
  /* Your code here */
  pthread_mutex_lock(&(this->m));
  while(this->size <=0) {
    pthread_cond_wait(&(this->cv),&(this->m)); 
  }
  
  this->size--; 
  queue_node* tempHold = this->head;
  if(this->tail != this->head){
    this->head = tempHold->next;
  }else{
    this->head = NULL;
    this->tail = NULL;
  }
    /*
  if(tmep == barrier->times_used){
    barrier->time_used++;
  }
  */
  void *retVal = DATA(tempHold);
  free(tempHold);
  pthread_cond_broadcast(&(this->cv));
  pthread_mutex_unlock(&(this->m));
  return retVal;
}


// The following is code generated:

queue *char_queue_create() {
  return queue_create(-1, char_copy_constructor, char_destructor);
}
queue *double_queue_create() {
  return queue_create(-1, double_copy_constructor, double_destructor);
}
queue *float_queue_create() {
  return queue_create(-1, float_copy_constructor, float_destructor);
}
queue *int_queue_create() {
  return queue_create(-1, int_copy_constructor, int_destructor);
}
queue *long_queue_create() {
  return queue_create(-1, long_copy_constructor, long_destructor);
}
queue *short_queue_create() {
  return queue_create(-1, short_copy_constructor, short_destructor);
}
queue *unsigned_char_queue_create() {
  return queue_create(-1, unsigned_char_copy_constructor,
                      unsigned_char_destructor);
}
queue *unsigned_int_queue_create() {
  return queue_create(-1, unsigned_int_copy_constructor,
                      unsigned_int_destructor);
}
queue *unsigned_long_queue_create() {
  return queue_create(-1, unsigned_long_copy_constructor,
                      unsigned_long_destructor);
}
queue *unsigned_short_queue_create() {
  return queue_create(-1, unsigned_short_copy_constructor,
                      unsigned_short_destructor);
}
