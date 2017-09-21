/**
 * Extreme Edge Cases Lab
 * CS 241 - Spring 2017
 */
#include "camelCaser_tests.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "camelCaser.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */

int test_camelCaser(char **(*camelCaser)(const char *)) {
  // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.

	int retVal = 1;
	const char* test;
	char* trueAnswer[128];
	char** myAnswer = NULL;

	printf("===================Testing Empty String========================\n");
	test = "";
	trueAnswer[0] = NULL;
	char** temp = camelCaser(test);
	if(myAnswer[0] == '\0'){
		printf("Empty String Test Passed!\n");
	}else{
		printf("Empty String Test Failed!\n");
		retVal = 0;
		return retVal;
	}

	printf("==================End Test====================\n");




	printf("===================Testing Punctuation String========================\n");
	test = "......";
	//printf("imhere");
	myAnswer = camelCaser(test);
	if(myAnswer == NULL){
		retVal = 0;
		printf("Punctuation String Failed!\n");
		return retVal;
	}

	trueAnswer[0] = "";
	trueAnswer[1] = "";
	trueAnswer[2] = "";
	trueAnswer[3] = "";
	trueAnswer[4] = "";
	trueAnswer[5] = "";
	trueAnswer[6] = NULL;
	for(int j=0; j<(int)strlen(*trueAnswer); j++){

		if(strlen(*myAnswer) != strlen(*trueAnswer)){
			retVal = 0;
			printf("Punctuation String Failed!\n");
			break;
			return retVal;
		}else if(strcmp(myAnswer[j], temp[j])!=0){ // not equal
			retVal = 0;
			printf("Punctuation String Failed!\n");
			break;
			return retVal;
		}else{
			printf("Punctuation String Passed!\n");
		}
	}
	printf("==================End Test====================\n");


	printf("===================Testing Tab String========================\n");
	test = "		";
	myAnswer = camelCaser(test);

	if(myAnswer == NULL){
		retVal = 0;
		return retVal;
	}
	trueAnswer[0] = NULL;
	if(myAnswer[0] == NULL){
		printf("Tab Test Passed!\n");
	}else{
		retVal = 0;
		printf("Test Tab Failed\n");
		return retVal;
		}
	printf("==================End Test====================\n");



	printf("===================Testing Special String========================\n");
	test = "^.^";
	myAnswer = camelCaser(test);
	if(myAnswer == NULL){
		retVal = 0;
		printf("Special Test Failed!\n");
		return retVal;
	}

	trueAnswer[0] = "";
	trueAnswer[1] = "";
	trueAnswer[2] = "";
	trueAnswer[3] = NULL;
	for(int j=0; j<(int)strlen(*trueAnswer); j++){
		//printf("%s", trueAnswer[j]);
		if(strlen(*myAnswer) != strlen(*trueAnswer)){
			retVal = 0;
			printf("Special Test Failed!\n");
			break;
			return retVal;
		}else if(myAnswer[j] != temp[j]){ // not equal
			retVal = 0;
			printf("Special Test Failed!\n");
			break;
			return retVal;
		}else{
			printf("Special Test Passed!\n");
		}
	}
	printf("==================End Test====================\n");


	printf("===================Testing Incomplete========================\n");
	test = "asdf  97  fgf ";
	myAnswer = camelCaser(test);
	if(myAnswer[0] == NULL){
		retVal = 1;
		printf("Testing Incomplete Passed!\n");
	}else{
		retVal = 0;
		printf("Testing Incomplete Failed!\n");
		return retVal;
	}
	printf("==================End Test====================\n");


/*
	printf("===================Testing Normal========================\n");
	test = "The Heisenbug is an incredible creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener.";
	trueAnswer[0] = "theHeisenbugIsAnIncredibleCreature";
	trueAnswer[1] = "facenovelServersGetTheirPowerFromItsIndeterminism";
	trueAnswer[2] = "codeSmellCanBeIgnoredWithIncredibleUseOfAirFreshener";
	trueAnswer[3] = NULL;
	myAnswer = camelCaser(test);
	if(strlen(*myAnswer) != strlen(*trueAnswer)){
			retVal = 0;
			printf("Normal Failed!\n");
			return retVal;
		}
	
	for(int j=0; j<(int)strlen(*trueAnswer); j++){
		char * myString = myAnswer[j];
		if(myAnswer[j] != trueAnswer[j]){ // not equal
			retVal = 0;
			printf("Normal Failed!\n");
			return retVal;
		}else{
			printf("Normal Passed!\n");
		}
	}
	
	
	for (int j = 0; j<(int)strlen(*trueAnswer); j++) {
    		char * myString = myAnswer[j];
		if(myString){
			for (int i = 0; i <= (int)strlen(myString); i++){
				if(myString[i] != trueAnswer
				)
			}
			printf("\n");
		}
		else{
			printf("%s\n", "NULL");
		}
	}
	
	printf("==================End Test====================\n");

*/


	printf("===================Testing Number String========================\n");
	test = "1234.5678.90.";
	myAnswer = camelCaser(test);
	if(myAnswer == NULL){
		retVal = 0;
		printf("Number String Failed!\n");
		return retVal;
	}

	trueAnswer[0] = "1234";
	trueAnswer[1] = "5678";
	trueAnswer[2] = "90";
	trueAnswer[3] = NULL;
	
	for(int j=0; j<(int)strlen(*trueAnswer); j++){
		if(strlen(*myAnswer) != strlen(*trueAnswer)){
			retVal = 0;
			printf("Number String Failed!\n");
			return retVal;
		}else if(myAnswer[j] != trueAnswer[j]){ // not equal
			retVal = 0;
			printf("Number String Failed!\n");
			return retVal;
		}else{
			printf("Number String Passed!\n");
		}
	}
	
	printf("==================End Test====================\n");


	return retVal;
}
