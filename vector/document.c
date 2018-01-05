/**
 * Machine Problem: Vector
 * CS 241 - Spring 2017
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"
#include "vector.h"

struct document {
  vector *vector;
};

// This is the constructor function for string element.
// Use this as copy_constructor callback in vector.
void *string_copy_constructor(void *elem) {
  char *str = elem;
  assert(str);
  return strdup(str);
}

// This is the destructor function for string element.
// Use this as destructor callback in vector.
void string_destructor(void *elem) { free(elem); }

// This is the default constructor function for string element.
// Use this as a default constructor callback in vector.
void *string_default_constructor(void) {
  // A single null byte
  return calloc(1, sizeof(char));
}

document *document_create() {
  // you code here!
  document* retVal = malloc(sizeof(vector*));
  retVal->vector = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
  return retVal;
}

void document_write_to_file(document *this, const char *path_to_file) {
  assert(this);
  assert(path_to_file);
  FILE * file = fopen(path_to_file, "w+");
  if(!file){
    perror("Could not read file");
    return;
  }
  const char* bytes;
  for(size_t i=1; i<=vector_size(this->vector); i++){
    bytes = document_get_line(this, i);
    fprintf(file, "%s\n", bytes);

  }
  fclose(file);
  // see the comment in the header file for a description of how to do this!
  // your code here!
}

document *document_create_from_file(const char *path_to_file) {

  assert(path_to_file);
  document * retVal = document_create();
  FILE * file = fopen(path_to_file, "r");
  if(!file){
    perror("Could not read file");
    return NULL;
  }
  char* bytes = NULL;
  size_t len = 0;
  ssize_t read;
  while((read = getline(&bytes, &len, file) != -1)){
    if(strchr(bytes, '\n') != NULL){
      char * temp = strchr(bytes, '\n');
      *temp = '\0';
    }
    vector_push_back(retVal->vector, bytes);
  }
  free(bytes);
  bytes = NULL;
  fclose(file);
  // this function will read a file which is created by document_write_to_file
  // your code here!
  return retVal;
}

void document_destroy(document *this) {
  assert(this);
  vector_destroy(this->vector);
  free(this);
}

size_t document_size(document *this) {
  assert(this);
  return vector_size(this->vector);
}

void document_set_line(document *this, size_t line_number, const char *str) {
  assert(this);
  assert(str);
  size_t index = line_number - 1;
  vector_set(this->vector, index, (void *)str);
}

const char *document_get_line(document *this, size_t line_number) {
  assert(this);
  assert(line_number > 0);
  size_t index = line_number - 1;
  return (const char *)vector_get(this->vector, index);
}

void document_insert_line(document *this, size_t line_number, const char *str) {
  assert(this);
  assert(str);
  assert(line_number > 0);
  if(line_number-1 > vector_size(this->vector)){
    vector_resize(this->vector, line_number-1);
  }
  vector_insert(this->vector, line_number-1, (void*)str);
  // your code here!
  // How are you going to handle the case when the user wants to
  // insert a line past the end of the document?
}

void document_delete_line(document *this, size_t line_number) {
  assert(this);
  assert(line_number > 0);
  size_t index = line_number - 1;
  vector_erase(this->vector, index);
}
