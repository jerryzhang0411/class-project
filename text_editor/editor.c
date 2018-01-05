/**
 * Machine Problem: Text Editor
 * CS 241 - Spring 2017
 */
#include "document.h"
#include "editor.h"
#include "format.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_filename(int argc, char *argv[]) {
  // TODO implement get_filename
  // take a look at editor_main.c to see what this is used for
  if(argc < 2 || argc > 4){
    print_usage_error();
    return NULL;
  }
  return argv[1];
}

Document *handle_create_document(char *path_to_file) {
  // TODO create the document
  if(path_to_file == NULL){
    return NULL;
  }
  return Document_create_from_file(path_to_file);
}

void handle_cleanup(Document *document) {
  // TODO destroy the document
  Document_destroy(document);
}

void handle_display_command(Document *document, size_t start_line,
                            size_t max_lines) {
  // TODO implement handle_display_command
  if(Document_size(document) <= 0){
    print_document_empty_error();
    exit(1);
  }
  if(MIN(Document_size(document), (start_line + max_lines)) == Document_size(document)){ // document smaller than expected
    for(size_t i=start_line; i<=Document_size(document); i++){
      print_line(document, i);
    }
  }else{
    for(size_t i=start_line; i<(start_line+max_lines); i++){
      print_line(document, i);
    }
  }
  
}

void handle_insert_command(Document *document, location loc, char *line) {
  // TODO implement handle_insert_command
  assert(document);
  if(loc.line_no <= Document_size(document)){
    const char* oriLine = Document_get_line(document, loc.line_no);
    char* modifiedLine = malloc(strlen(line) + strlen(oriLine) +1 );
    int modiCount = 0;
    size_t i=0; // main phase iterator

    for(; i<loc.idx; i++){

      modifiedLine[modiCount] = oriLine[i];
      modiCount++;
    }
    //printf("%zu\n", i);
    for(size_t j=0; j<strlen(line); j++){
      //printf("imhere\n");
      modifiedLine[modiCount] = line[j];
      modiCount++;
    }
    for(; i<strlen(oriLine); i++){
      modifiedLine[modiCount] = oriLine[i];
      modiCount++;
    }
    modifiedLine[modiCount] = '\0';
    //printf("%s\n", modifiedLine);
    Document_set_line(document, loc.line_no, modifiedLine);
    free(modifiedLine);
    modifiedLine = NULL;
  }else{
    Document_insert_line(document, loc.line_no, line);
  }
}

void handle_delete_command(Document *document, location loc, size_t num_chars) {
  // TODO implement handle_delete_command
  assert(document);
  char* Line = strdup(Document_get_line(document, loc.line_no));
  char* modifiedLine = malloc(strlen(Line)+1);
  strncpy(modifiedLine, Line, loc.idx);
  modifiedLine[loc.idx] = '\0';
  if(strlen(modifiedLine)-loc.idx<=num_chars){
    Document_set_line(document, loc.line_no, modifiedLine);
  }else{
    strcat(modifiedLine, Line+loc.idx+num_chars);
    modifiedLine[strlen(Line)+1-num_chars] = '\0';
    Document_set_line(document, loc.line_no, modifiedLine);
  }
  free(modifiedLine);
  modifiedLine = NULL;
  free(Line);
  Line = NULL;
}

void handle_delete_line(Document *document, size_t line_no) {
  // TODO implement handle_delete_line
  assert(document);
  if(line_no > Document_size(document) || line_no <=0){
    exit(1);
  }
  Document_delete_line(document, line_no);
}

location handle_search_command(Document *document, location loc,
                               const char *search_str) {
  // TODO implement handle_search_command
  assert(document);
  assert(loc.line_no>0);
  const char * tempHold;
  const char * founded;

  for(size_t i=loc.line_no; i<=Document_size(document); i++){
    tempHold = Document_get_line(document, i);
    
    if(i==loc.line_no){                                             // found str @ 1st line
      founded = strstr(tempHold+loc.idx, search_str);
      if(founded != NULL){
        return (location){loc.line_no, (founded - tempHold)};
      }else{                                                                                                    // found str before the idx

      }                                                                                           
    }else{
      //printf("imhere\n");
      founded = strstr(tempHold, search_str);
      if(founded != NULL){
        //printf("imhere\n");
        
        return (location){i, founded-tempHold};
      }
    }
  } // end of for loop
  for(size_t i=1; i<loc.line_no+1; i++){
    tempHold = Document_get_line(document, i);                                                                                   
    founded = strstr(tempHold, search_str);
    if(founded != NULL){
      //printf("imhere\n");
      return (location){i, founded-tempHold};
    }
    
  } // end of for loop
  return (location){0,0};
}

void handle_merge_line(Document *document, size_t line_no) {
  // TODO implement handle_merge_command
  assert(document);
  assert(line_no>0);
  const char * thisLine = Document_get_line(document, line_no);
  const char * nextLine = Document_get_line(document, line_no+1);
  char * newLine = calloc(1, strlen(thisLine) + strlen(nextLine) +1);
  /*
  if(thisLine != NULL && nextLine != NULL){
    strcpy(newLine, thisLine);
    strcat(newLine, nextLine);
  }
  */
  strcat(newLine, thisLine);
  strcat(newLine, nextLine);
  newLine[strlen(newLine)] = '\0';
  Document_set_line(document, line_no, newLine);
  Document_delete_line(document, line_no+1);
  free(newLine);
  newLine = NULL;
}

void handle_split_line(Document *document, location loc) {
  // TODO implement handle_split_line
  assert(document);
  assert(loc.line_no>0);
  //size_t i;
  //int secondCount;
  int gaoshiCount = 0;
  const char *tempHold = Document_get_line(document, loc.line_no);
  char *gaoshiLine = malloc(loc.idx +1);
  for(size_t j=0; j<loc.idx; j++){
    gaoshiLine[gaoshiCount] = tempHold[j];
    gaoshiCount++;
  }
  gaoshiLine[gaoshiCount] = '\0';
  //printf("%s\n", gaoshiLine);
  /*
  char *secondPart = malloc(strlen(gaoshiLine)-loc.idx);
  for(secondCount = 0,i=loc.idx; i<strlen(Document_get_line(document, loc.line_no)); secondCount++, i++){
    //printf("%c\n", tempHold[i]);
    secondPart[secondCount] = tempHold[i];
  }
  */
  gaoshiLine[loc.idx] = '\0';
  //secondPart[secondCount] = '\0';
  //printf("%s\n", secondPart);
  Document_insert_line(document, loc.line_no+1, tempHold+loc.idx);
  Document_insert_line(document, loc.line_no+1, gaoshiLine);
  Document_delete_line(document, loc.line_no);
  free(gaoshiLine);
  gaoshiLine = NULL;
}

void handle_save_command(Document *document, const char *filename) {
  // TODO implement handle_save_command
  assert(document);
  if(filename != NULL)
    Document_write_to_file(document, filename);
}
