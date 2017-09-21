/**
 * Deadlocked Diners Lab
 * CS 241 - Spring 2017
 */
#include "company.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//pthread_mutex_t arbitrator1 = PTHREAD_MUTEX_INITIALIZER;

void *work_interns(void *p) {
	Company *company = (Company *)p;
	pthread_mutex_t *left_intern, *right_intern;
	pthread_mutex_t *arbitrator = malloc(sizeof(pthread_mutex_t)); 
	pthread_mutex_init(arbitrator, NULL);
	int flag = 0;
	while (running){
		
		left_intern = Company_get_left_intern(company);
    	right_intern = Company_get_right_intern(company);
    	pthread_mutex_lock(arbitrator);
    	if(left_intern < right_intern){
    		int failed_left = pthread_mutex_trylock(left_intern);
	    	if(!failed_left){
	    		int failed_right = pthread_mutex_trylock(right_intern);
	    		if(!failed_right){
	    			flag = 1;
	    			Company_hire_interns(company);
	    			pthread_mutex_unlock(right_intern);
	    			pthread_mutex_unlock(left_intern);
	    		}else{
	    			pthread_mutex_unlock(left_intern);
	    		}
	    	}
	    	pthread_mutex_unlock(arbitrator);
		    if(flag){
		    	flag = 0;
		    	Company_have_board_meeting(company);
		    }else{
		    	// do nothing
		    }
    	}else{
    		int failed_right = pthread_mutex_trylock(right_intern);
	    	if(!failed_right){
	    		int failed_left = pthread_mutex_trylock(left_intern);
	    		if(!failed_left){
	    			flag = 1;
	    			Company_hire_interns(company);
	    			pthread_mutex_unlock(left_intern);
	    			pthread_mutex_unlock(right_intern);
	    		}else{
	    			pthread_mutex_unlock(right_intern);
	    		}
	    	}
	    	pthread_mutex_unlock(arbitrator);
		    if(flag){
		    	flag = 0;
		    	Company_have_board_meeting(company);
		    }else{
		    	// do nothing
		    }
    	}
    	
	    
	    
	}
	free(arbitrator);
	return NULL;
}
