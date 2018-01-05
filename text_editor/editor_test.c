/**
 * Machine Problem: Text Editor
 * CS 241 - Spring 2017
 */
#include "document.h"
#include "editor.h"
#include "format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * You can programatically test your text editor.
*/
int main() {
  // Setting up a docment based on the file named 'filename'.
  char *filename = "test.txt";
  Document *document = Document_create_from_file(filename);

  // print the first 5 lines
printf("ORIGINAL\n");
  handle_display_command(document, 1, 5);
printf("\nINSERT\n");
  handle_insert_command(document, (location){2, 3}, "kkkkkk");
  handle_display_command(document, 1, 5);
printf("\nINSERT_LINE\n");
  handle_insert_command(document, (location){7, 3}, "kkkkkk");
  handle_display_command(document, 1, 7);
printf("\nLOCATION_2, 3\n");
  location a=handle_search_command(document, (location){1, 3},"kkkkkk");
  printf("line:%zu, idx:%zu\n", a.line_no, a.idx);
  a=handle_search_command(document, (location){2, 0},"kkkkkk");
  printf("line:%zu, idx:%zu\n", a.line_no, a.idx);
printf("\nDELETE\n");
  handle_delete_command(document, (location){2, 3}, 6);
  handle_display_command(document, 1, 7);
printf("\nDELETE_LINE\n");
  handle_delete_line(document, 7);
  handle_display_command(document, 1, 7);
printf("\nLOCATION_NON\n");
  a=handle_search_command(document, (location){1, 3},"kkkkkk");
  printf("line:%zu, idx:%zu\n", a.line_no, a.idx);
printf("\nMERGE\n");
  handle_merge_line(document, 3);
  handle_display_command(document, 1, 7);
printf("\nSPLIT\n");
  handle_split_line(document, (location){3, 7});
  handle_display_command(document, 1, 7);

handle_save_command(document, "a.txt");
printf("\nSAVE\n");
handle_cleanup(document);
printf("\nCLEAN\n");






}
