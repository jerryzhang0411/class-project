/**
 * Mini Valgrind Lab
 * CS 241 - Spring 2017
 */

#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
  // your code here
  if(request_size == 0){
    return NULL;
  }
  meta_data * myMeta;
  myMeta = malloc(sizeof(meta_data) + request_size);
  if(myMeta != NULL){
    
  }else{
    return NULL;
  }
  myMeta->request_size = request_size;
  myMeta->filename = filename;
  myMeta->instruction = instruction;
  myMeta->next = head;
  head = myMeta;
  total_memory_requested += myMeta->request_size;

  return (void*)(head + 1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
  // your code here
  if((num_elements == 0) || (element_size == 0)){
    return NULL;
  }
  meta_data * myMeta;
  myMeta = calloc(element_size*num_elements + sizeof(meta_data), 1);
  if(myMeta != NULL){

  }else{
    return NULL;
  }
  myMeta->request_size = num_elements*element_size;
  myMeta->filename = filename;
  myMeta->instruction = instruction;
  myMeta->next = head;
  head = myMeta;
  meta_data * payload = myMeta + 1;
  char * it = payload;
  for(size_t i=0; i<(num_elements*element_size); i++){
    it[i] = 0;
  }
  total_memory_requested += myMeta->request_size;
  return (void*)(head + 1);
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
  // your code here
  int founded = 0;
  if((payload == NULL) && (request_size == 0)){
    return NULL;
  }
  if(payload == NULL){
    return mini_malloc(request_size, filename, instruction);
  }else if(request_size == 0){
    mini_free(payload);
    return NULL;
  }else{
    meta_data * checkHold = payload-sizeof(meta_data); // checking valid
    meta_data * tracker = head;
    while( tracker!= NULL ){
      if(checkHold == tracker){
        founded = 1;
        break;
      }else{
        tracker = tracker->next;
      }
    }                                            // end of while
    if(founded){                                 // valid meta_data
      if(checkHold -> request_size < request_size){
        tracker = realloc(tracker, sizeof(meta_data)+request_size);
        total_memory_requested += (request_size - checkHold->request_size); // increasing requested memory
        tracker->request_size = request_size;
      }else if(checkHold->request_size > request_size){
        tracker = realloc(tracker, sizeof(meta_data)+request_size);
        total_memory_freed += (checkHold->request_size - request_size); // increasing freed momery
        tracker->request_size = request_size;
      }else{

      }
      return (void*)(tracker+1);
    }else{
      invalid_addresses++;
      return NULL;
    }
  }
  return NULL;
}

void mini_free(void *payload) {
  // your code here
  int founded = 0;
  if(payload == NULL){
    return;
  }else{
    meta_data * checkHold = payload-sizeof(meta_data); // checking valid
    meta_data * tracker = head;
    meta_data * sb = head;                                     // prev pointer
    while( tracker!= NULL ){
      if(checkHold == tracker){
        founded = 1;
        break;
      }else{
        sb = tracker;
        tracker = tracker->next;
      }
    }                                            // end of while
    if(founded){
      if(tracker == head){
        head = head->next;
        total_memory_freed += tracker->request_size;
        free(tracker);
        return;
      }
      total_memory_freed += tracker->request_size;
      sb->next = tracker->next;
      tracker->next = NULL;
      free(tracker);
    }else{
      invalid_addresses++;
      return;
    }
  }                                                 // end of else
  return;
}
