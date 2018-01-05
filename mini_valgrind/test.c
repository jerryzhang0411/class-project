/**
 * Mini Valgrind Lab
 * CS 241 - Spring 2017
 */

#include <stdio.h>
#include <stdlib.h>
 #include <string.h>
 #include "mini_valgrind.h"


int main() {
    // your tests here using malloc and free
    void *p1 =malloc(30);
   void *p2 =calloc(2,2);
  void * p4 = "abc";
  free(p4);
   //void *p3 =realloc(p4,5);
    void *p3=realloc(p2, 1);
   // void *p4 = calloc(50,1);
       //free(p4);
       free(p3);
  free(p1);
     free(p2);


 /*char *str;

   str = (char *) malloc(15);
   strcpy(str, "tutorialspoint");
   printf("String = %s,  Address = %s\n", str, str);


       str = (char *) realloc(str,30);
   //  strcat(str, ".com");
   printf("String = %s,  Address = %s\n", str, str);

   free(str);*/
   
   return(0);
   // return 0;
}
